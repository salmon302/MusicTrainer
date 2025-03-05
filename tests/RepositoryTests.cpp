#include <gtest/gtest.h>
#include "domain/errors/ErrorBase.h"
#include "adapters/EventSourcedRepository.h"
#include "adapters/CachingScoreRepository.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Duration.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/ErrorHandler.h"
#include "domain/errors/ErrorLogger.h"
#include "domain/errors/DomainErrors.h"
#include "utils/DebugUtils.h"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace events = music::events;

class RepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        MusicTrainer::Debug::LockTracker::clearHistory();
    }
    
    void TearDown() override {
        MusicTrainer::Debug::LockTracker::dumpLockHistory();
    }
};

TEST_F(RepositoryTest, HandlesEventsAndSnapshots) {
    std::cout << "[Test] Creating repository and test data...\n";
    auto repo = music::adapters::EventSourcedRepository::create();
    auto timeSignature = MusicTrainer::music::Voice::TimeSignature(4, MusicTrainer::music::Duration::createQuarter());
    auto score = MusicTrainer::music::Score::create(timeSignature);
    auto voice = MusicTrainer::music::Voice::create(timeSignature);
    auto c4 = MusicTrainer::music::Pitch::create(MusicTrainer::music::Pitch::NoteName::C, 4);
    
    std::cout << "[Voice] Adding note C4 with quarter duration\n";
    voice->addNote(c4, MusicTrainer::music::Duration::createQuarter().getTotalBeats());
    
    std::cout << "[Score::addVoice] Adding voice to score\n";
    score->addVoice(std::move(voice));
    std::cout << "[Score::addVoice] Voice added successfully\n";
    
    std::cout << "[EventSourcedRepository::save] Saving score: test_score\n";
    std::cout << "[EventSourcedRepository::save] Creating initial snapshot\n";
    repo->save("test_score", *score);
    std::cout << "[EventSourcedRepository::save] Score saved successfully\n";
    
    std::cout << "[Test] Creating and appending events\n";
    std::vector<std::unique_ptr<events::Event>> events;
    auto quarterDuration = MusicTrainer::music::Duration::createQuarter();
    events.push_back(events::NoteAddedEvent::create(
        0, 
        c4, 
        quarterDuration,
        timeSignature
    ));
    
    std::cout << "[EventSourcedRepository::appendEvents] Appending NoteAddedEvent\n";
    repo->appendEvents("test_score", events);
    
    std::cout << "[EventSourcedRepository::createSnapshot] Creating new snapshot\n";
    repo->createSnapshot("test_score");
    std::cout << "[EventSourcedRepository::createSnapshot] Snapshot created successfully\n";
    
    std::cout << "[EventSourcedRepository::load] Loading score: test_score\n";
    auto loaded = repo->load("test_score");
    std::cout << "[EventSourcedRepository::load] Score loaded successfully\n";
    
    ASSERT_TRUE(loaded != nullptr) << "Loaded score should not be null";
    EXPECT_EQ(loaded->getVoiceCount(), 1) << "Loaded score should have exactly one voice";
}

TEST_F(RepositoryTest, CacheOperations) {
    auto baseRepo = music::adapters::EventSourcedRepository::create();
    auto repo = music::adapters::CachingScoreRepository::create(std::move(baseRepo));
    repo->clearCache();  // Ensure clean state
    
    auto timeSignature = MusicTrainer::music::Voice::TimeSignature(4, MusicTrainer::music::Duration::createQuarter());
    auto score = MusicTrainer::music::Score::create(timeSignature);
    
    // First save and load should be a miss
    repo->save("test_score", *score);
    std::cout << "First load (should be miss)..." << std::endl;
    auto loaded1 = repo->load("test_score");
    auto rate1 = repo->getCacheHitRate();
    std::cout << "Hit rate after first load: " << rate1 << std::endl;
    EXPECT_DOUBLE_EQ(rate1, 0.0);
    
    // Second load should be a hit
    std::cout << "Second load (should be hit)..." << std::endl;
    auto loaded2 = repo->load("test_score");
    auto rate2 = repo->getCacheHitRate();
    std::cout << "Hit rate after second load: " << rate2 << std::endl;
    EXPECT_DOUBLE_EQ(rate2, 0.5);
    
    // Test cache expiration
    repo->setCacheTimeout(1s);
    std::cout << "Waiting for cache expiration..." << std::endl;
    std::this_thread::sleep_for(2s);
    std::cout << "Third load (should be miss due to expiration)..." << std::endl;
    auto loaded3 = repo->load("test_score");
    auto rate3 = repo->getCacheHitRate();
    std::cout << "Hit rate after third load: " << rate3 << std::endl;
    EXPECT_NEAR(rate3, 0.33333333, 0.0001);
}

TEST_F(RepositoryTest, CacheMetrics) {
    auto baseRepo = music::adapters::EventSourcedRepository::create();
    auto repo = music::adapters::CachingScoreRepository::create(std::move(baseRepo));
    auto timeSignature = MusicTrainer::music::Voice::TimeSignature(4, MusicTrainer::music::Duration::createQuarter());
    auto score = MusicTrainer::music::Score::create(timeSignature);
    
    // Save scores first
    for (int i = 0; i < 5; ++i) {
        repo->save("score_" + std::to_string(i), *score);
    }
    
    // Load each score once to populate cache
    for (int i = 0; i < 5; ++i) {
        repo->load("score_" + std::to_string(i));
    }
    
    EXPECT_EQ(repo->getCacheSize(), 5);
    
    // Load each score again to test hit rate
    for (int i = 0; i < 5; ++i) {
        repo->load("score_" + std::to_string(i));
    }
    
    EXPECT_GT(repo->getCacheHitRate(), 0.45);
    
    repo->clearCache();
    EXPECT_EQ(repo->getCacheSize(), 0);
    EXPECT_EQ(repo->getCacheHitRate(), 0.0);
}

TEST_F(RepositoryTest, ErrorHandling) {
    auto baseRepo = music::adapters::EventSourcedRepository::create();
    auto repo = music::adapters::CachingScoreRepository::create(std::move(baseRepo));
    
    // Configure error handling
    auto& errorHandler = MusicTrainer::ErrorHandler::getInstance();
    auto& logger = MusicTrainer::ErrorLogger::getInstance();
    logger.setLogLevel(MusicTrainer::ErrorLogger::LogLevel::DEBUG);
    logger.enableConsoleOutput(true);
    
    bool handlerCalled = false;
    
    // Register error handler that will be called by ErrorHandler::handleError
    errorHandler.registerHandler("RepositoryError",
        [&handlerCalled](const MusicTrainer::MusicTrainerError& error) {
            std::cout << "[TEST] Error handler called for: " << error.what() << std::endl;
            std::this_thread::sleep_for(50ms);
            handlerCalled = true;
        },
        MusicTrainer::ErrorSeverity::Error
    );
    
    // Set the error handler directly on the repository as well
    repo->setErrorHandler([&handlerCalled](const MusicTrainer::MusicTrainerError& error) {
        std::cout << "[TEST] Repository error handler called for: " << error.what() << std::endl;
        std::this_thread::sleep_for(50ms);
        handlerCalled = true;
    });
    
    // Test error handling - this should trigger the error handler we set
    EXPECT_THROW({
        try {
            repo->load("nonexistent_score");
        } catch (const MusicTrainer::RepositoryError& e) {
            EXPECT_EQ(e.getType(), "RepositoryError");
            throw;
        }
    }, MusicTrainer::RepositoryError);
    
    std::cerr << "[TEST] Address of handlerCalled before assertion: " << &handlerCalled << std::endl;
    std::cout << "[TEST] handlerCalled value before assertion: " << handlerCalled << std::endl;
    EXPECT_TRUE(handlerCalled) << "Error handler was not called";
    std::cout << "[TEST] handlerCalled value after assertion: " << handlerCalled << std::endl;
}


