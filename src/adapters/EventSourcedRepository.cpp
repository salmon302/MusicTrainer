#include "adapters/EventSourcedRepository.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Duration.h"
#include "domain/errors/DomainErrors.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <stdexcept>
#include <memory>

namespace music::adapters {

std::unique_ptr<EventSourcedRepository> EventSourcedRepository::create() {
	return std::unique_ptr<EventSourcedRepository>(new EventSourcedRepository());
}

void EventSourcedRepository::save(const std::string& name, const Score& score) {
	std::cout << "[EventSourcedRepository::save] Saving score: " << name << std::endl;
	
	// Create snapshot without holding any locks
	auto newSnapshot = events::Snapshot::create(score);
	
	// Then update state under repository lock
	{
		::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto& state = scoreStates[name];
		state.snapshot = std::move(newSnapshot);
		state.version = state.events.size();
	}
	std::cout << "[EventSourcedRepository::save] Save complete" << std::endl;
}


std::unique_ptr<Score> EventSourcedRepository::load(const std::string& name) {
	std::cout << "[EventSourcedRepository::load] Loading score: " << name << std::endl;
	
	// Get state data under repository lock
	ScoreState stateCopy;
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto it = scoreStates.find(name);
		if (it == scoreStates.end()) {
			throw MusicTrainer::RepositoryError("Score not found: " + name);
		}
		// Make a copy of the state data
		if (it->second.snapshot) {
			stateCopy.snapshot = it->second.snapshot->clone();
		}
		for (const auto& event : it->second.events) {
			if (event) {
				stateCopy.events.push_back(event->clone());
			}
		}
		stateCopy.version = it->second.version;
	}
	
	// Reconstruct score without holding locks
	auto result = reconstructScore(stateCopy);
	std::cout << "[EventSourcedRepository::load] Load complete" << std::endl;
	return result;
}

std::vector<std::string> EventSourcedRepository::listScores() {
	::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
	std::vector<std::string> names;
	names.reserve(scoreStates.size());
	for (const auto& [name, _] : scoreStates) {
		names.push_back(name);
	}
	return names;
}

void EventSourcedRepository::remove(const std::string& name) {
	::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
	scoreStates.erase(name);
}

void EventSourcedRepository::appendEvents(const std::string& name, const std::vector<std::unique_ptr<events::Event>>& newEvents) {
	std::cout << "[EventSourcedRepository::appendEvents] Appending events for: " << name << std::endl;
	
	// Make copies of all events without holding any locks
	std::vector<std::unique_ptr<events::Event>> eventCopies;
	eventCopies.reserve(newEvents.size());
	for (const auto& event : newEvents) {
		if (event) {
			eventCopies.push_back(event->clone());
		}
	}
	
	// Add events to state in a single lock operation
	{
		::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto& state = scoreStates[name];
		for (auto& event : eventCopies) {
			state.events.push_back(std::move(event));
		}
		state.version = state.events.size();
	}
	
	std::cout << "[EventSourcedRepository::appendEvents] Append complete" << std::endl;
}


void EventSourcedRepository::createSnapshot(const std::string& name) {
	std::cout << "[EventSourcedRepository::createSnapshot] Creating snapshot for: " << name << std::endl;
	
	// First get state data under repository lock
	ScoreState stateCopy;
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto it = scoreStates.find(name);
		if (it == scoreStates.end()) {
			throw std::runtime_error("Score not found: " + name);
		}
		// Make a copy of the state data
		if (it->second.snapshot) {
			stateCopy.snapshot = it->second.snapshot->clone();
		}
		for (const auto& event : it->second.events) {
			if (event) {
				stateCopy.events.push_back(event->clone());
			}
		}
		stateCopy.version = it->second.version;
	} // Release repository lock before any Score operations
	
	// Reconstruct score and create snapshot without holding any locks
	auto score = reconstructScore(stateCopy);
	if (!score) {
		throw std::runtime_error("Failed to reconstruct score for snapshot");
	}
	
	// Create new snapshot from reconstructed score
	auto newSnapshot = events::Snapshot::create(*score);
	
	// Finally update repository state
	{
		::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto& state = scoreStates[name];
		state.snapshot = std::move(newSnapshot);
		state.version = state.events.size();
	}
	
	std::cout << "[EventSourcedRepository::createSnapshot] Snapshot creation complete" << std::endl;
}


std::vector<std::unique_ptr<events::Event>> EventSourcedRepository::getEvents(const std::string& name, size_t fromVersion) {
	::utils::TrackedUniqueLock lock(mutex_, "EventSourcedRepository::mutex_", ::utils::LockLevel::REPOSITORY);
	auto it = scoreStates.find(name);
	if (it == scoreStates.end()) {
		return {};
	}
	
	if (fromVersion >= it->second.events.size()) {
		return {};
	}
	
	std::vector<std::unique_ptr<events::Event>> result;
	result.reserve(it->second.events.size() - fromVersion);
	
	for (size_t i = fromVersion; i < it->second.events.size(); ++i) {
		if (it->second.events[i]) {
			result.push_back(it->second.events[i]->clone());
		}
	}
	return result;
}

std::unique_ptr<Score> EventSourcedRepository::reconstructScore(const ScoreState& state) {
	// No need for additional lock here since the caller already holds the mutex
	std::cout << "[EventSourcedRepository::reconstructScore] Starting reconstruction" << std::endl;
	std::unique_ptr<Score> score;
	
	// Create base score without holding any locks
	if (state.snapshot) {
		score = state.snapshot->reconstruct();
	} else {
		auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
		score = Score::create(timeSignature);
	}
	
	if (!score) {
		return nullptr;
	}
	
	// Apply events without holding any locks
	for (const auto& event : state.events) {
		if (event) {
			event->apply(*score);
		}
	}
	
	std::cout << "[EventSourcedRepository::reconstructScore] Reconstruction complete" << std::endl;
	return score;
}

} // namespace music::adapters