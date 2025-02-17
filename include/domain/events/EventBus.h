#pragma once

#include "Event.h"
#include "utils/TrackedLock.h"
#include <functional>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace music::events {

class EventBus {
public:
	static std::unique_ptr<EventBus> create();
	
	void publish(std::unique_ptr<Event> event) {
		std::vector<std::function<void(const Event&)>> handlersCopy;
		const Event* eventPtr = nullptr;
		std::string eventType;
		
		// Acquire EVENT_BUS lock (level 2)
		{
			::utils::TrackedUniqueLock lock(mutex_, "EventBus::mutex_", ::utils::LockLevel::EVENT_BUS);
			eventType = event->getType();
			events.push_back(std::move(event));
			eventPtr = events.back().get();
			
			// Get handlers for event type
			auto handlersIt = handlers.find(eventType);
			if (handlersIt != handlers.end()) {
				handlersCopy = handlersIt->second;
			}
		}
		
		// Call handlers outside of lock to prevent deadlocks
		if (eventPtr) {
			for (const auto& handler : handlersCopy) {
				handler(*eventPtr);
			}
		}
	}

	void subscribe(const std::string& eventType, std::function<void(const Event&)> handler) {
		::utils::TrackedUniqueLock lock(mutex_, "EventBus::mutex_", ::utils::LockLevel::EVENT_BUS);
		handlers[eventType].push_back(std::move(handler));
	}

	std::vector<std::unique_ptr<Event>> getEvents() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "EventBus::mutex_", ::utils::LockLevel::EVENT_BUS);
		std::vector<std::unique_ptr<Event>> eventsCopy;
		eventsCopy.reserve(events.size());
		for (const auto& event : events) {
			eventsCopy.push_back(event->clone());
		}
		return eventsCopy;
	}

	void clear() {
		::utils::TrackedUniqueLock lock(mutex_, "EventBus::mutex_", ::utils::LockLevel::EVENT_BUS);
		events.clear();
		handlers.clear();
	}

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return mutex_; }
#endif

private:
	EventBus() = default;
	mutable std::shared_mutex mutex_;  // Level EVENT_BUS
	std::vector<std::unique_ptr<Event>> events;
	std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> handlers;
};

} // namespace music::events

