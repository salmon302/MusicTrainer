#include <gtest/gtest.h>
#include "domain/progression/ProgressionSystem.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Duration.h"

class ProgressionSystemTest : public ::testing::Test {
protected:
    using Score = MusicTrainer::music::Score;
    using Voice = MusicTrainer::music::Voice;
    using Duration = MusicTrainer::music::Duration;

    Voice::TimeSignature timeSignature;
    std::unique_ptr<Score> exercise;
    std::unique_ptr<Score> attempt;
    std::unique_ptr<music::progression::ProgressionSystem> system;

    ProgressionSystemTest() 
        : timeSignature(4, Duration::createQuarter()) {}

    void SetUp() override {
        system = music::progression::ProgressionSystem::create();
        exercise = Score::create(timeSignature);
        attempt = Score::create(timeSignature);
    }
};

TEST_F(ProgressionSystemTest, TracksSkillProgression) {
    system->recordExerciseAttempt(*exercise, *attempt, 60.0);
    auto skills = system->getCurrentSkillLevel();
    EXPECT_GE(skills.overallProgress.load(), 0.0);
}

TEST_F(ProgressionSystemTest, ManagesDifficulty) {
    // Get initial difficulty
    double initialDifficulty = system->calculateNextDifficultyLevel();
    
    // Multiple failures should decrease difficulty
    for (int i = 0; i < 3; ++i) {
        system->recordExerciseAttempt(*exercise, *attempt, 30.0);
    }
    
    double currentDifficulty = system->calculateNextDifficultyLevel();
    EXPECT_LE(currentDifficulty, initialDifficulty);
}

TEST_F(ProgressionSystemTest, TracksAnalytics) {
    system->recordExerciseAttempt(*exercise, *attempt, 45.0);
    system->recordExerciseAttempt(*exercise, *attempt, 40.0);
    
    auto analytics = system->getAnalytics();
    EXPECT_EQ(analytics.totalAttempts.load(), 2);
    EXPECT_GT(analytics.averageAccuracy.load(), 0.0);
}
