#include "domain/rules/ValidationPipeline.h"
#include "domain/rules/ParallelFifthsRule.h"
#include "domain/rules/ParallelOctavesRule.h"
#include "domain/rules/VoiceLeadingRule.h"
#include "domain/music/Voice.h"
#include "domain/music/Score.h"
#include "domain/music/Pitch.h"
#include "domain/music/Duration.h"
#include "domain/music/Interval.h"
#include "domain/errors/DomainErrors.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace MusicTrainer::music;
using namespace MusicTrainer::music::rules;

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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(d4, quarter.getTotalBeats());
	
	voice2->addNote(f3, quarter.getTotalBeats());
	voice2->addNote(g3, quarter.getTotalBeats());

	
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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(d4, quarter.getTotalBeats());
	
	voice2->addNote(c3, quarter.getTotalBeats());
	voice2->addNote(d3, quarter.getTotalBeats());

	
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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(d4, quarter.getTotalBeats());
	voice1->addNote(e4, quarter.getTotalBeats());
	voice1->addNote(f4, quarter.getTotalBeats());
	
	voice2->addNote(c3, quarter.getTotalBeats());
	voice2->addNote(d3, quarter.getTotalBeats());
	voice2->addNote(e3, quarter.getTotalBeats());
	voice2->addNote(f3, quarter.getTotalBeats());

	
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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(d4, quarter.getTotalBeats());
	
	voice2->addNote(g3, quarter.getTotalBeats());
	voice2->addNote(f3, quarter.getTotalBeats());

	
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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(g3, quarter.getTotalBeats());
	
	voice2->addNote(a3, quarter.getTotalBeats());
	voice2->addNote(b3, quarter.getTotalBeats());

	
	score->addVoice(std::move(voice1));
	score->addVoice(std::move(voice2));
	
	pipeline->addRule(VoiceLeadingRule::create());  // Remove parameter, using default max leap
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
	
	auto quarter = Duration::createQuarter();
	voice->addNote(c4, quarter.getTotalBeats());
	voice->addNote(e5, quarter.getTotalBeats());

	
	score->addVoice(std::move(voice));
	
	pipeline->addRule(VoiceLeadingRule::create());  // Remove parameter, using default max leap
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
	
	auto quarter = Duration::createQuarter();
	voice1->addNote(c4, quarter.getTotalBeats());
	voice1->addNote(d4, quarter.getTotalBeats());
	
	voice2->addNote(g3, quarter.getTotalBeats());
	voice2->addNote(a3, quarter.getTotalBeats());

	
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
	auto quarter = Duration::createQuarter();
	soprano->addNote(c4, quarter.getTotalBeats());
	soprano->addNote(g5, quarter.getTotalBeats());  // This creates a large leap

	
	auto alto = Voice::create(timeSignature);
	auto f3 = Pitch::create(Pitch::NoteName::F, 3);
	auto c4_alto = Pitch::create(Pitch::NoteName::C, 4);  // Creates parallel fifths with soprano
	alto->addNote(f3, quarter.getTotalBeats());
	alto->addNote(c4_alto, quarter.getTotalBeats());

	
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
	
	auto voiceLeading = VoiceLeadingRule::create();  // Remove parameter
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

TEST(ValidationPipelineTest, MelodicIntervalRuleTest) {
    auto pipeline = ValidationPipeline::create();
    auto score = std::make_unique<Score>();
    
    // Add a melodic interval rule
    auto melodicRule = std::make_unique<MelodicIntervalRule>();
    pipeline->addRule(std::move(melodicRule));
    pipeline->compileRules();
    
    // Add two voices
    score->addVoice();
    auto& voice = score->getVoice(0);
    
    // Test valid stepwise motion
    voice.addNote(60, 1.0); // Middle C
    voice.addNote(62, 1.0); // D
    EXPECT_TRUE(pipeline->validate(*score));
    
    // Test invalid large leap
    voice.addNote(76, 1.0); // E5 - octave plus major third
    EXPECT_FALSE(pipeline->validate(*score));
    
    // Test invalid diminished fifth
    score = std::make_unique<Score>();
    score->addVoice();
    auto& voice2 = score->getVoice(0);
    voice2.addNote(60, 1.0); // C
    voice2.addNote(66, 1.0); // F#
    EXPECT_FALSE(pipeline->validate(*score));
}

TEST(ValidationPipelineTest, DissonancePreparationRuleTest) {
    auto pipeline = ValidationPipeline::create();
    auto score = std::make_unique<Score>();
    
    // Add the dissonance preparation rule
    auto dissonanceRule = std::make_unique<DissonancePreparationRule>();
    pipeline->addRule(std::move(dissonanceRule));
    pipeline->compileRules();
    
    // Add two voices
    score->addVoice();
    score->addVoice();
    auto& voice1 = score->getVoice(0);
    auto& voice2 = score->getVoice(1);
    
    // Test valid preparation of dissonance
    voice1.addNote(60, 1.0); // C
    voice1.addNote(62, 1.0); // D
    voice1.addNote(60, 1.0); // C
    
    voice2.addNote(64, 1.0); // E
    voice2.addNote(65, 1.0); // F
    voice2.addNote(64, 1.0); // E
    
    EXPECT_TRUE(pipeline->validate(*score));
    
    // Test invalid unprepared dissonance
    score = std::make_unique<Score>();
    score->addVoice();
    score->addVoice();
    auto& voice3 = score->getVoice(0);
    auto& voice4 = score->getVoice(1);
    
    voice3.addNote(60, 1.0); // C
    voice3.addNote(61, 1.0); // C# - forms dissonant interval
    
    voice4.addNote(64, 1.0); // E
    voice4.addNote(64, 1.0); // E
    
    EXPECT_FALSE(pipeline->validate(*score));
}

TEST(RuleSettingsTest, ManagesRuleStates) {
    auto& settings = RuleSettings::instance();
    settings.resetToDefaults();
    
    // Check default states
    EXPECT_TRUE(settings.isRuleEnabled("ParallelFifthsRule"));
    EXPECT_TRUE(settings.isRuleEnabled("MelodicIntervalRule"));
    EXPECT_TRUE(settings.isRuleEnabled("DissonancePreparationRule"));
    
    // Test disabling rules
    settings.setRuleEnabled("MelodicIntervalRule", false);
    EXPECT_FALSE(settings.isRuleEnabled("MelodicIntervalRule"));
    EXPECT_TRUE(settings.isRuleEnabled("ParallelFifthsRule")); // Other rules unaffected
    
    // Test re-enabling rules
    settings.setRuleEnabled("MelodicIntervalRule", true);
    EXPECT_TRUE(settings.isRuleEnabled("MelodicIntervalRule"));
    
    // Test unknown rule (should default to enabled)
    EXPECT_TRUE(settings.isRuleEnabled("UnknownRule"));
}

TEST(RuleSettingsTest, IntegratesWithValidationPipeline) {
    auto pipeline = ValidationPipeline::create();
    auto score = std::make_unique<Score>();
    
    // Add voices for testing
    score->addVoice();
    score->addVoice();
    auto& voice1 = score->getVoice(0);
    auto& voice2 = score->getVoice(1);
    
    // Add notes that create parallel fifths
    voice1.addNote(60, 1.0); // C4
    voice1.addNote(62, 1.0); // D4
    
    voice2.addNote(53, 1.0); // F3
    voice2.addNote(55, 1.0); // G3
    
    // Add rules
    auto fifthsRule = std::make_unique<ParallelFifthsRule>();
    auto melodicRule = std::make_unique<MelodicIntervalRule>();
    
    pipeline->addRule(std::move(fifthsRule));
    pipeline->addRule(std::move(melodicRule));
    pipeline->compileRules();
    
    // Test with all rules enabled
    RuleSettings::instance().resetToDefaults();
    bool result = pipeline->validate(*score);
    EXPECT_FALSE(result); // Should fail due to parallel fifths
    
    // Test with parallel fifths rule disabled
    RuleSettings::instance().setRuleEnabled("ParallelFifthsRule", false);
    result = pipeline->validate(*score);
    EXPECT_TRUE(result); // Should pass now
    
    // Add invalid melodic interval
    voice1.addNote(76, 1.0); // E5 - large leap
    result = pipeline->validate(*score);
    EXPECT_FALSE(result); // Should fail due to melodic rule
    
    // Disable melodic rule too
    RuleSettings::instance().setRuleEnabled("MelodicIntervalRule", false);
    result = pipeline->validate(*score);
    EXPECT_TRUE(result); // Should pass with both rules disabled
}

