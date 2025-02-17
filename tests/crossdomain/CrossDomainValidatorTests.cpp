#include <gtest/gtest.h>
#include "domain/crossdomain/CrossDomainValidator.h"
#include "domain/rules/ValidationPipeline.h"
#include "adapters/InMemoryScoreRepository.h"
#include "domain/events/NoteAddedEvent.h"
#include "domain/events/EventStore.h"
#include "domain/progression/ProgressionSystem.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include <thread>
#include <atomic>
#include <iostream>

using namespace music;
using namespace music::crossdomain;
using namespace music::adapters;
using namespace music::rules;
using namespace music::events;
using namespace music::progression;

class CrossDomainValidatorTest : public ::testing::Test {
protected:
	void SetUp() override {
		try {
			repository = InMemoryScoreRepository::create();
			eventStore = EventStore::create();
			progressionSystem = ProgressionSystem::create();
			pipeline = ValidationPipeline::create();
			validator = std::make_unique<CrossDomainValidator>(*pipeline, *repository);
		} catch (const std::exception& e) {
			std::cerr << "Setup error: " << e.what() << std::endl;
			throw;
		}
	}

	std::unique_ptr<InMemoryScoreRepository> repository;
	std::unique_ptr<ValidationPipeline> pipeline;
	std::unique_ptr<CrossDomainValidator> validator;
	std::unique_ptr<EventStore> eventStore;
	std::unique_ptr<ProgressionSystem> progressionSystem;

};

TEST_F(CrossDomainValidatorTest, ValidatesScoreWithProgressionHistory) {
	auto score = Score::create();
	auto voice = Voice::create();
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	score->addVoice(std::move(voice));
	
	progressionSystem->recordExerciseAttempt(*score, *score, 0.95);
	auto result = validator->validate(*score);
	EXPECT_TRUE(result.isValid);
}

TEST_F(CrossDomainValidatorTest, ValidatesScoreWithEventHistory) {
	auto score = Score::create();
	auto voice = Voice::create();
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	score->addVoice(std::move(voice));
	
	auto event = NoteAddedEvent::create(0, Pitch::create(Pitch::NoteName::C, 4), 
									  Duration::createQuarter(), Voice::TimeSignature(4, Duration::createQuarter()),
									  "test_correlation");
	eventStore->addEvent(std::move(event));
	
	auto result = validator->validate(*score);
	EXPECT_TRUE(result.isValid);
}

TEST_F(CrossDomainValidatorTest, ValidatesComplexScoreAcrossDomains) {
	auto score = Score::create();
	auto voice1 = Voice::create();
	auto voice2 = Voice::create();
	
	// Add notes to voices
	voice1->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	voice1->addNote(Pitch::create(Pitch::NoteName::D, 4), Duration::createQuarter());
	
	voice2->addNote(Pitch::create(Pitch::NoteName::F, 3), Duration::createQuarter());
	voice2->addNote(Pitch::create(Pitch::NoteName::G, 3), Duration::createQuarter());
	
	// Add voices in order (lower voice first)
	score->addVoice(std::move(voice2));  // Lower voice (F3 -> G3)
	score->addVoice(std::move(voice1));  // Upper voice (C4 -> D4)
	
	// Add progression history
	progressionSystem->recordExerciseAttempt(*score, *score, 0.85);
	
	// Add event history
	auto event = NoteAddedEvent::create(0, Pitch::create(Pitch::NoteName::C, 4),
									  Duration::createQuarter(), Voice::TimeSignature(4, Duration::createQuarter()),
									  "test_correlation");
	eventStore->addEvent(std::move(event));
	
	auto result = validator->validate(*score);
	EXPECT_FALSE(result.isValid);
	EXPECT_NE(result.message.find("parallel fifths"), std::string::npos);
}

TEST_F(CrossDomainValidatorTest, HandlesEdgeCases) {
	// Empty score
	auto emptyScore = Score::create();
	auto emptyResult = validator->validate(*emptyScore);
	EXPECT_TRUE(emptyResult.isValid);
	
	// Score with maximum voices
	auto maxScore = Score::create();
	for (int i = 0; i < 100; i++) {
		auto voice = Voice::create();
		voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
		maxScore->addVoice(std::move(voice));
	}
	auto maxResult = validator->validate(*maxScore);
	EXPECT_TRUE(maxResult.isValid);
}

TEST_F(CrossDomainValidatorTest, ValidatesWithConcurrentOperations) {
	const int NUM_THREADS = 4;
	auto score = Score::create();
	auto voice = Voice::create();
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	score->addVoice(std::move(voice));

	
	std::vector<std::thread> threads;
	std::atomic<int> validationCount{0};
	
	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back([this, &score, &validationCount]() {
			try {
				auto result = validator->validate(*score);
				if (result.isValid) {
					validationCount.fetch_add(1, std::memory_order_relaxed);
				}
			} catch (const std::exception& e) {
				std::cerr << "Validation error: " << e.what() << std::endl;
			}
		});
	}
	
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	
	EXPECT_EQ(validationCount.load(std::memory_order_acquire), NUM_THREADS);
}
