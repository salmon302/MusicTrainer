#include "domain/errors/ErrorLogger.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace MusicTrainer {

ErrorLogger& ErrorLogger::getInstance() {
	static ErrorLogger instance;
	return instance;
}

ErrorLogger::ErrorLogger() = default;

void ErrorLogger::logError(const MusicTrainerError& error, LogLevel level) {
	::utils::TrackedUniqueLock lock(logMutex_, "ErrorLogger::logMutex_", ::utils::LockLevel::ERROR_LOGGING);
	if (level >= currentLevel_) {
		writeLog(formatLogMessage(error, level));
	}
}


void ErrorLogger::logCorrelatedError(const MusicTrainerError& error, 
								   const std::string& correlationId,
								   LogLevel level) {
	::utils::TrackedUniqueLock lock(logMutex_, "ErrorLogger::logMutex_", ::utils::LockLevel::ERROR_LOGGING);
	if (level >= currentLevel_) {
		auto& correlation = correlatedErrors_[correlationId];
		if (correlation.count == 0) {
			correlation.firstOccurrence = std::chrono::system_clock::now();
		}
		correlation.count++;
		correlation.contexts.push_back(error.what());
		writeLog(formatLogMessage(error, level, correlationId));
	}
}


void ErrorLogger::setLogFile(const std::string& path) {
	::utils::TrackedUniqueLock lock(logMutex_, "ErrorLogger::logMutex_", ::utils::LockLevel::ERROR_LOGGING);
	if (logFile_.is_open()) {
		logFile_.close();
	}
	logFile_.open(path, std::ios::app);
}

void ErrorLogger::setLogLevel(LogLevel level) {
	currentLevel_ = level;
}

void ErrorLogger::enableConsoleOutput(bool enable) {
	consoleOutput_ = enable;
}

std::string ErrorLogger::formatLogMessage(const MusicTrainerError& error, 
										LogLevel level,
										const std::string& correlationId) {
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto nowTime = std::chrono::system_clock::to_time_t(now);
	
	ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S")
	   << " [" << levelToString(level) << "] ";
	
	if (!correlationId.empty()) {
		const auto& correlation = correlatedErrors_[correlationId];
		ss << "[Correlation: " << correlationId 
		   << ", Count: " << correlation.count << "] ";
	}
	
	const auto& context = error.getContext();
	ss << context.file << ":" << context.line 
	   << " " << context.function << " - "
	   << error.what();
	
	if (!context.additionalInfo.empty()) {
		ss << "\nAdditional Info: " << context.additionalInfo;
	}
	
	return ss.str();
}

std::string ErrorLogger::levelToString(LogLevel level) {
	switch (level) {
		case LogLevel::DEBUG: return "DEBUG";
		case LogLevel::INFO: return "INFO";
		case LogLevel::WARNING: return "WARNING";
		case LogLevel::ERROR: return "ERROR";
		case LogLevel::CRITICAL: return "CRITICAL";
		default: return "UNKNOWN";
	}
}

void ErrorLogger::writeLog(const std::string& message) {
	// Skip file operations for validation errors
	if (message.find("ValidationError") != std::string::npos) {
		if (consoleOutput_) {
			std::cerr << message << std::endl;
		}
		return;
	}
	
	if (logFile_.is_open()) {
		logFile_ << message << std::endl;
		logFile_.flush();
	}
	
	if (consoleOutput_) {
		std::cerr << message << std::endl;
	}
}

void ErrorLogger::logRecoveryAttempt(const std::string& errorType,
								   const std::string& strategy,
								   const std::string& details,
								   LogLevel level) {
	::utils::TrackedUniqueLock lock(logMutex_, "ErrorLogger::logMutex_", ::utils::LockLevel::ERROR_LOGGING);
	if (level >= currentLevel_) {
		writeLog(formatRecoveryMessage(errorType, "[RECOVERY] " + strategy, details));
	}
}


void ErrorLogger::logRecoveryResult(const std::string& errorType,
								  bool success,
								  const std::string& details,
								  LogLevel level) {
	::utils::TrackedUniqueLock lock(logMutex_, "ErrorLogger::logMutex_", ::utils::LockLevel::ERROR_LOGGING);
	if (level >= currentLevel_) {
		std::string status = success ? "succeeded" : "failed";
		writeLog(formatRecoveryMessage(errorType, "[RECOVERY] " + status, details));
	}
}

std::string ErrorLogger::formatRecoveryMessage(const std::string& errorType,
                                             const std::string& message,
                                             const std::string& details) {
    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    
    ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S")
       << " [RECOVERY] [" << errorType << "] " << message;
    
    if (!details.empty()) {
        ss << " - " << details;
    }
    
    return ss.str();
}

} // namespace MusicTrainer