#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <future>
#include <random>

#include "adapters/EventSourcedRepository.h"
#include "adapters/CachingScoreRepository.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorLogger.h"
#include "domain/errors/DomainErrors.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Duration.h"
#include "domain/music/Pitch.h"
#include "domain/rules/VoiceLeadingRule.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "domain/rules/ValidationPipeline.h"

using namespace std::chrono_literals;
using namespace MusicTrainer;

namespace {

class SystemIntegrationTest : public ::testing::Test {
protected:
    using Score = MusicTrainer::music::Score;
    using Voice = MusicTrainer::music::Voice;
    using Duration = MusicTrainer::music::Duration;
    using Pitch = MusicTrainer::music::Pitch;
    using ValidationPipeline = MusicTrainer::music::rules::ValidationPipeline;
    using VoiceLeadingRule = MusicTrainer::music::rules::VoiceLeadingRule;
    using ParallelFifthsRule = MusicTrainer::music::rules::ParallelFifthsRule;
    using ParallelOctavesRule = MusicTrainer::music::rules::ParallelOctavesRule;

    std::unique_ptr<ValidationPipeline> validationPipeline;
    std::unique_ptr<music::adapters::EventSourcedRepository> eventRepo;
    std::unique_ptr<music::adapters::CachingScoreRepository> repository;

    void SetUp() override {
        // Initialize error handling
        ErrorLogger::getInstance().setLogLevel(ErrorLogger::LogLevel::DEBUG);
        ErrorLogger::getInstance().enableConsoleOutput(true);

        // Initialize repositories
        eventRepo = music::adapters::EventSourcedRepository::create();
        repository = music::adapters::CachingScoreRepository::create(std::move(eventRepo));

        // Initialize validation pipeline
        validationPipeline = ValidationPipeline::create();

        // Configure voice leading rules
        auto voiceLeading = VoiceLeadingRule::create();
        std::cout << "[Test] Adding voice leading rule..." << std::endl;
        validationPipeline->addRule(std::move(voiceLeading), {}, 2);
        std::cout << "[Test] Voice leading rule added" << std::endl;

        // Configure parallel fifths rule
        auto parallelFifths = ParallelFifthsRule::create();
        std::cout << "[Test] Adding parallel fifths rule..." << std::endl;
        validationPipeline->addRule(std::move(parallelFifths), {}, 1);
        std::cout << "[Test] Parallel fifths rule added" << std::endl;

        // Configure parallel octaves rule
        auto parallelOctaves = ParallelOctavesRule::create();
        std::cout << "[Test] Adding parallel octaves rule..." << std::endl;
        validationPipeline->addRule(std::move(parallelOctaves), {}, 1);
        std::cout << "[Test] Parallel octaves rule added" << std::endl;
    }
};

TEST_F(SystemIntegrationTest, EndToEndExerciseFlow) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    
    // Create a score
    auto score = Score::create(timeSignature);
    
    // Add a voice
    auto voice = Voice::create(timeSignature);
    
    // Add some notes
    voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter().getTotalBeats());
    voice->addNote(Pitch::create(Pitch::NoteName::E, 4), Duration::createQuarter().getTotalBeats());
    voice->addNote(Pitch::create(Pitch::NoteName::G, 4), Duration::createQuarter().getTotalBeats());
    voice->addNote(Pitch::create(Pitch::NoteName::C, 5), Duration::createQuarter().getTotalBeats());
    
    score->addVoice(std::move(voice));
    
    // Save to repository
    repository->save("test_score", *score);
    
    // Load from repository
    auto loadedScore = repository->load("test_score");
    ASSERT_TRUE(loadedScore != nullptr);
    
    // Validate score
    bool isValid = validationPipeline->validate(*score);
    EXPECT_TRUE(isValid);
}

TEST_F(SystemIntegrationTest, StateManagementAndPersistence) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    
    // Test persistence
    repository->save("test_persistence", *score);
    auto loadedScore = repository->load("test_persistence");
    ASSERT_TRUE(loadedScore != nullptr);
    
    // Test cache behavior
    auto cacheLoadedScore = repository->load("test_persistence");
    ASSERT_TRUE(cacheLoadedScore != nullptr);
}

TEST_F(SystemIntegrationTest, EventProcessingAndValidation) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    
    // Create and process events
    auto noteEvent = music::events::NoteAddedEvent::create(
        0,
        Pitch::create(Pitch::NoteName::C, 4),
        Duration::createQuarter(),
        timeSignature
    );
    
    std::vector<std::unique_ptr<music::events::Event>> events;
    events.push_back(std::move(noteEvent));
    
    repository->save("test_events", *score);
    repository->appendEvents("test_events", events);
}

TEST_F(SystemIntegrationTest, ConcurrentOperations) {
    const int numThreads = 4;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
            auto score = Score::create(timeSignature);
            repository->save("concurrent_test_" + std::to_string(i), *score);
        }));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
}

TEST_F(SystemIntegrationTest, PerformanceMetrics) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    
    // Measure validation performance
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        auto score = Score::create(timeSignature);
        validationPipeline->validate(*score);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Validation performance: " << duration.count() << "ms for 100 iterations" << std::endl;
}

TEST_F(SystemIntegrationTest, CrossDomainInteractions) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    
    // Test interactions between validation and persistence
    bool isValid = validationPipeline->validate(*score);
    EXPECT_TRUE(isValid);
    
    repository->save("cross_domain_test", *score);
    auto loadedScore = repository->load("cross_domain_test");
    ASSERT_TRUE(loadedScore != nullptr);
}

TEST_F(SystemIntegrationTest, EdgeCaseHandling) {
    auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
    auto score = Score::create(timeSignature);
    
    // Empty score should be valid
    EXPECT_TRUE(validationPipeline->validate(*score));
    
    // Score with empty voice
    auto voice = Voice::create(timeSignature);
    score->addVoice(std::move(voice));
    EXPECT_TRUE(validationPipeline->validate(*score));
    
    // Test error handling
    try {
        repository->load("nonexistent_score");
        FAIL() << "Expected RepositoryError";
    } catch (const music::RepositoryError&) {
        // Expected
    }
    
    // Test boundary conditions
    try {
        auto noteEvent = music::events::NoteAddedEvent::create(
            0,
            Pitch::create(Pitch::NoteName::C, 4),
            Duration::createQuarter(),
            timeSignature
        );
        
        std::vector<std::unique_ptr<music::events::Event>> events;
        for (int i = 0; i < 1000; ++i) {  // Test with large number of events
            events.push_back(music::events::NoteAddedEvent::create(
                i,
                Pitch::create(Pitch::NoteName::C, 4),
                Duration::createQuarter(),
                timeSignature
            ));
        }
        
        repository->save("boundary_test", *score);
        repository->appendEvents("boundary_test", events);
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST_F(SystemIntegrationTest, ComprehensivePerformanceValidation) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> noteDist(0, 6);  // C through B
    std::uniform_int_distribution<> octaveDist(4, 5);  // Middle octaves
    
    const int numIterations = 100;
    int validScores = 0;
    
    for (int i = 0; i < numIterations; ++i) {
        try {
            auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
            auto score = Score::create(timeSignature);
            auto voice = Voice::create(timeSignature);
            voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter().getTotalBeats());
            
            score->addVoice(std::move(voice));
            
            if (validationPipeline->validate(*score)) {
                validScores++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Iteration " << i << " failed: " << e.what() << std::endl;
        }
    }
    
    double successRate = static_cast<double>(validScores) / numIterations;
    std::cout << "Performance validation success rate: " << (successRate * 100) << "%" << std::endl;
    EXPECT_GT(successRate, 0.9);  // Expect at least 90% success rate
}

} // namespace
