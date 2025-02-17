#include "utils/DebugUtils.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace MusicTrainer {
namespace Debug {

std::mutex LockTracker::trackerMutex;
std::unordered_map<const void*, std::vector<LockTracker::LockEvent>> LockTracker::lockHistory;

void LockTracker::recordLockAttempt(const void* mutex, const std::string& location) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	LockEvent event{
		std::chrono::system_clock::now(),
		std::this_thread::get_id(),
		"attempt",
		location
	};
	lockHistory[mutex].push_back(event);
}

void LockTracker::recordLockAcquired(const void* mutex, const std::string& location) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	LockEvent event{
		std::chrono::system_clock::now(),
		std::this_thread::get_id(),
		"acquired", 
		location
	};
	lockHistory[mutex].push_back(event);
}

void LockTracker::recordLockReleased(const void* mutex, const std::string& location) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	LockEvent event{
		std::chrono::system_clock::now(),
		std::this_thread::get_id(),
		"released",
		location
	};
	lockHistory[mutex].push_back(event);
}

void LockTracker::dumpLockHistory() {
	std::lock_guard<std::mutex> lock(trackerMutex);
	std::cout << "\n=== Lock History ===\n";
	for (const auto& [mutex, events] : lockHistory) {
		std::cout << "Mutex " << mutex << ":\n";
		for (const auto& event : events) {
			std::cout << "  Thread " << event.threadId 
					 << " " << event.action 
					 << " at " << event.location << "\n";
		}
	}
	std::cout << "==================\n";
}

void LockTracker::clearHistory() {
	std::lock_guard<std::mutex> lock(trackerMutex);
	lockHistory.clear();
}

} // namespace Debug
} // namespace MusicTrainer

