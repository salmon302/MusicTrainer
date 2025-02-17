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

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	testing::AddGlobalTestEnvironment(new TestEnvironment);
	
	// Set test filter to run tests sequentially
	testing::FLAGS_gtest_filter = "*";
	testing::FLAGS_gtest_break_on_failure = true;
	
	return RUN_ALL_TESTS();
}