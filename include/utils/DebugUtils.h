#ifndef MUSICTRAINERV3_DEBUGUTILS_H
#define MUSICTRAINERV3_DEBUGUTILS_H

#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iostream>
#include <chrono>

namespace MusicTrainer {
namespace Debug {

class LockTracker {
public:
	static void recordLockAttempt(const void* mutex, const std::string& location);
	static void recordLockAcquired(const void* mutex, const std::string& location);
	static void recordLockReleased(const void* mutex, const std::string& location);
	static void dumpLockHistory();
	static void clearHistory();

private:
	struct LockEvent {
		std::chrono::system_clock::time_point timestamp;
		std::thread::id threadId;
		std::string action;
		std::string location;
	};
	
	static std::mutex trackerMutex;
	static std::unordered_map<const void*, std::vector<LockEvent>> lockHistory;
};

template<typename MutexType>
class DebugLockGuard {
public:
	DebugLockGuard(MutexType& mutex, const std::string& location) 
		: mutex_(mutex), location_(location) {
		LockTracker::recordLockAttempt(&mutex_, location_);
		mutex_.lock();
		LockTracker::recordLockAcquired(&mutex_, location_);
	}
	
	~DebugLockGuard() {
		mutex_.unlock();
		LockTracker::recordLockReleased(&mutex_, location_);
	}

private:
	MutexType& mutex_;
	std::string location_;
};

} // namespace Debug

// Helper macro that creates a unique variable name
#define DEBUG_LOCK_GUARD_CONCAT2(x, y) x##y
#define DEBUG_LOCK_GUARD_CONCAT(x, y) DEBUG_LOCK_GUARD_CONCAT2(x, y)

#define DEBUG_LOCK_GUARD(mutex) \
	MusicTrainer::Debug::DebugLockGuard<std::remove_reference_t<decltype(mutex)>> \
	DEBUG_LOCK_GUARD_CONCAT(debugLockGuard_, __LINE__)(mutex, std::string(__FILE__) + ":" + std::to_string(__LINE__))

} // namespace MusicTrainer

#endif // MUSICTRAINERV3_DEBUGUTILS_H