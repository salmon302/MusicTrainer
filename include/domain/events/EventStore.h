#ifndef MUSICTRAINERV3_EVENTSTORE_H
#define MUSICTRAINERV3_EVENTSTORE_H

#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include "Event.h"
#include "Snapshot.h"
#include "utils/TrackedLock.h"

namespace music::events {

class EventStore {
public:
	static std::unique_ptr<EventStore> create();
	
	// Event management
	void addEvent(std::unique_ptr<Event> event);
	std::vector<std::unique_ptr<Event>> getEvents() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		std::vector<std::unique_ptr<Event>> eventsCopy;
		eventsCopy.reserve(events.size());
		for (const auto& event : events) {
			eventsCopy.push_back(event->clone());
		}
		return eventsCopy;
	}
	size_t getEventCount() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "EventStore::mutex_", ::utils::LockLevel::EVENT_BUS);
		return events.size();
	}
	void clear();
	
	// Event querying
	std::vector<const Event*> getEventsByCorrelation(const std::string& correlationId) const;
	std::vector<const Event*> getEventsByMetadata(const std::string& key, const std::string& value) const;
	std::vector<const Event*> getEventsByType(const std::string& type) const;
	
	// Snapshot management
	void createSnapshot(const Score& score);
	std::optional<const Snapshot*> getLatestSnapshot() const;
	const Score& reconstructState(size_t toEventIndex) const;
	
	// Configuration
	void setSnapshotInterval(size_t eventCount) { snapshotInterval = eventCount; }
	size_t getSnapshotInterval() const { return snapshotInterval; }

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() { return mutex_; }
#endif

private:
	EventStore();
	
	mutable std::shared_mutex mutex_;  // Level EVENT_BUS - Mutex for thread safety
	
	// Event storage
	std::vector<std::unique_ptr<Event>> events;
	std::unordered_map<std::string, std::vector<const Event*>> correlationIndex;
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const Event*>>> metadataIndex;
	std::unordered_map<std::string, std::vector<const Event*>> typeIndex;
	
	// Snapshot management
	std::vector<std::unique_ptr<Snapshot>> snapshots;
	size_t snapshotInterval{100}; // Create snapshot every N events
	
	// State reconstruction
	mutable Score reconstructedScore;
	
	// Helper methods
	void checkSnapshotNeeded(const Score& score);
	const Snapshot* findNearestSnapshot(size_t eventIndex) const;
	Score applyEvents(const Score& baseScore, size_t fromEventIndex, size_t toEventIndex) const;
	void updateIndices(const Event* event);
};

} // namespace music::events

#endif // MUSICTRAINERV3_EVENTSTORE_H