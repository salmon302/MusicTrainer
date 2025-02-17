#pragma once

#include "LockTracker.h"
#include <shared_mutex>
#include <chrono>
#include <thread>
#include <sstream>
#include <type_traits>

namespace utils {

template<typename Mutex>
class TrackedLock {
public:
	TrackedLock(Mutex& mutex, const char* name, LockLevel level, 
				std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
		: mutex_(mutex), name_(name), level_(level), locked_(false), timeout_(timeout) {
			
		if (!LockTracker::checkLockOrder(level_, name_)) {
			throw std::runtime_error("Lock order violation: " + std::string(name_));
		}
		
		lock();
	}

	void lock() {
		if (locked_) return;
		
		auto start = std::chrono::steady_clock::now();
		auto deadline = start + timeout_;
		int attempts = 0;
		
		while (true) {
			if (mutex_.try_lock()) {
				TRACK_LOCK(mutex_, name_, level_);
				locked_ = true;
				return;
			}
			
			attempts++;
			auto now = std::chrono::steady_clock::now();
			if (now >= deadline) {
				std::stringstream ss;
				ss << "[TrackedLock] Timeout acquiring lock " << name_ 
				   << " after " << attempts << " attempts ("
				   << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
				   << "ms elapsed)";
				throw std::runtime_error(ss.str());
			}
			
			// Increase sleep duration for longer timeouts
			auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
			auto sleepTime = std::min(remainingTime / 10, std::chrono::milliseconds(50));
			std::this_thread::sleep_for(sleepTime);
		}
	}
	
	void unlock() {
		if (!locked_) return;
		mutex_.unlock();
		TRACK_UNLOCK(mutex_);
		locked_ = false;
	}
	
	~TrackedLock() {
		if (locked_) {
			unlock();
		}
	}

	bool owns_lock() const { return locked_; }

private:
	Mutex& mutex_;
	const char* name_;
	LockLevel level_;
	bool locked_;
	std::chrono::milliseconds timeout_;
	
	TrackedLock(const TrackedLock&) = delete;
	TrackedLock& operator=(const TrackedLock&) = delete;
};

// Specialization for recursive mutex to ensure proper recursive locking
template<>
class TrackedLock<std::recursive_mutex> {
public:
	TrackedLock(std::recursive_mutex& mutex, const char* name, LockLevel level,
				std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
		: mutex_(mutex), name_(name), level_(level), locked_(false), timeout_(timeout) {
			
		if (!LockTracker::checkLockOrder(level_, name_)) {
			throw std::runtime_error("Lock order violation: " + std::string(name_));
		}
		
		lock();
	}

	void lock() {
		if (locked_) return;
		
		auto start = std::chrono::steady_clock::now();
		auto deadline = start + timeout_;
		int attempts = 0;
		
		while (true) {
			if (mutex_.try_lock()) {
				TRACK_LOCK(mutex_, name_, level_);
				locked_ = true;
				return;
			}
			
			attempts++;
			auto now = std::chrono::steady_clock::now();
			if (now >= deadline) {
				std::stringstream ss;
				ss << "[TrackedLock] Timeout acquiring recursive lock " << name_ 
				   << " after " << attempts << " attempts ("
				   << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
				   << "ms elapsed)";
				throw std::runtime_error(ss.str());
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	
	void unlock() {
		if (!locked_) return;
		mutex_.unlock();
		TRACK_UNLOCK(mutex_);
		locked_ = false;
	}
	
	~TrackedLock() {
		if (locked_) {
			unlock();
		}
	}

	bool owns_lock() const { return locked_; }

private:
	std::recursive_mutex& mutex_;
	const char* name_;
	LockLevel level_;
	bool locked_;
	std::chrono::milliseconds timeout_;
	
	TrackedLock(const TrackedLock&) = delete;
	TrackedLock& operator=(const TrackedLock&) = delete;
};

template<typename Mutex>
class TrackedSharedLock {
public:
	TrackedSharedLock(Mutex& mutex, const char* name, LockLevel level,
					 std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
		: mutex_(mutex), name_(name), level_(level), locked_(false), timeout_(timeout) {
			
		if (!LockTracker::checkLockOrder(level_, name_)) {
			throw std::runtime_error("Lock order violation: " + std::string(name_));
		}
		
		lock_shared();
	}
	
	void lock_shared() {
		if (locked_) return;
		
		auto deadline = std::chrono::steady_clock::now() + timeout_;
		int attempts = 0;
		while (!mutex_.try_lock_shared()) {
			if (std::chrono::steady_clock::now() >= deadline) {
				std::stringstream ss;
				ss << "[TrackedSharedLock] Timeout acquiring shared lock " << name_
				   << " after " << attempts << " attempts";
				throw std::runtime_error(ss.str());
			}
			attempts++;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
		TRACK_LOCK(mutex_, name_, level_);
		locked_ = true;
	}
	
	void unlock_shared() {
		if (!locked_) return;
		mutex_.unlock_shared();
		TRACK_UNLOCK(mutex_);
		locked_ = false;
	}
	
	~TrackedSharedLock() {
		if (locked_) {
			unlock_shared();
		}
	}

	bool owns_lock() const { return locked_; }

private:
	Mutex& mutex_;
	const char* name_;
	LockLevel level_;
	bool locked_;
	std::chrono::milliseconds timeout_;
	
	TrackedSharedLock(const TrackedSharedLock&) = delete;
	TrackedSharedLock& operator=(const TrackedSharedLock&) = delete;
};

using TrackedUniqueLock = TrackedLock<std::shared_mutex>;
using TrackedSharedMutexLock = TrackedSharedLock<std::shared_mutex>;
using TrackedRecursiveLock = TrackedLock<std::recursive_mutex>;

} // namespace utils