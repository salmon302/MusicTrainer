#pragma once
#include "ErrorBase.h"
#include "utils/TrackedLock.h"
#include "RecoveryStrategy.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include <shared_mutex>
#include <future>
#include <iostream>

namespace MusicTrainer {

enum class ErrorSeverity {
	Info,
	Warning,
	Error,
	Critical
};

class ErrorHandler {
public:
	using ErrorCallback = std::function<void(const MusicTrainerError&)>;
	using RecoveryCallback = std::function<bool(const MusicTrainerError&)>;
	
	static ErrorHandler& getInstance();
	
	void registerHandler(const std::string& errorType, 
						ErrorCallback handler,
						ErrorSeverity severity = ErrorSeverity::Error) {
		::utils::TrackedUniqueLock lock(mutex_, "ErrorHandler::mutex_", ::utils::LockLevel::ERROR_HANDLER);
		ErrorConfig config;
		config.handler = std::move(handler);
		config.severity = severity;
		handlers_[errorType] = std::move(config);
	}
	
	void registerRecoveryStrategy(const std::string& errorType, 
								RecoveryCallback strategy);
	
	std::future<void> handleError(const MusicTrainerError& error);
	void setGlobalHandler(ErrorCallback handler);
	
	void clearHandlers();

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return mutex_; }
#endif
	
	// Prevent copying
	ErrorHandler(const ErrorHandler&) = delete;
	ErrorHandler& operator=(const ErrorHandler&) = delete;

private:
	ErrorHandler() = default;
	
	struct ErrorConfig {
		ErrorCallback handler;
		ErrorSeverity severity;
		std::vector<RecoveryCallback> recoveryStrategies;
	};
	
	mutable std::shared_mutex mutex_;  // Level 9 (ERROR_HANDLER) - Shared mutex for better concurrency
	std::unordered_map<std::string, ErrorConfig> handlers_;
	ErrorCallback globalHandler_;
	
	bool attemptRecovery(const std::string& errorType, 
						const MusicTrainerError& error) {
		// Get a copy of recovery strategies first while holding the ErrorHandler lock
		std::vector<RecoveryCallback> strategies;
		{
			::utils::TrackedSharedMutexLock errorLock(mutex_, "ErrorHandler::mutex_", 
				::utils::LockLevel::ERROR_HANDLER);
			
			auto it = handlers_.find(errorType);
			if (it == handlers_.end()) return false;
			
			strategies = it->second.recoveryStrategies;
		} // Release ErrorHandler lock
		
		// Now execute recovery strategies without holding any locks
		for (const auto& strategy : strategies) {
			if (strategy(error)) return true;
		}
		
		return false;
	}

};

// Helper macro for error handling
#define HANDLE_ERROR(error) \
	MusicTrainer::ErrorHandler::getInstance().handleError(error).wait()

} // namespace MusicTrainer