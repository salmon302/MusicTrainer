#include <gtest/gtest.h>
#include <iostream>
#include "domain/rules/ValidationPipeline.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "domain/rules/VoiceLeadingRule.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/music/Interval.h"

using namespace music;
using namespace music::rules;
using namespace std;

TEST(ValidationPipelineTest, DetectsParallelFifths) {
	auto pipeline = ValidationPipeline::create();
	
	// Create score with 2/4 time signature
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	// Add two voices with same time signature
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Add notes to create parallel fifths in measure 0
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto d4 = Pitch::create(Pitch::NoteName::D, 4);
	auto f3 = Pitch::create(Pitch::NoteName::F, 3);
	auto g3 = Pitch::create(Pitch::NoteName::G, 3);
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(d4, Duration::createQuarter());
	
	voice2->addNote(f3, Duration::createQuarter());
	voice2->addNote(g3, Duration::createQuarter());

	
	// Add voices to score (Score handles its own locking)
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	// Add rule and compile
	pipeline->addRule(ParallelFifthsRule::create());
	pipeline->compileRules();
	
	// Test validation
	std::cout << "[Pipeline] Starting validation...\n";
	bool result = pipeline->validate(*score);
	EXPECT_FALSE(result);
	
	auto violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
	
	// Test cache
	auto metrics = pipeline->getMetrics();
	EXPECT_EQ(metrics.ruleExecutions, 1);
	EXPECT_EQ(metrics.cacheMisses, 1);
	EXPECT_EQ(metrics.cacheHits, 0);
	
	// Test incremental validation
	result = pipeline->validateIncremental(*score, 0);
	EXPECT_FALSE(result);
	
	violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
	
	// Test empty score
	auto emptyScore = Score::create(timeSignature);
	result = pipeline->validate(*emptyScore);
	EXPECT_TRUE(result);
	
	violations = pipeline->getViolations();
	EXPECT_TRUE(violations.empty());
}


// Test detection of parallel octaves in different scenarios
TEST(ValidationPipelineTest, DetectsParallelOctavesBasic) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Create parallel octaves
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto d4 = Pitch::create(Pitch::NoteName::D, 4);
	auto c3 = Pitch::create(Pitch::NoteName::C, 3);
	auto d3 = Pitch::create(Pitch::NoteName::D, 3);
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(d4, Duration::createQuarter());
	
	voice2->addNote(c3, Duration::createQuarter());
	voice2->addNote(d3, Duration::createQuarter());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(ParallelOctavesRule::create());
	pipeline->compileRules();
	
	bool result = pipeline->validate(*score);
	EXPECT_FALSE(result);
	auto violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());

	   // Verify violation message immediately after validation
	   EXPECT_TRUE(violations[0].find("parallel octaves") != std::string::npos);
}

TEST(ValidationPipelineTest, DetectsParallelOctavesComplex) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(4, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Create more complex voice movement with parallel octaves
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto d4 = Pitch::create(Pitch::NoteName::D, 4);
	auto e4 = Pitch::create(Pitch::NoteName::E, 4);
	auto f4 = Pitch::create(Pitch::NoteName::F, 4);
	
	auto c3 = Pitch::create(Pitch::NoteName::C, 3);
	auto d3 = Pitch::create(Pitch::NoteName::D, 3);
	auto e3 = Pitch::create(Pitch::NoteName::E, 3);
	auto f3 = Pitch::create(Pitch::NoteName::F, 3);
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(d4, Duration::createQuarter());
	voice1->addNote(e4, Duration::createQuarter());
	voice1->addNote(f4, Duration::createQuarter());
	
	voice2->addNote(c3, Duration::createQuarter());
	voice2->addNote(d3, Duration::createQuarter());
	voice2->addNote(e3, Duration::createQuarter());
	voice2->addNote(f3, Duration::createQuarter());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(ParallelOctavesRule::create());
	pipeline->compileRules();
	
	// Test full validation
	bool result = pipeline->validate(*score);
	EXPECT_FALSE(result);
	auto violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
	
	// Test incremental validation
	pipeline->clearRuleCache();
	result = pipeline->validateIncremental(*score, 0);  // Changed from 1 to 0
	EXPECT_FALSE(result);
	violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
}

TEST(ValidationPipelineTest, NoParallelOctaves) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Create contrary motion
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto d4 = Pitch::create(Pitch::NoteName::D, 4);
	auto g3 = Pitch::create(Pitch::NoteName::G, 3);
	auto f3 = Pitch::create(Pitch::NoteName::F, 3);
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(d4, Duration::createQuarter());
	
	voice2->addNote(g3, Duration::createQuarter());
	voice2->addNote(f3, Duration::createQuarter());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(ParallelOctavesRule::create());
	pipeline->compileRules();
	
	bool result = pipeline->validate(*score);
	EXPECT_TRUE(result);
	auto violations = pipeline->getViolations();
	EXPECT_TRUE(violations.empty());
}

TEST(ValidationPipelineTest, DetectsVoiceCrossing) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Create voice crossing
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto g3 = Pitch::create(Pitch::NoteName::G, 3);  // Upper voice goes low
	auto a3 = Pitch::create(Pitch::NoteName::A, 3);
	auto b3 = Pitch::create(Pitch::NoteName::B, 3);  // Lower voice stays higher
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(g3, Duration::createQuarter());
	
	voice2->addNote(a3, Duration::createQuarter());
	voice2->addNote(b3, Duration::createQuarter());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(VoiceLeadingRule::create(9));  // Set max leap to 9 semitones
	pipeline->compileRules();
	
	bool result = pipeline->validate(*score);
	EXPECT_FALSE(result);
	auto violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
	
	if (!violations.empty()) {
		EXPECT_TRUE(violations[0].find("voice crossing") != std::string::npos);
	}
}

TEST(ValidationPipelineTest, DetectsLargeLeaps) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice = Voice::create(timeSignature);
	
	// Create large leap
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto e5 = Pitch::create(Pitch::NoteName::E, 5);  // Leap of 15 semitones
	
	voice->addNote(c4, Duration::createQuarter());
	voice->addNote(e5, Duration::createQuarter());

	
	score->addVoice(std::move(voice));
	
	pipeline->addRule(VoiceLeadingRule::create(8));  // Set max leap to 8 semitones (smaller than the 15 semitone leap)
	pipeline->compileRules();
	
	bool result = pipeline->validate(*score);
	EXPECT_FALSE(result);
	auto violations = pipeline->getViolations();
	EXPECT_FALSE(violations.empty());
	
	if (!violations.empty()) {
		EXPECT_TRUE(violations[0].find("large leap") != std::string::npos);
	}
}

TEST(ValidationPipelineTest, AcceptsGoodVoiceLeading) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	auto voice1 = Voice::create(timeSignature);
	auto voice2 = Voice::create(timeSignature);
	
	// Create stepwise motion
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto d4 = Pitch::create(Pitch::NoteName::D, 4);
	auto g3 = Pitch::create(Pitch::NoteName::G, 3);
	auto a3 = Pitch::create(Pitch::NoteName::A, 3);
	
	voice1->addNote(c4, Duration::createQuarter());
	voice1->addNote(d4, Duration::createQuarter());
	
	voice2->addNote(g3, Duration::createQuarter());
	voice2->addNote(a3, Duration::createQuarter());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(VoiceLeadingRule::create());
	pipeline->compileRules();
	
	bool result = pipeline->validate(*score);
	EXPECT_TRUE(result);
	auto violations = pipeline->getViolations();
	EXPECT_TRUE(violations.empty());
}

TEST(ValidationPipelineTest, TracksPerformanceMetrics) {
	auto pipeline = ValidationPipeline::create();
	auto timeSignature = Voice::TimeSignature(2, Duration::createQuarter());
	auto score = Score::create(timeSignature);
	
	// Add voices with both parallel fifths and a large leap
	auto soprano = Voice::create(timeSignature);
	auto c4 = Pitch::create(Pitch::NoteName::C, 4);
	auto g5 = Pitch::create(Pitch::NoteName::G, 5);  // Large leap
	soprano->addNote(c4, Duration::createQuarter());
	soprano->addNote(g5, Duration::createQuarter());  // This creates a large leap

	
	auto alto = Voice::create(timeSignature);
	auto f3 = Pitch::create(Pitch::NoteName::F, 3);
	auto c4_alto = Pitch::create(Pitch::NoteName::C, 4);  // Creates parallel fifths with soprano
	alto->addNote(f3, Duration::createQuarter());
	alto->addNote(c4_alto, Duration::createQuarter());

	
	// Debug output for intervals
	auto firstInterval = Interval::fromPitches(f3, c4);
	std::cout << "First interval: " << firstInterval.toString() << "\n";
	
	// Add voices to score (Score handles its own locking)
	score->addVoice(std::move(soprano));
	score->addVoice(std::move(alto));

	
	std::cout << "\nAdding rules..." << std::endl;
	
	// Add rules with different priorities
	auto parallelFifths = ParallelFifthsRule::create();
	std::cout << "Added ParallelFifthsRule with priority 8" << std::endl;
	pipeline->addRule(std::move(parallelFifths), {}, 8);  // High priority
	
	auto voiceLeading = VoiceLeadingRule::create(8);
	std::cout << "Added VoiceLeadingRule with priority 3" << std::endl;
	pipeline->addRule(std::move(voiceLeading), {}, 3);   // Lower priority
	
	pipeline->compileRules();
	std::cout << "Rules compiled" << std::endl;
	
	// Validate multiple times to test caching
	std::cout << "\nValidating score..." << std::endl;
	bool result = pipeline->validate(*score);
	std::cout << "First validation result: " << (result ? "valid" : "invalid") << std::endl;
	
	auto violations = pipeline->getViolations();
	std::cout << "Violations after first validation:" << std::endl;
	for (const auto& v : violations) {
		std::cout << "- " << v << std::endl;
	}
	
	pipeline->validate(*score);
	pipeline->validate(*score);
	
	// Check metrics
	const auto& metrics = pipeline->getMetrics();
	std::cout << "\nValidation Metrics:" << std::endl;
	std::cout << "Total execution time (us): " << metrics.totalExecutionTime << std::endl;
	std::cout << "Max execution time (us): " << metrics.maxExecutionTime << std::endl;
	std::cout << "Average execution time (us): " << metrics.avgExecutionTime << std::endl;
	std::cout << "Rule executions: " << metrics.ruleExecutions << std::endl;
	std::cout << "Cache hits: " << metrics.cacheHits << std::endl;
	std::cout << "Cache misses: " << metrics.cacheMisses << std::endl;
	std::cout << "Cache hit rate: " << metrics.cacheHitRate << std::endl;
	std::cout << "Total violations: " << metrics.violationsCount << std::endl;
	
	// Verify timing metrics
	EXPECT_GT(metrics.totalExecutionTime, 0) << "Total execution time should be positive";
	EXPECT_GT(metrics.maxExecutionTime, 0) << "Max execution time should be positive";
	EXPECT_GT(metrics.avgExecutionTime, 0) << "Average execution time should be positive";
	
	// Verify execution counts
	EXPECT_EQ(metrics.ruleExecutions, 6) << "Expected 6 rule executions (2 rules * 3 validations)";
	EXPECT_EQ(metrics.cacheHits + metrics.cacheMisses, metrics.ruleExecutions) 
		<< "Sum of cache hits and misses should equal total executions";
	
	// Verify cache performance
	EXPECT_GT(metrics.cacheHits, 0) << "Should have some cache hits after multiple validations";
	EXPECT_GT(metrics.cacheHitRate, 0.0) << "Cache hit rate should be positive";
	
	// Verify rule results
	EXPECT_GT(metrics.violationsCount, 0) << "Should have detected some rule violations";
	EXPECT_FALSE(metrics.ruleTimings.empty()) << "Should have timing data for rules";

	// Check feedback
	const auto& feedback = pipeline->getFeedback();
	std::cout << "\nFeedback items: " << feedback.size() << std::endl;
	for (const auto& item : feedback) {
		std::cout << "Feedback: " << item.message << " (Level: " 
				 << (item.level == FeedbackLevel::ERROR ? "ERROR" : 
					 item.level == FeedbackLevel::WARNING ? "WARNING" : "INFO")
				 << ", Priority: " << item.voiceIndex << ")" << std::endl;
	}
	
	EXPECT_FALSE(feedback.empty());
	
	// Verify feedback levels
	bool hasError = false;
	bool hasWarning = false;
	for (const auto& item : feedback) {
		if (item.level == FeedbackLevel::ERROR) hasError = true;
		if (item.level == FeedbackLevel::WARNING) hasWarning = true;
	}
	EXPECT_TRUE(hasError);  // ParallelFifthsRule should generate ERROR
	EXPECT_TRUE(hasWarning);  // VoiceLeadingRule should generate WARNING
}

