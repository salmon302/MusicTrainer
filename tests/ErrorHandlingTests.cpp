#include "gtest/gtest.h"
#include "domain/errors/ErrorLogger.h"
#include "domain/errors/RecoveryStrategy.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/DefaultErrorHandlers.h"
#include <thread>
#include <fstream>
#include <string>

using namespace MusicTrainer;

class ErrorHandlingTestFixture : public ::testing::Test {
protected:
	void SetUp() override {
		configureDefaultErrorHandlers();
	}
	
	void TearDown() override {
		// Clean up any test files
		std::remove("test_log.txt");
		std::remove("test_correlated_log.txt");
		std::remove("test_recovery_log.txt");
	}
};

TEST_F(ErrorHandlingTestFixture, LogError) {
	auto& logger = ErrorLogger::getInstance();
	logger.setLogLevel(ErrorLogger::LogLevel::DEBUG);
	logger.setLogFile("test_log.txt");
	
	DomainError error("Test error", "TestError");
	logger.logError(error, ErrorLogger::LogLevel::ERROR);
	
	std::ifstream logFile("test_log.txt");
	ASSERT_TRUE(logFile.is_open());
	std::string line;
	std::getline(logFile, line);
	ASSERT_FALSE(line.empty());
	ASSERT_TRUE(line.find("Test error") != std::string::npos);
}

TEST_F(ErrorHandlingTestFixture, LogCorrelatedError) {
	auto& logger = ErrorLogger::getInstance();
	logger.setLogFile("test_correlated_log.txt");
	
	DomainError error1("Test error 1", "TestError");
	DomainError error2("Test error 2", "TestError");
	
	logger.logCorrelatedError(error1, "correlation1");
	logger.logCorrelatedError(error2, "correlation1");
	
	std::ifstream logFile("test_correlated_log.txt");
	ASSERT_TRUE(logFile.is_open());
	
	std::string line1, line2;
	std::getline(logFile, line1);
	std::getline(logFile, line2);
	
	ASSERT_TRUE(line1.find("correlation1") != std::string::npos);
	ASSERT_TRUE(line2.find("correlation1") != std::string::npos);
}

TEST_F(ErrorHandlingTestFixture, RetryStrategy) {
	auto& strategy = RecoveryStrategy::getInstance();
	
	int attempts = 0;
	RecoveryStrategy::RecoveryAction action = [&attempts](const MusicTrainerError&) {
		attempts++;
		return attempts == 3;  // Succeed on third attempt
	};
	
	strategy.registerStrategy("TestError", 
							RecoveryStrategy::StrategyType::RETRY,
							action);
	
	DomainError error("Test error", "TestError");
	auto result = strategy.attemptRecovery(error);
	
	ASSERT_TRUE(result.successful);
	ASSERT_EQ(attempts, 3);
}

TEST_F(ErrorHandlingTestFixture, CircuitBreakerStrategy) {
	auto& strategy = RecoveryStrategy::getInstance();
	
	RecoveryStrategy::RecoveryAction action = [](const MusicTrainerError&) {
		return false;  // Always fail
	};
	
	strategy.registerStrategy("test_error",
							RecoveryStrategy::StrategyType::CIRCUIT_BREAKER,
							action);
	
	DomainError error("Test error", "test_error");
	
	// Should fail and eventually open circuit
	for (int i = 0; i < 10; i++) {
		auto result = strategy.attemptRecovery(error);
		ASSERT_FALSE(result.successful);
	}
}

TEST_F(ErrorHandlingTestFixture, ErrorHandling) {
	auto& handler = ErrorHandler::getInstance();
	bool handlerCalled = false;
	
	// Register a handler for TestError
	handler.registerHandler("TestError",
		[&handlerCalled](const MusicTrainerError&) {
			handlerCalled = true;
		},
		ErrorSeverity::Error);
	
	// Register a recovery strategy that will fail, ensuring the handler is called
	auto& strategy = RecoveryStrategy::getInstance();
	strategy.registerStrategy("TestError", 
							RecoveryStrategy::StrategyType::RETRY,
							[](const MusicTrainerError&) { return false; });  // Always fail
	
	DomainError error("Test error", "TestError");
	auto future = handler.handleError(error);
	future.wait(); // Wait for async completion
	
	ASSERT_TRUE(handlerCalled);
}

TEST_F(ErrorHandlingTestFixture, RecoveryIntegration) {
	auto& handler = ErrorHandler::getInstance();
	auto& strategy = RecoveryStrategy::getInstance();
	
	bool recoveryAttempted = false;
	RecoveryStrategy::RecoveryAction action = [&recoveryAttempted](const MusicTrainerError&) {
		recoveryAttempted = true;
		return true;
	};
	
	strategy.registerStrategy("TestError",
							RecoveryStrategy::StrategyType::RETRY,
							action);
	
	handler.registerHandler("TestError",
		[](const MusicTrainerError&) {},
		ErrorSeverity::Error);
	
	DomainError error("Test error", "TestError");
	handler.handleError(error);
	
	ASSERT_TRUE(recoveryAttempted);
}

TEST_F(ErrorHandlingTestFixture, RecoveryLogging) {
	auto& logger = ErrorLogger::getInstance();
	logger.setLogFile("test_recovery_log.txt");
	
	logger.logRecoveryAttempt("TestError", "RETRY", "Testing recovery attempt");
	logger.logRecoveryResult("TestError", true, "Recovery succeeded");
	
	std::ifstream logFile("test_recovery_log.txt");
	ASSERT_TRUE(logFile.is_open());
	
	std::string line1, line2;
	std::getline(logFile, line1);
	std::getline(logFile, line2);
	
	ASSERT_TRUE(line1.find("[RECOVERY]") != std::string::npos);
	ASSERT_TRUE(line1.find("Testing recovery attempt") != std::string::npos);
	ASSERT_TRUE(line2.find("Recovery succeeded") != std::string::npos);
	
	std::remove("test_recovery_log.txt");
}

TEST_F(ErrorHandlingTestFixture, DefaultHandlersConfigured) {
	auto& handler = ErrorHandler::getInstance();
	auto& logger = ErrorLogger::getInstance();
	auto& strategy = RecoveryStrategy::getInstance();
	
	// Test MIDI error handling
	DomainError midiError("Test MIDI error", "MidiError");
	ASSERT_NO_THROW(handler.handleError(midiError));
	
	// Test state error handling
	DomainError stateError("Test state error", "StateError");
	ASSERT_NO_THROW(handler.handleError(stateError));
	
	// Test plugin error handling
	DomainError pluginError("Test plugin error", "PluginError");
	ASSERT_NO_THROW(handler.handleError(pluginError));
	
	// Test repository error handling
	DomainError repoError("Test repository error", "RepositoryError");
	ASSERT_NO_THROW(handler.handleError(repoError));
	
	// Test validation error handling
	DomainError validationError("Test validation error", "ValidationError");
	ASSERT_NO_THROW(handler.handleError(validationError));
	
	// Test unhandled error type
	DomainError unknownError("Test unknown error", "UnknownError");
	ASSERT_NO_THROW(handler.handleError(unknownError));
}