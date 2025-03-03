#include <gtest/gtest.h>
#include "domain/statesync/StateSynchronizationManager.h"
#include "domain/events/EventBus.h"
#include "domain/events/EventStore.h"
#include "domain/events/ScoreUpdatedEvent.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/errors/ErrorHandler.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

using namespace music;
using namespace music::statesync;
using namespace music::events;

class StateSynchronizationManagerTest : public ::testing::Test {
protected:
	void SetUp() override {
		eventBus = EventBus::create();
		eventStore = EventStore::create();
		manager = std::make_unique<StateSynchronizationManager>(*eventBus);
	}

	std::unique_ptr<EventBus> eventBus;
	std::unique_ptr<EventStore> eventStore;
	std::unique_ptr<StateSynchronizationManager> manager;
};

TEST_F(StateSynchronizationManagerTest, BasicStateSynchronization) {
	std::cout << "[Test] Starting basic state synchronization test\n";
	auto score = Score::create();
	auto voice = Voice::create();
	
	// Add note to voice
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	
	// Add voice to score
	score->addVoice(std::move(voice));
	
	// Synchronize state
	manager->synchronize(*score);
	
	auto events = eventBus->getEvents();
	ASSERT_EQ(events.size(), 1);
	EXPECT_EQ(events[0]->getType(), "ScoreUpdated");
}

TEST_F(StateSynchronizationManagerTest, ConcurrentStateUpdates) {
	const int NUM_THREADS = 4;
	std::atomic<int> updateCount{0};
	std::atomic<int> errorCount{0};
	std::vector<std::thread> threads;
	
	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back([this, &updateCount, &errorCount, i]() {
			try {
				// Each thread creates its own score
				auto score = Score::create();
				auto voice = Voice::create();
				voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
				score->addVoice(std::move(voice));
				manager->synchronize(*score);
				updateCount.fetch_add(1, std::memory_order_relaxed);
			} catch (const std::exception& e) {
				std::cerr << "Thread " << i << " error: " << e.what() << std::endl;
				errorCount.fetch_add(1, std::memory_order_relaxed);
			}
		});
	}
	
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	
	const int totalCount = updateCount.load(std::memory_order_acquire);
	const int errors = errorCount.load(std::memory_order_acquire);
	EXPECT_EQ(errors, 0) << "Encountered " << errors << " errors during concurrent updates";
	EXPECT_EQ(totalCount, NUM_THREADS);
	
	auto events = eventBus->getEvents();
	EXPECT_EQ(events.size(), NUM_THREADS);
}

TEST_F(StateSynchronizationManagerTest, ConflictResolution) {
	auto score1 = Score::create();
	auto score2 = Score::create();
	
	// Add voices to scores
	auto voice1 = Voice::create();
	voice1->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	score1->addVoice(std::move(voice1));
	
	auto voice2 = Voice::create();
	voice2->addNote(Pitch::create(Pitch::NoteName::G, 4), Duration::createQuarter());
	score2->addVoice(std::move(voice2));
	
	// Synchronize both states
	manager->synchronize(*score1);
	manager->synchronize(*score2);
	
	auto events = eventBus->getEvents();
	ASSERT_GE(events.size(), 2);
	EXPECT_EQ(events.back()->getType(), "ScoreUpdated");
}

TEST_F(StateSynchronizationManagerTest, StateRecovery) {
	auto score = Score::create();
	auto voice = Voice::create();
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	score->addVoice(std::move(voice));
	
	// Create snapshot in event store
	eventStore->createSnapshot(*score);
	
	// Simulate state corruption
	score = Score::create();
	
	// Recover state from event store
	auto snapshot = eventStore->getLatestSnapshot();
	ASSERT_TRUE(snapshot.has_value());
	auto recoveredScore = snapshot.value()->reconstruct();
	
	// Synchronize the recovered state
	manager->synchronize(*recoveredScore);
	
	auto events = eventBus->getEvents();
	EXPECT_GE(events.size(), 1);
}

TEST_F(StateSynchronizationManagerTest, EventPropagation) {
	auto score = Score::create();
	bool eventReceived = false;
	
	// Register event handler
	eventBus->subscribe("ScoreUpdated", [&eventReceived](const Event& event) {
		eventReceived = true;
	});
	
	manager->synchronize(*score);
	
	// Allow time for event propagation
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(eventReceived);
}

TEST_F(StateSynchronizationManagerTest, ComplexStateSync) {
	auto score = Score::create();
	
	// Create complex score state
	for (int i = 0; i < 5; i++) {
		auto voice = Voice::create();
		for (int j = 0; j < 4; j++) {
			voice->addNote(
				Pitch::create(static_cast<Pitch::NoteName>(j % 7), 4),
				Duration::createQuarter()
			);
		}
		score->addVoice(std::move(voice));
	}
	
	manager->synchronize(*score);
	
	auto events = eventBus->getEvents();
	EXPECT_GE(events.size(), 1);
	
	auto latestEvent = dynamic_cast<const ScoreUpdatedEvent*>(events.back().get());
	ASSERT_NE(latestEvent, nullptr);
	EXPECT_EQ(latestEvent->getSnapshot().voiceNotes.size(), 5);
}
