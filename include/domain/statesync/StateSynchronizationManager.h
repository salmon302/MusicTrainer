#pragma once

#include "domain/events/EventBus.h"
#include "domain/events/Event.h"
#include "domain/events/ScoreUpdatedEvent.h"
#include "domain/music/Score.h"
#include "utils/TrackedLock.h"
#include <memory>
#include <shared_mutex>
#include <atomic>

namespace music::statesync {

class StateSynchronizationManager {
public:
	explicit StateSynchronizationManager(events::EventBus& eventBus);
	
	void synchronize(const Score& score) {
		// Phase 1: Create score snapshot (Score lock -> Voice lock)
		auto snapshot = score.createSnapshot();
		
		// Phase 2: Get next version (atomic, no locks needed)
		auto version = versionCounter.fetch_add(1, std::memory_order_relaxed);
		
		// Phase 3: Create and publish event (EVENT_BUS lock)
		auto event = std::make_unique<events::ScoreUpdatedEvent>(snapshot, version);
		eventBus.publish(std::move(event));
	}



#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return mutex_; }
#endif

private:
	events::EventBus& eventBus;
	std::atomic<uint64_t> versionCounter{0};
	mutable std::shared_mutex mutex_; // Level STATE_SYNC (3) - State synchronization operations
};

} // namespace music::statesync
