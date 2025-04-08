#include "domain/events/EventStore.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/music/Voice.h"
#include <stdexcept>
#include <iostream>
#include <future>
#include <memory>
#include <atomic>

namespace MusicTrainer::music::events {

std::unique_ptr<EventStore> EventStore::create() {
    return std::unique_ptr<EventStore>(new EventStore());
}

EventStore::EventStore() 
    : version(0)
    , eventCount(0)
    , snapshotInterval(100)
    , reconstructedScore(*Score::create(Voice::TimeSignature::commonTime())) {}

void EventStore::addEvent(std::unique_ptr<Event> event) {
    if (!event) return;

    // Try to add to pending events queue, push() will return false if queue is full
    if (pendingEvents.push(std::move(event))) {
        // Process pending events if we successfully pushed
        std::optional<std::unique_ptr<Event>> pendingEvent;
        while ((pendingEvent = pendingEvents.pop())) {
            events.push_back(std::move(*pendingEvent));
            updateIndices(events.back().get());
            incrementVersion();
            eventCount.fetch_add(1, std::memory_order_release);
            
            // Check if we need a snapshot
            if (events.size() >= snapshotInterval.load(std::memory_order_acquire)) {
                checkSnapshotNeeded(reconstructedScore);
            }
        }
    }
}

void EventStore::updateIndices(const Event* event) {
    if (!event) return;

    // Create a new IndexEntry for this event
    auto entry = std::make_shared<IndexEntry>(event);
    
    if (!event->getCorrelationId().empty()) {
        correlationIndex[event->getCorrelationId()].push_back(entry);
    }
    
    // Index by type
    typeIndex[event->getType()].push_back(entry);
    
    // Index by metadata
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
    pendingEvents.clear(); // Use the queue's clear() method instead of assignment
    correlationIndex.clear();
    metadataIndex.clear();
    typeIndex.clear();
    snapshots.clear();
    eventCount.store(0, std::memory_order_release);
    version.store(0, std::memory_order_release);
    reconstructedScore = *Score::create(Voice::TimeSignature::commonTime());
    incrementVersion();
}

void EventStore::createSnapshot(const MusicTrainer::music::Score& score) {
    snapshots.push_back(std::make_unique<::music::events::Snapshot>(score));
}

std::optional<const ::music::events::Snapshot*> EventStore::getLatestSnapshot() const {
    if (snapshots.empty()) return std::nullopt;
    return snapshots.back().get();
}

const MusicTrainer::music::Score& EventStore::reconstructState(size_t toEventIndex) const {
    const ::music::events::Snapshot* baseSnapshot = findNearestSnapshot(toEventIndex);
    
    if (baseSnapshot) {
        auto reconstructed = baseSnapshot->reconstruct();
        if (reconstructed) {
            reconstructedScore = *reconstructed;
            
            // Apply events from snapshot's version up to desired event index
            for (size_t i = getLatestSnapshot().value_or(nullptr)->getVersion() + 1; 
                 i < toEventIndex && i < events.size(); ++i) {
                if (events[i]) {
                    events[i]->apply(reconstructedScore);
                }
            }
        }
    } else {
        // No snapshot found, start from beginning with common time signature
        reconstructedScore = *Score::create(Voice::TimeSignature::commonTime());
        
        // Apply all events up to the desired index
        for (size_t i = 0; i < toEventIndex && i < events.size(); ++i) {
            if (events[i]) {
                events[i]->apply(reconstructedScore);
            }
        }
    }
    
    return reconstructedScore;
}

void EventStore::checkSnapshotNeeded(const MusicTrainer::music::Score& score) {
    size_t interval = snapshotInterval.load(std::memory_order_acquire);
    if (interval == 0) return;
    
    size_t currentCount = eventCount.load(std::memory_order_acquire);
    if (currentCount % interval == 0) {
        createSnapshot(score);
        if (auto latest = snapshots.back().get()) {
            latest->setVersion(currentCount);
        }
    }
}

const ::music::events::Snapshot* EventStore::findNearestSnapshot(size_t eventIndex) const {
    if (snapshots.empty()) return nullptr;
    
    const ::music::events::Snapshot* nearestSnapshot = nullptr;
    for (const auto& snapshot : snapshots) {
        if (snapshot->getVersion() <= eventIndex) {
            nearestSnapshot = snapshot.get();
        } else {
            break;
        }
    }
    
    return nearestSnapshot;
}

MusicTrainer::music::Score EventStore::applyEvents(
    const MusicTrainer::music::Score& baseScore,
    size_t fromEventIndex,
    size_t toEventIndex) const 
{
    MusicTrainer::music::Score result = baseScore;
    
    for (size_t i = fromEventIndex; i < toEventIndex && i < events.size(); ++i) {
        if (events[i]) {
            events[i]->apply(result);
        }
    }
    
    return result;
}

} // namespace MusicTrainer::music::events

