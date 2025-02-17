#pragma once
#include "ErrorBase.h"
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace MusicTrainer {

class ErrorLogger {
public:
	static ErrorLogger& getInstance();

	// Log levels
	enum class LogLevel {
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL
	};

	// Log an error with context
	void logError(const MusicTrainerError& error, LogLevel level = LogLevel::ERROR);
	
	// Log error correlation
	void logCorrelatedError(const MusicTrainerError& error, 
						   const std::string& correlationId,
						   LogLevel level = LogLevel::ERROR);

	// Configuration
	void setLogFile(const std::string& path);
	void setLogLevel(LogLevel level);
	void enableConsoleOutput(bool enable);

	// Log recovery attempt
	void logRecoveryAttempt(const std::string& errorType, 
						   const std::string& strategy,
						   const std::string& details,
						   LogLevel level = LogLevel::INFO);
	
	// Log recovery result 
	void logRecoveryResult(const std::string& errorType,
						  bool success,
						  const std::string& details,
						  LogLevel level = LogLevel::INFO);

private:
	ErrorLogger();
	
	std::shared_mutex logMutex_;
	std::ofstream logFile_;
	LogLevel currentLevel_{LogLevel::INFO};
	bool consoleOutput_{true};
	
	// Error correlation tracking
	struct ErrorCorrelation {
		std::chrono::system_clock::time_point firstOccurrence;
		size_t count{0};
		std::vector<std::string> contexts;
	};
	std::unordered_map<std::string, ErrorCorrelation> correlatedErrors_;

	std::string formatLogMessage(const MusicTrainerError& error, 
							   LogLevel level,
							   const std::string& correlationId = "");
	std::string levelToString(LogLevel level);
	void writeLog(const std::string& message);
	std::string formatRecoveryMessage(const std::string& errorType,
									const std::string& message,
									const std::string& details);
};

// Helper macros for logging
#define LOG_ERROR(error) \
	MusicTrainer::ErrorLogger::getInstance().logError(error)

#define LOG_CORRELATED_ERROR(error, correlationId) \
	MusicTrainer::ErrorLogger::getInstance().logCorrelatedError(error, correlationId)

#define LOG_INFO(error) \
    MusicTrainer::ErrorLogger::getInstance().logError(error, MusicTrainer::ErrorLogger::LogLevel::INFO)

#define LOG_WARNING(error) \
    MusicTrainer::ErrorLogger::getInstance().logError(error, MusicTrainer::ErrorLogger::LogLevel::WARNING)

} // namespace MusicTrainer