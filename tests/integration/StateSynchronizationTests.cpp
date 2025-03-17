#include <gtest/gtest.h>
#include <future>
#include "domain/state/StateManager.h"
#include "domain/state/StateSnapshot.h"
#include "domain/state/StateObserver.h"
#include "domain/events/EventBus.h"
#include "domain/music/Score.h"
#include "domain/errors/ErrorHandler.h"
#include "adapters/CachingScoreRepository.h"

using namespace std::chrono_literals;

namespace {

class StateSynchronizationTest : public ::testing::Test {
protected:
    using Score = MusicTrainer::music::Score;
    using Voice = MusicTrainer::music::Voice;
    using StateManager = MusicTrainer::state::StateManager;
    using StateSnapshot = MusicTrainer::state::StateSnapshot;
    using StateObserver = MusicTrainer::state::StateObserver;
    using EventBus = MusicTrainer::events::EventBus;
    using CachingScoreRepository = MusicTrainer::adapters::CachingScoreRepository;

    std::unique_ptr<StateManager> stateManager;
    std::unique_ptr<EventBus> eventBus;
    std::unique_ptr<CachingScoreRepository> repository;

    class TestStateObserver : public StateObserver {
    public:
        void onStateChanged(const StateSnapshot& snapshot) override {
            std::lock_guard<std::mutex> lock(mutex);
            updates.push_back(snapshot);
        }

        std::vector<StateSnapshot> getUpdates() const {
            std::lock_guard<std::mutex> lock(mutex);
            return updates;
        }

    private:
        mutable std::mutex mutex;
        std::vector<StateSnapshot> updates;
    };

    void SetUp() override {
        stateManager = StateManager::create();
        eventBus = EventBus::create();
        repository = CachingScoreRepository::create();
    }
};

TEST_F(StateSynchronizationTest, BasicStateSync) {
    auto observer = std::make_shared<TestStateObserver>();
    stateManager->addObserver(observer);

    // Create initial state
    auto score = Score::create();
    auto voice = Voice::create();
    voice->addNote(60, 1.0);
    score->addVoice(std::move(voice));

    // Update state
    stateManager->updateScore(std::move(score));

    // Verify observer received update
    auto updates = observer->getUpdates();
    ASSERT_EQ(updates.size(), 1);
    EXPECT_TRUE(updates[0].isValid());
}

TEST_F(StateSynchronizationTest, ConcurrentStateAccess) {
    const int numThreads = 4;
    std::vector<std::shared_ptr<TestStateObserver>> observers;
    std::vector<std::future<void>> futures;

    // Add multiple observers
    for (int i = 0; i < numThreads; ++i) {
        auto observer = std::make_shared<TestStateObserver>();
        observers.push_back(observer);
        stateManager->addObserver(observer);
    }

    // Perform concurrent state updates
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            auto score = Score::create();
            auto voice = Voice::create();
            voice->addNote(60 + i, 1.0);
            score->addVoice(std::move(voice));
            stateManager->updateScore(std::move(score));
        }));
    }

    // Wait for all updates
    for (auto& future : futures) {
        future.wait();
    }

    // Verify all observers received all updates
    for (const auto& observer : observers) {
        auto updates = observer->getUpdates();
        EXPECT_EQ(updates.size(), numThreads);
        for (const auto& update : updates) {
            EXPECT_TRUE(update.isValid());
        }
    }
}

TEST_F(StateSynchronizationTest, StateConsistency) {
    auto observer = std::make_shared<TestStateObserver>();
    stateManager->addObserver(observer);

    // Create and save initial state
    auto score = Score::create();
    auto voice = Voice::create();
    voice->addNote(60, 1.0);
    score->addVoice(std::move(voice));
    
    repository->save("test_state", *score);
    stateManager->updateScore(std::move(score));

    // Perform multiple state transitions
    for (int i = 0; i < 5; ++i) {
        auto currentScore = repository->load("test_state");
        ASSERT_TRUE(currentScore != nullptr);

        auto newVoice = Voice::create();
        newVoice->addNote(60 + i, 1.0);
        currentScore->addVoice(std::move(newVoice));

        repository->save("test_state", *currentScore);
        stateManager->updateScore(std::move(currentScore));

        // Verify state consistency
        auto snapshot = stateManager->getCurrentState();
        EXPECT_EQ(snapshot.getScore()->getVoiceCount(), i + 2);
    }

    // Verify observer received all updates in order
    auto updates = observer->getUpdates();
    EXPECT_EQ(updates.size(), 6); // Initial + 5 updates
    for (size_t i = 1; i < updates.size(); ++i) {
        EXPECT_GT(updates[i].getVersion(), updates[i-1].getVersion());
    }
}

TEST_F(StateSynchronizationTest, EventPropagation) {
    auto observer = std::make_shared<TestStateObserver>();
    stateManager->addObserver(observer);

    // Subscribe to events
    int eventCount = 0;
    auto subscription = eventBus->subscribe([&eventCount](const auto& event) {
        eventCount++;
    });

    // Create state changes that generate events
    for (int i = 0; i < 3; ++i) {
        auto score = Score::create();
        auto voice = Voice::create();
        voice->addNote(60 + i, 1.0);
        score->addVoice(std::move(voice));
        
        stateManager->updateScore(std::move(score));
        eventBus->processEvents();
    }

    EXPECT_EQ(eventCount, 3);
    EXPECT_EQ(observer->getUpdates().size(), 3);
}

TEST_F(StateSynchronizationTest, ErrorRecovery) {
    auto observer = std::make_shared<TestStateObserver>();
    stateManager->addObserver(observer);

    // Create initial valid state
    auto score = Score::create();
    auto voice = Voice::create();
    voice->addNote(60, 1.0);
    score->addVoice(std::move(voice));
    
    stateManager->updateScore(std::move(score));

    // Attempt invalid state transition
    try {
        auto invalidScore = Score::create();
        invalidScore->addVoice(nullptr); // Invalid operation
        stateManager->updateScore(std::move(invalidScore));
        FAIL() << "Expected invalid state transition to throw";
    } catch (const MusicTrainer::StateError&) {
        // Expected
    }

    // Verify state remained consistent
    auto currentState = stateManager->getCurrentState();
    EXPECT_TRUE(currentState.isValid());
    EXPECT_EQ(currentState.getScore()->getVoiceCount(), 1);
}

TEST_F(StateSynchronizationTest, PerformanceAndResourceUsage) {
    auto observer = std::make_shared<TestStateObserver>();
    stateManager->addObserver(observer);

    const int numUpdates = 1000;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Perform many rapid state updates
    for (int i = 0; i < numUpdates; ++i) {
        auto score = Score::create();
        auto voice = Voice::create();
        voice->addNote(60 + (i % 12), 1.0);
        score->addVoice(std::move(voice));
        
        stateManager->updateScore(std::move(score));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Performance requirements
    EXPECT_LT(duration.count(), 5000) << "State updates took too long";
    EXPECT_EQ(observer->getUpdates().size(), numUpdates);
}

} // namespace