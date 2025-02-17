#include "domain/events/EventStore.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "utils/TrackedLock.h"
#include <stdexcept>
#include <iostream>
#include <future>

using namespace MusicTrainer;

namespace music::events {

std::unique_ptr<EventStore> EventStore::create() {
	return std::unique_ptr<EventStore>(new EventStore());
}

EventStore::EventStore() : snapshotInterval(100), reconstructedScore(*Score::create()) {}

void EventStore::addEvent(std::unique_ptr<Event> event) {
	if (!event) {
		throw StateError("Cannot add null event");
	}
	
	{
		::utils::TrackedUniqueLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		try {
			updateIndices(event.get());
			events.push_back(std::move(event));
		} catch (const StateError& error) {
			HANDLE_ERROR(error);
			throw;
		}
	}
	
	size_t currentSize;
	{
		::utils::TrackedSharedMutexLock readLock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		currentSize = events.size();
	}
	
	if (currentSize % (snapshotInterval * 10) == 0) {
		::utils::TrackedUniqueLock snapshotLock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		checkSnapshotNeeded(reconstructedScore);
	}
}

void EventStore::updateIndices(const Event* event) {
	if (!event->getCorrelationId().empty()) {
		correlationIndex[event->getCorrelationId()].push_back(event);
	}
	
	typeIndex[event->getType()].push_back(event);
	
	for (const auto& [key, value] : event->getAllMetadata()) {
		metadataIndex[key][value].push_back(event);
	}
}

std::vector<const Event*> EventStore::getEventsByCorrelation(const std::string& correlationId) const {
	::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	auto it = correlationIndex.find(correlationId);
	return it != correlationIndex.end() ? it->second : std::vector<const Event*>{};
}

std::vector<const Event*> EventStore::getEventsByMetadata(const std::string& key, const std::string& value) const {
	::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	auto keyIt = metadataIndex.find(key);
	if (keyIt != metadataIndex.end()) {
		auto valueIt = keyIt->second.find(value);
		if (valueIt != keyIt->second.end()) {
			return valueIt->second;
		}
	}
	return {};
}

std::vector<const Event*> EventStore::getEventsByType(const std::string& type) const {
	::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	auto it = typeIndex.find(type);
	return it != typeIndex.end() ? it->second : std::vector<const Event*>{};
}

void EventStore::clear() {
	::utils::TrackedUniqueLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	events.clear();
	snapshots.clear();
	correlationIndex.clear();
	metadataIndex.clear();
	typeIndex.clear();
	reconstructedScore = *Score::create();
}

void EventStore::createSnapshot(const Score& score) {
	::utils::TrackedUniqueLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	auto snapshot = Snapshot::create(score);
	snapshot->setVersion(events.size());
	snapshots.push_back(std::move(snapshot));
}

std::optional<const Snapshot*> EventStore::getLatestSnapshot() const {
	::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
	if (snapshots.empty()) {
		return std::nullopt;
	}
	return snapshots.back().get();
}

const Score& EventStore::reconstructState(size_t toEventIndex) const {
	// First collect all data needed for reconstruction under EventStore lock
	std::vector<std::unique_ptr<Event>> eventsCopy;
	std::unique_ptr<Score> baseScore;
	
	{
		// Acquire EventStore lock first (level 2)
		::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		
		if (toEventIndex > events.size()) {
			throw StateError("Event index out of range");
		}
		
		// Find nearest snapshot and copy events
		auto snapshot = findNearestSnapshot(toEventIndex);
		if (snapshot) {
			baseScore = snapshot->reconstruct();
		} else {
			baseScore = Score::create();
		}
		
		eventsCopy.reserve(toEventIndex);
		for (size_t i = 0; i < toEventIndex && i < events.size(); ++i) {
			if (events[i]) {
				eventsCopy.push_back(events[i]->clone());
			}
		}
	}
	
	// Create a new score to hold the result
	auto resultScore = std::make_unique<Score>(*baseScore);
	
	// Apply events without holding any locks
	for (const auto& event : eventsCopy) {
		if (event) {
			event->apply(*resultScore);
		}
	}
	
	// Update reconstructed score under EventStore lock
	{
		::utils::TrackedUniqueLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		reconstructedScore = std::move(*resultScore);
		return reconstructedScore;
	}
}

void EventStore::checkSnapshotNeeded(const Score& score) {
	if (events.size() >= snapshotInterval && events.size() % (snapshotInterval * 10) == 0) {
		createSnapshot(score);
	}
}

const Snapshot* EventStore::findNearestSnapshot(size_t eventIndex) const {
	const Snapshot* nearest = nullptr;
	for (const auto& snapshot : snapshots) {
		if (snapshot->getVersion() <= eventIndex) {
			nearest = snapshot.get();
		} else {
			break;
		}
	}
	return nearest;
}

Score EventStore::applyEvents(const Score& baseScore, size_t fromEventIndex, size_t toEventIndex) const {
	auto result = baseScore;
	
	for (size_t i = fromEventIndex; i < toEventIndex && i < events.size(); ++i) {
		const auto& event = events[i];
		event->apply(result);
	}
	
	return result;
}

} // namespace music::events
