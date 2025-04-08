#include <gtest/gtest.h>
#include "../../src/rules/ParallelFifthsRule.h"
#include "../../src/rules/RuleTypes.h"
#include "../../src/domain/Score.h"
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"

using namespace MusicTrainer::Domain;
using namespace MusicTrainer::Rules;

class ParallelFifthsRuleTest : public ::testing::Test {
protected:
    ParallelFifthsRule rule;
    Score score;
    ValidationResult result;

    // Pitches (Middle C = 60)
    Pitch C4{60}, D4{62}, E4{64}, F4{65}, G4{67}, A4{69}, B4{71}, C5{72};

    // Positions (beats)
    Position p0{0.0}, p1{1.0}, p2{2.0}, p3{3.0}, p4{4.0};

    void SetUp() override {
        // Reset score and result before each test
        score = Score(); // Re-initializes with defaults
        result.clear();
    }
};

// Test case: No parallel fifths
TEST_F(ParallelFifthsRuleTest, NoParallelFifths) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C-G (P5) -> D-A (P5) - PARALLEL FIFTH (Should fail if logic was wrong)
    // Let's make it C-G (P5) -> D-G (P4) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(G4, p1, 1.0, 100, v1)); // Changed A4 to G4

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Simple Parallel Fifths
TEST_F(ParallelFifthsRuleTest, SimpleParallelFifths) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-G4 (P5) -> D4-A4 (P5)
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(A4, p1, 1.0, 100, v1));

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1);
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
    EXPECT_EQ(result.violations[0].severity, Severity::Error);
    // Check involved notes? Might be tricky due to pointers
}

// Test case: Parallel Fifths with different octave
TEST_F(ParallelFifthsRuleTest, OctaveParallelFifths) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-G4 (P5) -> D4-A5 (P5 + Octave) - Still parallel fifths
    Pitch A5{81};
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(A5, p1, 1.0, 100, v1));

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1);
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
}

// Test case: Repeated Perfect Fifth (Not Parallel)
TEST_F(ParallelFifthsRuleTest, RepeatedPerfectFifth) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-G4 (P5) -> C4-G4 (P5) - Same notes, not parallel motion
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(C4, p1, 1.0, 100, v0)); // Repeated C4
    score.addNote(Note(G4, p1, 1.0, 100, v1)); // Repeated G4

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Perfect Fifth separated by other interval
TEST_F(ParallelFifthsRuleTest, InterruptedPerfectFifth) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();

    // C4-G4 (P5) -> D4-G4 (P4) -> E4-B4 (P5) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(G4, p1, 1.0, 100, v1)); // P4
    score.addNote(Note(E4, p2, 1.0, 100, v0));
    score.addNote(Note(B4, p2, 1.0, 100, v1)); // P5

    rule.validate(score, result);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.violations.size(), 0);
}

// Test case: Three voices - check pairs
TEST_F(ParallelFifthsRuleTest, ThreeVoices) {
    int v0 = score.addVoice();
    int v1 = score.addVoice();
    int v2 = score.addVoice();

    // V0/V1: C-G (P5) -> D-A (P5) - Parallel
    // V0/V2: C-C (P8) -> D-D (P8) - Parallel Octave (not checked by this rule yet)
    // V1/V2: G-C (P4) -> A-D (P4) - OK
    score.addNote(Note(C4, p0, 1.0, 100, v0));
    score.addNote(Note(G4, p0, 1.0, 100, v1));
    score.addNote(Note(C5, p0, 1.0, 100, v2)); // Octave above v0

    score.addNote(Note(D4, p1, 1.0, 100, v0));
    score.addNote(Note(A4, p1, 1.0, 100, v1)); // Parallel 5th with v0
    score.addNote(Note(D4, p1, 1.0, 100, v2)); // Parallel Octave with v0, P4 with v1

    rule.validate(score, result);
    EXPECT_FALSE(result.isValid);
    ASSERT_EQ(result.violations.size(), 1); // Only expects the P5 violation
    EXPECT_EQ(result.violations[0].ruleId, rule.getId());
    // Check that the violation message mentions v0 and v1 (or their IDs)
    EXPECT_NE(result.violations[0].description.find("voice 0"), std::string::npos);
    EXPECT_NE(result.violations[0].description.find("voice 1"), std::string::npos);
}

// TODO: Add tests for contrary motion fifths (allowed)
// TODO: Add tests with rests interrupting the parallel motion
// TODO: Add tests with more complex rhythms and overlapping notes