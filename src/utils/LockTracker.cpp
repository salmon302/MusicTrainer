#include "utils/LockTracker.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace utils {

std::mutex LockTracker::trackerMutex;
std::unordered_map<std::thread::id, std::vector<LockTracker::LockInfo>> LockTracker::activeLocks;

bool LockTracker::checkLockOrder(LockLevel attemptedLevel, const char* name) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	auto& threadLocks = activeLocks[std::this_thread::get_id()];
	
	// Check for reentrant locking
	for (const auto& info : threadLocks) {
		if (std::string(name) == info.name) {
			return true; // Allow reentrant locking
		}
	}
	
	// Check if we're trying to acquire a lower level lock than any currently held lock
	for (const auto& info : threadLocks) {
		if (attemptedLevel < info.level) {
			std::stringstream ss;
			ss << "Lock order violation: Attempting to acquire " << name 
			   << " (level " << static_cast<int>(attemptedLevel) 
			   << ") while holding " << info.name 
			   << " (level " << static_cast<int>(info.level) << ")";
			throw std::runtime_error(ss.str());
		}
	}
	
	return true;
}


void LockTracker::recordLock(const void* mutex, const char* name, LockLevel level, const char* file, int line) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	auto& threadLocks = activeLocks[std::this_thread::get_id()];
	
	// Check for existing lock
	auto it = std::find_if(threadLocks.begin(), threadLocks.end(),
						 [mutex](const LockInfo& info) { return info.mutex == mutex; });
						 
	if (it != threadLocks.end()) {
		it->recursionCount++;
	} else {
		threadLocks.push_back({mutex, name, file, line, level, 1});
	}
	
	checkDeadlock(std::this_thread::get_id());

}

void LockTracker::recordUnlock(const void* mutex) {
	std::lock_guard<std::mutex> lock(trackerMutex);
	auto& threadLocks = activeLocks[std::this_thread::get_id()];
	
	auto it = std::find_if(threadLocks.begin(), threadLocks.end(),
						 [mutex](const LockInfo& info) { return info.mutex == mutex; });
						 
	if (it != threadLocks.end()) {
		if (--it->recursionCount == 0) {
			threadLocks.erase(it);
		}
	}
}


void LockTracker::checkDeadlock(std::thread::id currentThread) {
	std::vector<std::thread::id> chain;
	chain.push_back(currentThread);
	
	std::thread::id nextThread = currentThread;
	while (true) {
		bool foundNext = false;
		for (const auto& [threadId, locks] : activeLocks) {
			if (threadId == nextThread || locks.empty()) continue;
			
			if (isWaitingFor(threadId, locks.back().mutex)) {
				if (std::find(chain.begin(), chain.end(), threadId) != chain.end()) {
					reportDeadlock(chain);
					return;
				}
				chain.push_back(threadId);
				nextThread = threadId;
				foundNext = true;
				break;
			}
		}
		if (!foundNext) break;
	}
}


bool LockTracker::isWaitingFor(std::thread::id threadId, const void* mutex) {
	auto it = activeLocks.find(threadId);
	if (it == activeLocks.end() || it->second.empty()) return false;
	
	return std::any_of(it->second.begin(), it->second.end(),
		[mutex](const LockInfo& info) { 
			return info.mutex == mutex && info.recursionCount > 0; 
		});
}


void LockTracker::reportDeadlock(const std::vector<std::thread::id>& chain) {
	std::stringstream ss;
	ss << "POTENTIAL DEADLOCK DETECTED! Lock chain:\n";
	for (size_t i = 0; i < chain.size(); ++i) {
		auto threadId = chain[i];
		ss << "Thread " << threadId;
		
		auto it = activeLocks.find(threadId);
		if (it != activeLocks.end() && !it->second.empty()) {
			const auto& lastLock = it->second.back();
			ss << " holding " << lastLock.name 
			   << " (level " << static_cast<int>(lastLock.level) << ")";
		}
		
		if (i < chain.size() - 1) ss << " ->\n";
	}
	std::cerr << ss.str() << std::endl;
}

} // namespace utils