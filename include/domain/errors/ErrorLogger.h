#pragma once
#include "ErrorBase.h"
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>

namespace MusicTrainer {

class ErrorLogger {
public:
	static ErrorLogger& getInstance();
	enum class LogLevel {
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL
	};
	void logError(const MusicTrainerError& error, LogLevel level = LogLevel::ERROR);
	void logCorrelatedError(const MusicTrainerError& error, 
						   const std::string& correlationId,
						   LogLevel level = LogLevel::ERROR);
	void setLogFile(const std::string& path);
	void setLogLevel(LogLevel level) {
		currentLevel = level;
	}
	void enableConsoleOutput(bool enable) {
		consoleOutput = enable;
	}
	void logRecoveryAttempt(const std::string& errorType, 
						   const std::string& strategy,
						   const std::string& details,
						   LogLevel level = LogLevel::INFO);
	void logRecoveryResult(const std::string& errorType,
						  bool success,
						  const std::string& details,
						  LogLevel level = LogLevel::INFO);

private:
	ErrorLogger();
	
	LogLevel currentLevel{LogLevel::INFO};
	bool consoleOutput{true};
	std::unique_ptr<std::ofstream> logFile;
	
	struct ErrorCorrelation {
		std::chrono::system_clock::time_point firstOccurrence{std::chrono::system_clock::now()};
		std::atomic<size_t> count{0};
		std::vector<std::string> contexts;
	};
	
	std::unordered_map<std::string, std::unique_ptr<ErrorCorrelation>> correlatedErrors;

	
	std::string formatLogMessage(const MusicTrainerError& error, 
								 LogLevel level,
								 const std::string& correlationId = "");
	std::string levelToString(LogLevel level);
	void writeLog(const std::string& message);
	std::string formatRecoveryMessage(const std::string& errorType,
									 const std::string& message,
									 const std::string& details);
};

#define LOG_ERROR(error) \
	MusicTrainer::ErrorLogger::getInstance().logError(error)

#define LOG_CORRELATED_ERROR(error, correlationId) \
	MusicTrainer::ErrorLogger::getInstance().logCorrelatedError(error, correlationId)

#define LOG_INFO(error) \
	MusicTrainer::ErrorLogger::getInstance().logError(error, MusicTrainer::ErrorLogger::LogLevel::INFO)

#define LOG_WARNING(error) \
	MusicTrainer::ErrorLogger::getInstance().logError(error, MusicTrainer::ErrorLogger::LogLevel::WARNING)

} // namespace MusicTrainer
