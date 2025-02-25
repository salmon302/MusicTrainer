#define TESTING
#include "domain/errors/DefaultErrorHandlers.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorLogger.h"
#include "domain/errors/RecoveryStrategy.h"
#include "domain/errors/DomainErrors.h"
#include <iostream>

namespace MusicTrainer {

void configureDefaultErrorHandlers() {
	auto& errorHandler = ErrorHandler::getInstance();
	auto& recoveryStrategy = RecoveryStrategy::getInstance();
	auto& errorLogger = ErrorLogger::getInstance();

	// Configure error logger (lowest level)
	errorLogger.setLogLevel(ErrorLogger::LogLevel::INFO);
	errorLogger.enableConsoleOutput(true);

	// Configure validation error handling
	errorHandler.registerHandler("ValidationError",
		[&errorLogger](const MusicTrainerError& error) {
			std::cerr << "Validation error: " << error.what() << std::endl;
			errorLogger.logError(error, ErrorLogger::LogLevel::INFO);
		},
		ErrorSeverity::Error
	);

	// Configure global error handler
	errorHandler.setGlobalHandler([&errorLogger](const MusicTrainerError& error) {
		std::cerr << "Global error handler: " << error.what() << std::endl;
		errorLogger.logError(error, ErrorLogger::LogLevel::ERROR);
	});

	// Configure test error recovery strategy
	recoveryStrategy.registerStrategy("TestError",
		RecoveryStrategy::StrategyType::RETRY,
		[](const MusicTrainerError&) { return true; }  // Always succeed
	);
}

} // namespace MusicTrainer

