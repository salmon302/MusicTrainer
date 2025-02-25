#include <gtest/gtest.h>
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/DefaultErrorHandlers.h"

class TestEnvironment : public ::testing::Environment {
public:
	void SetUp() override {
		auto& handler = MusicTrainer::ErrorHandler::getInstance();
		handler.clearHandlers();
		MusicTrainer::configureDefaultErrorHandlers();
	}
	
	void TearDown() override {
		auto& handler = MusicTrainer::ErrorHandler::getInstance();
		handler.clearHandlers();
	}
};

// Register the test environment
testing::Environment* const env = testing::AddGlobalTestEnvironment(new TestEnvironment);