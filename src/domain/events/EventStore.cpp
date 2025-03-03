#include "domain/events/EventStore.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <stdexcept>
#include <iostream>
#include <future>
#include <memory>
#include <atomic>

namespace music::events {

std::unique_ptr<EventStore> EventStore::create() {
	return std::unique_ptr<EventStore>(new EventStore());
}

EventStore::EventStore() 
	: version(0)
	, eventCount(0)
	, snapshotInterval(100)
	, reconstructedScore(*MusicTrainer::music::Score::create()) {}


void EventStore::addEvent(std::unique_ptr<Event> event) {
	if (!event) {
		throw MusicTrainer::StateError("Cannot add null event");
	}
	
	// Add event to pending queue
	pendingEvents.push(std::move(event));
	
	// Process pending events
	while (auto pendingEvent = pendingEvents.pop()) {
		events.push_back(std::move(*pendingEvent));
		updateIndices(events.back().get());
		
		auto currentCount = eventCount.fetch_add(1, std::memory_order_acq_rel);
		if (currentCount % (snapshotInterval.load(std::memory_order_acquire) * 10) == 0) {
			checkSnapshotNeeded(reconstructedScore);
		}
		
		incrementVersion();
	}
}

void EventStore::updateIndices(const Event* event) {
	if (!event) return;
	
	auto entry = std::make_shared<IndexEntry>(event);
	
	// Update correlation index
	if (!event->getCorrelationId().empty()) {
		correlationIndex[event->getCorrelationId()].push_back(entry);
	}
	
	// Update type index
	typeIndex[event->getType()].push_back(entry);
	
	// Update metadata index
	for (const auto& [key, value] : event->getAllMetadata()) {
		metadataIndex[key][value].push_back(entry);
	}
}

std::vector<const Event*> EventStore::getEventsByCorrelation(const std::string& correlationId) const {
	std::vector<const Event*> result;
	auto it = correlationIndex.find(correlationId);
	if (it != correlationIndex.end()) {
		result.reserve(it->second.size());
		for (const auto& entry : it->second) {
			result.push_back(entry->event);
		}
	}
	return result;
}

std::vector<const Event*> EventStore::getEventsByMetadata(const std::string& key, const std::string& value) const {
	std::vector<const Event*> result;
	auto keyIt = metadataIndex.find(key);
	if (keyIt != metadataIndex.end()) {
		auto valueIt = keyIt->second.find(value);
		if (valueIt != keyIt->second.end()) {
			result.reserve(valueIt->second.size());
			for (const auto& entry : valueIt->second) {
				result.push_back(entry->event);
			}
		}
	}
	return result;
}

std::vector<const Event*> EventStore::getEventsByType(const std::string& type) const {
	std::vector<const Event*> result;
	auto it = typeIndex.find(type);
	if (it != typeIndex.end()) {
		result.reserve(it->second.size());
		for (const auto& entry : it->second) {
			result.push_back(entry->event);
		}
	}
	return result;
}

void EventStore::clear() {
	events.clear();
	snapshots.clear();
	correlationIndex.clear();
	metadataIndex.clear();
	typeIndex.clear();
	reconstructedScore = *MusicTrainer::music::Score::create();
	incrementVersion();
}

void EventStore::createSnapshot(const MusicTrainer::music::Score& score) {
	auto snapshot = Snapshot::create(score);
	snapshot->setVersion(eventCount.load(std::memory_order_acquire));
	snapshots.push_back(std::move(snapshot));
	incrementVersion();
}

std::optional<const Snapshot*> EventStore::getLatestSnapshot() const {
	if (snapshots.empty()) {
		return std::nullopt;
	}
	return snapshots.back().get();
}

const MusicTrainer::music::Score& EventStore::reconstructState(size_t toEventIndex) const {
	auto currentCount = eventCount.load(std::memory_order_acquire);
	if (toEventIndex > currentCount) {
		throw MusicTrainer::StateError("Event index out of range");
	}
	
	// Find nearest snapshot
	auto snapshot = findNearestSnapshot(toEventIndex);
	auto baseScore = snapshot ? snapshot->reconstruct() : MusicTrainer::music::Score::create();
	
	// Apply events
	auto result = std::make_unique<MusicTrainer::music::Score>(*baseScore);
	for (size_t i = 0; i < toEventIndex && i < events.size(); ++i) {
		if (events[i]) {
			events[i]->apply(*result);
		}
	}
	
	reconstructedScore = std::move(*result);
	return reconstructedScore;
}

void EventStore::checkSnapshotNeeded(const MusicTrainer::music::Score& score) {
	auto interval = snapshotInterval.load(std::memory_order_acquire);
	auto count = eventCount.load(std::memory_order_acquire);
	if (count >= interval && count % (interval * 10) == 0) {
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

MusicTrainer::music::Score EventStore::applyEvents(
    const MusicTrainer::music::Score& baseScore,
    size_t fromEventIndex,
    size_t toEventIndex) const {
	MusicTrainer::music::Score result = baseScore;
	for (size_t i = fromEventIndex; i < toEventIndex && i < events.size(); ++i) {
		if (events[i]) {
			events[i]->apply(result);
		}
	}
	return result;
}

} // namespace music::events

