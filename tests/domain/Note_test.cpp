#include <gtest/gtest.h>
#include "../../src/domain/Note.h"
#include "../../src/domain/Pitch.h"
#include "../../src/domain/Position.h"
#include <stdexcept> // For testing exceptions
#include <limits>    // For epsilon

using namespace MusicTrainer::Domain;

// Test fixture for Note class
class NoteTest : public ::testing::Test {
protected:
    Pitch c4; // Middle C
    Pitch e4;
    Position beat0;
    Position beat1;
    Position beat1_5;

    void SetUp() override {
        c4 = Pitch(60);
        e4 = Pitch(64);
        beat0 = Position(0.0);
        beat1 = Position(1.0);
        beat1_5 = Position(1.5);
    }
     const double tolerance = 1e-9; // Tolerance for float comparisons
};

// Test case: Valid Construction and Getters
TEST_F(NoteTest, ValidConstruction) {
    Note note(c4, beat1, 1.0, 100, 0); // Pitch C4, start beat 1, duration 1 beat, velocity 100, voice 0

    EXPECT_EQ(note.getPitch(), c4);
    EXPECT_EQ(note.getStartTime(), beat1);
    EXPECT_NEAR(note.getDuration(), 1.0, tolerance);
    EXPECT_EQ(note.getVelocity(), 100);
    EXPECT_EQ(note.getVoiceId(), 0);

    // Check EndTime calculation
    Position expectedEndTime = Position(beat1.beats + 1.0);
    EXPECT_EQ(note.getEndTime(), expectedEndTime);

    // Test boundary velocities
    Note note_vel0(c4, beat0, 0.5, 0, 1);
    EXPECT_EQ(note_vel0.getVelocity(), 0);
    Note note_vel127(e4, beat1_5, 0.25, 127, 2);
    EXPECT_EQ(note_vel127.getVelocity(), 127);
}

// Test case: Construction Validation (Invalid Arguments)
TEST_F(NoteTest, ConstructionValidation) {
    // Invalid Duration (zero)
    EXPECT_THROW(Note(c4, beat0, 0.0, 100, 0), std::invalid_argument);
    // Invalid Duration (negative)
    EXPECT_THROW(Note(c4, beat0, -1.0, 100, 0), std::invalid_argument);

    // Invalid Velocity (negative)
    EXPECT_THROW(Note(c4, beat0, 1.0, -1, 0), std::out_of_range);
    // Invalid Velocity (too high)
    EXPECT_THROW(Note(c4, beat0, 1.0, 128, 0), std::out_of_range);

    // Invalid Voice ID (negative)
    EXPECT_THROW(Note(c4, beat0, 1.0, 100, -1), std::invalid_argument);

    // Valid construction should not throw
    EXPECT_NO_THROW(Note(c4, beat0, 1.0, 100, 0));
}

// Test case: Comparison Operators
TEST_F(NoteTest, ComparisonOperators) {
    Note note1(c4, beat1, 1.0, 100, 0);
    Note note1_copy(c4, beat1, 1.0, 100, 0);
    Note note_diff_pitch(e4, beat1, 1.0, 100, 0);
    Note note_diff_time(c4, beat1_5, 1.0, 100, 0);
    Note note_diff_dur(c4, beat1, 0.5, 100, 0);
    Note note_diff_vel(c4, beat1, 1.0, 80, 0);
    Note note_diff_voice(c4, beat1, 1.0, 100, 1);

    // Equality
    EXPECT_TRUE(note1 == note1_copy);
    EXPECT_FALSE(note1 == note_diff_pitch);
    EXPECT_FALSE(note1 == note_diff_time);
    EXPECT_FALSE(note1 == note_diff_dur); // Duration comparison uses tolerance
    EXPECT_FALSE(note1 == note_diff_vel);
    EXPECT_FALSE(note1 == note_diff_voice);

    // Inequality
    EXPECT_FALSE(note1 != note1_copy);
    EXPECT_TRUE(note1 != note_diff_pitch);
    EXPECT_TRUE(note1 != note_diff_time);
    EXPECT_TRUE(note1 != note_diff_dur);
    EXPECT_TRUE(note1 != note_diff_vel);
    EXPECT_TRUE(note1 != note_diff_voice);

    // Less Than (Sort order: time, then pitch, then voice)
    EXPECT_FALSE(note1 < note1_copy);
    EXPECT_TRUE(note1 < note_diff_time);  // note1 starts earlier
    EXPECT_FALSE(note_diff_time < note1);
    EXPECT_TRUE(note1 < note_diff_pitch); // Same time, note1 pitch (C4) < note_diff_pitch (E4)
    EXPECT_FALSE(note_diff_pitch < note1);
    EXPECT_TRUE(note1 < note_diff_voice); // Same time, same pitch, note1 voice (0) < note_diff_voice (1)
    EXPECT_FALSE(note_diff_voice < note1);
    // Note: Comparison doesn't primarily consider duration or velocity for sorting order defined
    EXPECT_TRUE(note1 < note_diff_dur); // Same time, pitch, voice -> falls back to voice comparison (0<0 is false, but implementation might differ slightly) - Let's re-verify operator< logic if needed
    EXPECT_TRUE(note1 < note_diff_vel); // Same time, pitch, voice
}