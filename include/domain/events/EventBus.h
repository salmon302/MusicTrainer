#pragma once

#include "Event.h"
#include "adapters/LockFreeEventQueue.h"
#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <vector>

namespace music::events {

class EventBus {
public:
	static std::unique_ptr<EventBus> create();
	
	void publish(std::unique_ptr<Event> event) {
		std::string eventType = event->getType();
		
		// Store event and get handlers
		events.push_back(std::move(event));
		const Event* eventPtr = events.back().get();
		
		// Get handlers atomically
		auto handlersIt = handlers.find(eventType);
		if (handlersIt != handlers.end()) {
			for (const auto& handler : handlersIt->second) {
				handler(*eventPtr);
			}
		}
		
		incrementVersion();
	}

	void subscribe(const std::string& eventType, std::function<void(const Event&)> handler) {
		handlers[eventType].push_back(std::move(handler));
		incrementVersion();
	}

	std::vector<std::unique_ptr<Event>> getEvents() const {
		std::vector<std::unique_ptr<Event>> eventsCopy;
		eventsCopy.reserve(events.size());
		for (const auto& event : events) {
			eventsCopy.push_back(event->clone());
		}
		return eventsCopy;
	}

	void clear() {
		events.clear();
		handlers.clear();
		incrementVersion();
	}

private:
	EventBus() : version(0) {}
	
	// Version control for concurrent access
	std::atomic<uint64_t> version{0};
	void incrementVersion() { version.fetch_add(1, std::memory_order_acq_rel); }
	uint64_t getCurrentVersion() const { return version.load(std::memory_order_acquire); }
	
	// Event storage
	std::vector<std::unique_ptr<Event>> events;
	std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> handlers;
};

} // namespace music::events




