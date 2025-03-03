#pragma once
#include "ErrorBase.h"
#include "RecoveryStrategy.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
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
	
	static ErrorHandler& getInstance() {
		static ErrorHandler instance;
		return instance;
	}
	
	void registerHandler(const std::string& errorType, 
						ErrorCallback handler,
						ErrorSeverity severity = ErrorSeverity::Error) {
		auto config = std::make_shared<ErrorConfig>();
		config->handler = std::move(handler);
		config->severity = severity;
		handlers[errorType] = config;
	}
	
	void registerRecoveryStrategy(const std::string& errorType, RecoveryCallback strategy);
	std::future<void> handleError(const MusicTrainerError& error);
	void setGlobalHandler(ErrorCallback handler) {
		globalHandler = std::move(handler);
	}
	void clearHandlers() {
		handlers.clear();
		globalHandler = nullptr;
	}
	
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
	
	std::unordered_map<std::string, std::shared_ptr<ErrorConfig>> handlers;
	ErrorCallback globalHandler;
	
	bool attemptRecovery(const std::string& errorType, const MusicTrainerError& error) {
		auto it = handlers.find(errorType);
		if (it == handlers.end()) return false;
		
		auto& strategies = it->second->recoveryStrategies;
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

