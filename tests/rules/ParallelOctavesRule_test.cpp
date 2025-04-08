#include <gtest/gtest.h>
#include "../../src/rules/ParallelOctavesRule.h"
#include "../../src/rules/RuleTypes.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Rules;

class ParallelOctavesRuleTest : public ::testing::Test {
protected:
    ParallelOctavesRule rule;
    Score score;
    ValidationResult result;

    // Pitches (Middle C = 60)
    Pitch C4{60}, D4{62}, E4{64}, F4{65}, G4{67}, A4{69}, B4{71}, C5{72}, D5{74};

    // Positions (beats)
    Position p0{0.0}, p1{1.0}, p2{2.0}, p3{3.0}, p4{4.0};

    void SetUp() override {
        score = Score();
        result.clear();
    }
};

// Test case: No parallel octaves/unisons
TEST_F(ParallelOctavesRuleTest, NoParallelOctaves) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-C5 (P8) -> D4-C5 (M7) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C5, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(C5, p1, 1.0, 100, v1)); // Changed D5 to C5

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Simple Parallel Octaves
TEST_F(ParallelOctavesRuleTest, SimpleParallelOctaves) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-C5 (P8) -> D4-D5 (P8)
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C5, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(D5, p1, 1.0, 100, v1));

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1);
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
    EXPECT_EQ(result.violations[0].severity, Severity::Error);
    EXPECT_NE(result.violations[0].description.find("octave"), std::string::npos);
}

// Test case: Simple Parallel Unisons
TEST_F(ParallelOctavesRuleTest, SimpleParallelUnisons) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-C4 (P1) -> D4-D4 (P1)
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C4, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(D4, p1, 1.0, 100, v1));

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1);
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
    EXPECT_NE(result.violations[0].description.find("unison"), std::string::npos);
}


// Test case: Repeated Perfect Octave (Not Parallel)
TEST_F(ParallelOctavesRuleTest, RepeatedPerfectOctave) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-C5 (P8) -> C4-C5 (P8) - Same notes, not parallel motion
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C5, p0, 1.0, 100, v1));
    score.addNote(Note(C4, p1, 1.0, 100, v0)); // Repeated C4
    score.addNote(Note(C5, p1, 1.0, 100, v1)); // Repeated C5

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Perfect Octave separated by other interval
TEST_F(ParallelOctavesRuleTest, InterruptedPerfectOctave) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-C5 (P8) -> D4-C5 (M7) -> E4-E5 (P8) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C5, p0, 1.0, 100, v1)); // P8
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(C5, p1, 1.0, 100, v1)); // M7
    score.addNote(Note(E4, p2, 1.0, 100, v0));
    score.addNote(Note(Pitch(E4.getMidiNoteNumber()+12), p2, 1.0, 100, v1)); // E5 - P8

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Three voices - check pairs
TEST_F(ParallelOctavesRuleTest, ThreeVoices) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();
    int v2 = score.addVoice();

    // V0/V1: C4-C5 (P8) -> D4-D5 (P8) - Parallel Octave
    // V0/V2: C4-G4 (P5) -> D4-A4 (P5) - Parallel Fifth (not checked by this rule)
    // V1/V2: C5-G4 (P4) -> D5-A4 (P4) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(C5, p0, 1.0, 100, v1));
    score.addNote(Note(G4, p0, 1.0, 100, v2));

    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(D5, p1, 1.0, 100, v1)); // Parallel Octave with v0
    score.addNote(Note(A4, p1, 1.0, 100, v2)); // Parallel Fifth with v0

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1); // Only expects the P8 violation
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
    EXPECT_NE(result.violations[0].description.find("voice 0"), std::string::npos);
    EXPECT_NE(result.violations[0].description.find("voice 1"), std::string::npos);
    EXPECT_NE(result.violations[0].description.find("octave"), std::string::npos);
}