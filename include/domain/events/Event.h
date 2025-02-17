#pragma once

#include "domain/music/Score.h"
#include "../../utils/TrackedLock.h"
#include <string>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

namespace music::events {

class Event {
public:
	Event() = default;
	
	Event(const Event& other) {
		::utils::TrackedSharedMutexLock lock(other.mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		correlationId = other.correlationId;
		metadata = other.metadata;
	}
	
	Event& operator=(const Event& other) {
		if (this != &other) {
			// Get data under other's lock first
			std::string corrId;
			std::unordered_map<std::string, std::string> meta;
			{
				::utils::TrackedSharedMutexLock lock(other.mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
				corrId = other.correlationId;
				meta = other.metadata;
			}
			
			// Then update our data under our lock
			{
				::utils::TrackedUniqueLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
				correlationId = std::move(corrId);
				metadata = std::move(meta);
			}
		}
		return *this;
	}
	
	virtual ~Event() = default;
	
	virtual std::string getType() const = 0;
	virtual void apply(Score& target) const = 0;
	virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
	virtual std::string getDescription() const = 0;
	virtual std::unique_ptr<Event> clone() const = 0;
	
	std::string getCorrelationId() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return correlationId;
	}
	
	void setCorrelationId(const std::string& id) {
		::utils::TrackedUniqueLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		correlationId = id;
	}
	
	std::unordered_map<std::string, std::string> getAllMetadata() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return metadata;
	}

protected:
	mutable std::shared_mutex mutex_;  // Level 4 (REPOSITORY)
	std::string correlationId;
	std::unordered_map<std::string, std::string> metadata;
};

class EmptyEvent : public Event {
public:
	std::string getType() const override { return "Empty"; }
	void apply(Score& target) const override {}
	std::chrono::system_clock::time_point getTimestamp() const override { return std::chrono::system_clock::time_point{}; }
	std::string getDescription() const override { return "Empty Event"; }
	std::unique_ptr<Event> clone() const override { return std::make_unique<EmptyEvent>(); }
};

} // namespace music::events
