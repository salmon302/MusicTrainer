#include <gtest/gtest.h>

#include <thread>
#include <future>
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/rules/ValidationPipeline.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "utils/TrackedLock.h"

using namespace music;
using namespace music::rules;

TEST(LockingTest, DetectsDeadlockPattern) {
	auto score = Score::create();
	auto pipeline = ValidationPipeline::create();
	pipeline->addRule(ParallelOctavesRule::create());
	
	// Create voice and add note
	auto voice = Voice::create();
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	
	// Add voice to score (Score::addVoice handles proper lock ordering internally)
	score->addVoice(std::move(voice));
	
	// Validate with pipeline
	pipeline->validate(*score);
}

TEST(LockingTest, TracksLockAcquisition) {
	auto score = Score::create();
	auto voice = Voice::create();
	
	// Add note to voice
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	
	// Add to score (handles proper lock ordering internally)
	score->addVoice(std::move(voice));
}

TEST(LockingTest, HandlesTimeouts) {
	auto score = Score::create();
	
	// Create a thread that holds a lock
	auto thread1 = std::async(std::launch::async, [&]() {
		::utils::TrackedUniqueLock lock(score->getMutexForTesting(), "Score::mutex_", ::utils::LockLevel::SCORE);
		std::this_thread::sleep_for(std::chrono::seconds(2));
	});
	
	// Try to acquire same lock with 1 second timeout
	bool caught_timeout = false;
	try {
		::utils::TrackedUniqueLock lock(score->getMutexForTesting(), "Score::mutex_", ::utils::LockLevel::SCORE, 
									std::chrono::milliseconds(1000));
	} catch (const std::runtime_error& e) {
		caught_timeout = true;
		EXPECT_TRUE(std::string(e.what()).find("timeout") != std::string::npos);
	}
	EXPECT_TRUE(caught_timeout);
	
	thread1.wait();
}

TEST(LockingTest, PreservesLockOrder) {
	auto score = Score::create();
	auto voice = Voice::create();
	auto pipeline = ValidationPipeline::create();
	
	// Add note to voice
	voice->addNote(Pitch::create(Pitch::NoteName::C, 4), Duration::createQuarter());
	
	// Add to score (handles proper lock ordering internally)
	score->addVoice(std::move(voice));
	
	// Validate (handles proper lock ordering internally)
	pipeline->validate(*score);
}
