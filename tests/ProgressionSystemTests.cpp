#include <gtest/gtest.h>
#include "domain/progression/ProgressionSystem.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"

using namespace music;
using namespace music::progression;

class ProgressionSystemTest : public ::testing::Test {
protected:
	ProgressionSystemTest() 
		: timeSignature(4, Duration::createQuarter()) {}
		
	void SetUp() override {
		system = ProgressionSystem::create();
		exercise = Score::create(timeSignature);
		attempt = Score::create(timeSignature);
	}
	
	std::unique_ptr<ProgressionSystem> system;
	Voice::TimeSignature timeSignature;
	std::unique_ptr<Score> exercise;
	std::unique_ptr<Score> attempt;
};

TEST_F(ProgressionSystemTest, TracksSkillProgression) {
	system->recordExerciseAttempt(*exercise, *attempt, 60.0);
	auto skills = system->getCurrentSkillLevel();
	
	EXPECT_GT(skills.pitchAccuracy, 0.0);
	EXPECT_GT(skills.rhythmAccuracy, 0.0);
	EXPECT_GT(skills.speedConsistency, 0.0);
	EXPECT_GT(skills.overallProgress, 0.0);
}

TEST_F(ProgressionSystemTest, ManagesDifficulty) {
	// Record multiple successful attempts
	for (int i = 0; i < 6; i++) {
		system->recordExerciseAttempt(*exercise, *attempt, 30.0);
	}
	
	EXPECT_GT(system->calculateNextDifficultyLevel(), 1.0);
	EXPECT_TRUE(system->shouldIncreaseComplexity());
}

TEST_F(ProgressionSystemTest, TracksAnalytics) {
	system->recordExerciseAttempt(*exercise, *attempt, 45.0);
	system->recordExerciseAttempt(*exercise, *attempt, 40.0);
	
	auto analytics = system->getAnalytics();
	EXPECT_EQ(analytics.totalAttempts, 2);
	EXPECT_GT(analytics.averageAccuracy, 0.0);
	EXPECT_GT(analytics.averageCompletionTime, 0.0);
	EXPECT_EQ(analytics.progressTrend.size(), 2);
}
