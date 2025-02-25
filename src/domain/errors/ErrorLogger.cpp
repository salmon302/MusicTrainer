#include "domain/errors/ErrorLogger.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <memory>

namespace MusicTrainer {

ErrorLogger& ErrorLogger::getInstance() {
	static ErrorLogger instance;
	return instance;
}

ErrorLogger::ErrorLogger() = default;

void ErrorLogger::logError(const MusicTrainerError& error, LogLevel level) {
	if (level >= currentLevel) {
		writeLog(formatLogMessage(error, level));
	}
}

void ErrorLogger::logCorrelatedError(const MusicTrainerError& error, 
								   const std::string& correlationId,
								   LogLevel level) {
	if (level >= currentLevel) {
		auto& correlation = correlatedErrors[correlationId];
		if (!correlation) {
			correlation = std::make_unique<ErrorCorrelation>();
			correlation->firstOccurrence = std::chrono::system_clock::now();
		}
		correlation->count++;
		correlation->contexts.push_back(error.what());
		writeLog(formatLogMessage(error, level, correlationId));
	}
}

void ErrorLogger::setLogFile(const std::string& path) {
	logFile = std::make_unique<std::ofstream>(path, std::ios::app);
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
		const auto& correlation = correlatedErrors[correlationId];
		ss << "[Correlation: " << correlationId 
		   << ", Count: " << correlation->count << "] ";
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
	if (message.find("ValidationError") != std::string::npos) {
		if (consoleOutput) {
			std::cerr << message << std::endl;
		}
		return;
	}
	
	if (logFile && logFile->is_open()) {
		*logFile << message << std::endl;
		logFile->flush();
	}
	
	if (consoleOutput) {
		std::cerr << message << std::endl;
	}
}

void ErrorLogger::logRecoveryAttempt(const std::string& errorType,
								   const std::string& strategy,
								   const std::string& details,
								   LogLevel level) {
	if (level >= currentLevel) {
		writeLog(formatRecoveryMessage(errorType, "[RECOVERY] " + strategy, details));
	}
}

void ErrorLogger::logRecoveryResult(const std::string& errorType,
								  bool success,
								  const std::string& details,
								  LogLevel level) {
	if (level >= currentLevel) {
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
