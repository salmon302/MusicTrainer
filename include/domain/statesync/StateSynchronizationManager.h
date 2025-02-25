#pragma once

#include "domain/events/EventBus.h"
#include "domain/events/Event.h"
#include "domain/events/ScoreUpdatedEvent.h"
#include "domain/music/Score.h"
#include <memory>
#include <atomic>

namespace music::statesync {

class StateSynchronizationManager {
public:
	explicit StateSynchronizationManager(events::EventBus& eventBus)
		: eventBus(eventBus) {}
	
	void synchronize(const Score& score) {
		// Phase 1: Create score snapshot
		auto snapshot = score.createSnapshot();
		
		// Phase 2: Get next version (atomic)
		auto version = versionCounter.fetch_add(1, std::memory_order_acq_rel);
		
		// Phase 3: Create and publish event
		auto event = std::make_unique<events::ScoreUpdatedEvent>(snapshot, version);
		eventBus.publish(std::move(event));
	}

private:
	events::EventBus& eventBus;
	std::atomic<uint64_t> versionCounter{0};
};

} // namespace music::statesync

