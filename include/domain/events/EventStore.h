#ifndef MUSICTRAINERV3_EVENTSTORE_H
#define MUSICTRAINERV3_EVENTSTORE_H

#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <atomic>
#include "Event.h"
#include "Snapshot.h"
#include "adapters/LockFreeEventQueue.h"

namespace music::events {

class EventStore {
public:
	static std::unique_ptr<EventStore> create();
	
	void addEvent(std::unique_ptr<Event> event);
	std::vector<std::unique_ptr<Event>> getEvents() const {
		std::vector<std::unique_ptr<Event>> eventsCopy;
		auto currentVersion = version.load(std::memory_order_acquire);
		eventsCopy.reserve(events.size());
		for (const auto& event : events) {
			eventsCopy.push_back(event->clone());
		}
		return eventsCopy;
	}
	
	size_t getEventCount() const { return eventCount.load(std::memory_order_acquire); }
	void clear();
	
	std::vector<const Event*> getEventsByCorrelation(const std::string& correlationId) const;
	std::vector<const Event*> getEventsByMetadata(const std::string& key, const std::string& value) const;
	std::vector<const Event*> getEventsByType(const std::string& type) const;
	
	void createSnapshot(const MusicTrainer::music::Score& score);
	std::optional<const Snapshot*> getLatestSnapshot() const;
	const MusicTrainer::music::Score& reconstructState(size_t toEventIndex) const;
	
	void setSnapshotInterval(size_t eventCount) { snapshotInterval.store(eventCount, std::memory_order_release); }
	size_t getSnapshotInterval() const { return snapshotInterval.load(std::memory_order_acquire); }

private:
	EventStore();
	
	std::atomic<uint64_t> version{0};
	std::atomic<size_t> eventCount{0};
	
	std::vector<std::unique_ptr<Event>> events;
	adapters::LockFreeEventQueue<std::unique_ptr<Event>, 1024> pendingEvents;
	
	struct IndexEntry {
		const Event* event;
		std::atomic<size_t> refCount{1};
		IndexEntry(const Event* e) : event(e) {}
	};
	
	std::unordered_map<std::string, std::vector<std::shared_ptr<IndexEntry>>> correlationIndex;
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::shared_ptr<IndexEntry>>>> metadataIndex;
	std::unordered_map<std::string, std::vector<std::shared_ptr<IndexEntry>>> typeIndex;
	
	std::vector<std::unique_ptr<Snapshot>> snapshots;
	std::atomic<size_t> snapshotInterval{100};
	
	mutable MusicTrainer::music::Score reconstructedScore;
	
	void checkSnapshotNeeded(const MusicTrainer::music::Score& score);
	const Snapshot* findNearestSnapshot(size_t eventIndex) const;
	MusicTrainer::music::Score applyEvents(const MusicTrainer::music::Score& baseScore, size_t fromEventIndex, size_t toEventIndex) const;
	void updateIndices(const Event* event);
	void incrementVersion() { version.fetch_add(1, std::memory_order_acq_rel); }
	uint64_t getCurrentVersion() const { return version.load(std::memory_order_acquire); }
};

} // namespace music::events

#endif // MUSICTRAINERV3_EVENTSTORE_H
