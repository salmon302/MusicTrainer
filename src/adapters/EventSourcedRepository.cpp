#include "adapters/EventSourcedRepository.h"
#include "domain/music/Score.h"
#include "domain/errors/DomainErrors.h"
#include <algorithm>

namespace music::adapters {

std::unique_ptr<EventSourcedRepository> EventSourcedRepository::create() {
	return std::unique_ptr<EventSourcedRepository>(new EventSourcedRepository());
}

void EventSourcedRepository::save(const std::string& name, const MusicTrainer::music::Score& score) {
	updateState(name, [&score](ScoreState& state) {
		state.snapshot = std::make_unique<events::Snapshot>(score);
		state.version = 0;
		state.events.clear();
	});
}

std::unique_ptr<MusicTrainer::music::Score> EventSourcedRepository::load(const std::string& name) {
	auto it = scoreStates.find(name);
	if (it == scoreStates.end()) {
		throw MusicTrainer::RepositoryError("Score not found: " + name);
	}
	return reconstructScore(it->second);
}

std::vector<std::string> EventSourcedRepository::listScores() {
	std::vector<std::string> scores;
	scores.reserve(scoreStates.size());
	for (const auto& pair : scoreStates) {
		scores.push_back(pair.first);
	}
	return scores;
}

void EventSourcedRepository::remove(const std::string& name) {
	scoreStates.erase(name);
}

void EventSourcedRepository::appendEvents(const std::string& name, 
	const std::vector<std::unique_ptr<events::Event>>& events) {
	updateState(name, [&events](ScoreState& state) {
		for (const auto& event : events) {
			state.events.push_back(event->clone());
		}
		state.version += events.size();
	});
}

void EventSourcedRepository::createSnapshot(const std::string& name) {
	auto score = load(name);
	if (score) {
		updateState(name, [&](ScoreState& state) {
			state.snapshot = std::make_unique<events::Snapshot>(*score);
			state.events.clear();
			state.version = 0;
		});
	}
}

std::vector<std::unique_ptr<events::Event>> EventSourcedRepository::getEvents(
	const std::string& name, size_t fromVersion) {
	auto it = scoreStates.find(name);
	if (it == scoreStates.end()) {
		return {};
	}
	
	std::vector<std::unique_ptr<events::Event>> result;
	size_t currentVersion = 0;
	for (const auto& event : it->second.events) {
		if (currentVersion >= fromVersion) {
			result.push_back(event->clone());
		}
		currentVersion++;
	}
	return result;
}

std::unique_ptr<MusicTrainer::music::Score> EventSourcedRepository::reconstructScore(const ScoreState& state) {
	if (!state.snapshot) {
		return nullptr;
	}
	
	auto score = state.snapshot->reconstruct();
	for (const auto& event : state.events) {
		event->apply(*score);
	}
	return score;
}

} // namespace music::adapters