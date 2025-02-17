#pragma once

#include <mutex>
#include <unordered_map>
#include <string>
#include <thread>
#include <vector>

namespace utils {

enum class LockLevel {
	VOICE = 0,          // Lowest level
	SCORE = 1,
	EVENT_BUS = 2,      // Event system operations
	STATE_SYNC = 3,     // State synchronization operations
	VALIDATION = 4,     // Validation operations
	METRICS = 5,        // Metrics tracking level
	REPOSITORY = 6,     // Repository operations
	PROGRESSION = 7,    // Progression system operations
	ERROR_LOGGING = 8,  // Error logging operations
	ERROR_HANDLER = 9,  // Error handling operations
	RECOVERY = 10       // Highest level - Recovery strategy operations
};

class LockTracker {
public:
	static bool checkLockOrder(LockLevel attemptedLevel, const char* name);
	static void recordLock(const void* mutex, const char* name, LockLevel level, const char* file, int line);
	static void recordUnlock(const void* mutex);

private:
	struct LockInfo {
		const void* mutex;
		std::string name;
		std::string file;
		int line;
		LockLevel level;
		int recursionCount;
	};
	
	static std::mutex trackerMutex;
	static std::unordered_map<std::thread::id, std::vector<LockInfo>> activeLocks;
	
	static void checkDeadlock(std::thread::id currentThread);
	static bool isWaitingFor(std::thread::id threadId, const void* mutex);
	static void reportDeadlock(const std::vector<std::thread::id>& chain);
};

} // namespace utils

#define TRACK_LOCK(mutex, name, level) \
	::utils::LockTracker::recordLock(&mutex, name, level, __FILE__, __LINE__)

#define TRACK_UNLOCK(mutex) \
	::utils::LockTracker::recordUnlock(&mutex)


