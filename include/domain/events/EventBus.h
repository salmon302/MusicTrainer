#pragma once

#include "Event.h"
#include "adapters/LockFreeEventQueue.h"
#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <mutex> // For thread safety

namespace music::events {

class EventBus {
public:
	static std::unique_ptr<EventBus> create();
	
	void publish(std::unique_ptr<Event> event) {
		std::string eventType = event->getType();
		const Event* eventPtr;
		
		// Create a copy of the handlers to avoid holding the lock during callback execution
		std::vector<std::function<void(const Event&)>> handlersCopy;
		
		{
			// Use lock to protect concurrent access to events vector - scope limited
			std::lock_guard<std::mutex> lock(mutex);
			
			// Store event
			events.push_back(std::move(event));
			eventPtr = events.back().get();
			
			// Get handlers
			auto handlersIt = handlers.find(eventType);
			if (handlersIt != handlers.end()) {
				handlersCopy = handlersIt->second;
			}
			
			incrementVersion();
		}
		
		// Execute handlers outside of the lock
		for (const auto& handler : handlersCopy) {
			handler(*eventPtr);
		}
	}

	void subscribe(const std::string& eventType, std::function<void(const Event&)> handler) {
		std::lock_guard<std::mutex> lock(mutex);
		handlers[eventType].push_back(std::move(handler));
		incrementVersion();
	}

	std::vector<std::unique_ptr<Event>> getEvents() const {
		std::lock_guard<std::mutex> lock(mutex);
		std::vector<std::unique_ptr<Event>> eventsCopy;
		eventsCopy.reserve(events.size());
		for (const auto& event : events) {
			eventsCopy.push_back(event->clone());
		}
		return eventsCopy;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex);
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
	mutable std::mutex mutex; // Mutex to protect concurrent access
	std::vector<std::unique_ptr<Event>> events;
	std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> handlers;
};

} // namespace music::events




