#include <gtest/gtest.h>
#include <future>
#include "domain/exercise/ExerciseGenerator.h"
#include "domain/exercise/ExerciseSettings.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/rules/ValidationPipeline.h"
#include "domain/progression/ProgressionSystem.h"
#include "domain/errors/ErrorHandler.h"

using namespace std::chrono_literals;

namespace {

class ExerciseGenerationIntegrationTest : public ::testing::Test {
protected:
    using Score = MusicTrainer::music::Score;
    using Voice = MusicTrainer::music::Voice;
    using Duration = MusicTrainer::music::Duration;
    using ValidationPipeline = MusicTrainer::music::rules::ValidationPipeline;
    using ExerciseGenerator = MusicTrainer::exercise::ExerciseGenerator;
    using ExerciseSettings = MusicTrainer::exercise::ExerciseSettings;
    using ProgressionSystem = MusicTrainer::progression::ProgressionSystem;

    std::unique_ptr<ValidationPipeline> validationPipeline;
    std::unique_ptr<ExerciseGenerator> exerciseGenerator;
    std::unique_ptr<ProgressionSystem> progressionSystem;

    void SetUp() override {
        validationPipeline = ValidationPipeline::create();
        exerciseGenerator = ExerciseGenerator::create();
        progressionSystem = ProgressionSystem::create();
    }
};

TEST_F(ExerciseGenerationIntegrationTest, GeneratesValidExercises) {
    ExerciseSettings settings;
    settings.setDifficulty(1);
    settings.setVoiceCount(2);
    settings.setMeasureCount(4);
    settings.setTimeSignature({4, Duration::createQuarter()});

    // Generate exercise
    auto exercise = exerciseGenerator->generate(settings);
    ASSERT_TRUE(exercise != nullptr);

    // Validate generated exercise
    bool isValid = validationPipeline->validate(*exercise);
    EXPECT_TRUE(isValid) << "Generated exercise failed validation";

    // Check exercise properties
    EXPECT_EQ(exercise->getVoiceCount(), 2);
    EXPECT_EQ(exercise->getMeasureCount(), 4);
}

TEST_F(ExerciseGenerationIntegrationTest, ProgressiveDifficultyGeneration) {
    const int numLevels = 5;
    std::vector<std::unique_ptr<Score>> exercises;

    // Generate exercises at increasing difficulty
    for (int level = 1; level <= numLevels; ++level) {
        ExerciseSettings settings;
        settings.setDifficulty(level);
        settings.setVoiceCount(2);
        settings.setMeasureCount(4);
        settings.setTimeSignature({4, Duration::createQuarter()});

        auto exercise = exerciseGenerator->generate(settings);
        ASSERT_TRUE(exercise != nullptr);
        exercises.push_back(std::move(exercise));
    }

    // Validate progression complexity
    for (int i = 1; i < exercises.size(); ++i) {
        auto complexity1 = progressionSystem->analyzeComplexity(*exercises[i-1]);
        auto complexity2 = progressionSystem->analyzeComplexity(*exercises[i]);
        EXPECT_GT(complexity2, complexity1) 
            << "Exercise at level " << i+1 
            << " should be more complex than level " << i;
    }
}

TEST_F(ExerciseGenerationIntegrationTest, ConcurrentGeneration) {
    const int numThreads = 4;
    std::vector<std::future<bool>> futures;
    
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this]() {
            try {
                ExerciseSettings settings;
                settings.setDifficulty(1);
                settings.setVoiceCount(2);
                settings.setMeasureCount(4);
                settings.setTimeSignature({4, Duration::createQuarter()});

                auto exercise = exerciseGenerator->generate(settings);
                return exercise != nullptr && validationPipeline->validate(*exercise);
            } catch (const std::exception& e) {
                std::cerr << "Exercise generation failed: " << e.what() << std::endl;
                return false;
            }
        }));
    }

    // All generations should succeed
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

TEST_F(ExerciseGenerationIntegrationTest, ConsistentValidation) {
    ExerciseSettings settings;
    settings.setDifficulty(2);
    settings.setVoiceCount(3);
    settings.setMeasureCount(8);
    settings.setTimeSignature({4, Duration::createQuarter()});

    // Generate and validate multiple times
    const int iterations = 10;
    int validCount = 0;

    for (int i = 0; i < iterations; ++i) {
        auto exercise = exerciseGenerator->generate(settings);
        ASSERT_TRUE(exercise != nullptr);

        if (validationPipeline->validate(*exercise)) {
            validCount++;
        }
    }

    // Expect high consistency in validation results
    double successRate = static_cast<double>(validCount) / iterations;
    EXPECT_GE(successRate, 0.9) << "Exercise generation should have at least 90% success rate";

    // Check validation performance metrics
    const auto& metrics = validationPipeline->getMetrics();
    EXPECT_GT(metrics.cacheHitRate, 0.0) << "Cache should be utilized during validation";
}

TEST_F(ExerciseGenerationIntegrationTest, ErrorHandling) {
    // Test invalid settings
    ExerciseSettings invalidSettings;
    invalidSettings.setVoiceCount(0); // Invalid voice count
    
    EXPECT_THROW(exerciseGenerator->generate(invalidSettings), MusicTrainer::DomainError);

    // Test resource exhaustion
    ExerciseSettings heavySettings;
    heavySettings.setVoiceCount(100); // Excessive voice count
    heavySettings.setMeasureCount(1000); // Excessive measure count

    EXPECT_THROW(exerciseGenerator->generate(heavySettings), MusicTrainer::ResourceError);
}

} // namespace