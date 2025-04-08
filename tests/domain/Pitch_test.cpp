#include <gtest/gtest.h>
#include "../../src/domain/Pitch.h"
#include <stdexcept> // For testing exceptions

using namespace MusicTrainer::Domain;

// Test fixture for Pitch class
class PitchTest : public ::testing::Test {
protected:
    Pitch middleC; // MIDI 60
    Pitch cSharp4; // MIDI 61
    Pitch bFlat3;  // MIDI 58
    Pitch lowest;  // MIDI 0
    Pitch highest; // MIDI 127

    void SetUp() override {
        middleC = Pitch(60);
        cSharp4 = Pitch(61);
        bFlat3 = Pitch(58);
        lowest = Pitch(0);
        highest = Pitch(127);
    }
};

// Test case: Default Construction
TEST_F(PitchTest, DefaultConstruction) {
    Pitch p_default;
    EXPECT_EQ(p_default.getMidiNoteNumber(), 60); // Default should be Middle C
}

// Test case: Value Construction and Getter
TEST_F(PitchTest, ValueConstruction) {
    EXPECT_EQ(middleC.getMidiNoteNumber(), 60);
    EXPECT_EQ(lowest.getMidiNoteNumber(), 0);
    EXPECT_EQ(highest.getMidiNoteNumber(), 127);
}

// Test case: Construction Validation (Out of Range)
TEST_F(PitchTest, ConstructionValidation) {
    // Test lower bound
    EXPECT_THROW(Pitch(-1), std::out_of_range);
    // Test upper bound
    EXPECT_THROW(Pitch(128), std::out_of_range);
    // Test valid bounds
    EXPECT_NO_THROW(Pitch(0));
    EXPECT_NO_THROW(Pitch(127));
}

// Test case: Comparison Operators
TEST_F(PitchTest, ComparisonOperators) {
    Pitch middleC_copy(60);

    // Equality
    EXPECT_TRUE(middleC == middleC_copy);
    EXPECT_FALSE(middleC == cSharp4);

    // Inequality
    EXPECT_FALSE(middleC != middleC_copy);
    EXPECT_TRUE(middleC != cSharp4);

    // Less than
    EXPECT_FALSE(middleC < middleC_copy);
    EXPECT_TRUE(bFlat3 < middleC);
    EXPECT_FALSE(middleC < bFlat3);

    // Greater than
    EXPECT_FALSE(middleC > middleC_copy);
    EXPECT_TRUE(cSharp4 > middleC);
    EXPECT_FALSE(middleC > cSharp4);

    // Less than or equal to
    EXPECT_TRUE(middleC <= middleC_copy);
    EXPECT_TRUE(bFlat3 <= middleC);
    EXPECT_FALSE(cSharp4 <= middleC);

    // Greater than or equal to
    EXPECT_TRUE(middleC >= middleC_copy);
    EXPECT_TRUE(cSharp4 >= middleC);
    EXPECT_FALSE(bFlat3 >= middleC);
}

// Test case: Transposition
TEST_F(PitchTest, Transposition) {
    Pitch g4 = middleC.transpose(7); // Transpose C4 up 7 semitones to G4
    EXPECT_EQ(g4.getMidiNoteNumber(), 67);

    Pitch f3 = middleC.transpose(-7); // Transpose C4 down 7 semitones to F3
    EXPECT_EQ(f3.getMidiNoteNumber(), 53);

    // Test transposition resulting in out of range
    EXPECT_THROW(highest.transpose(1), std::out_of_range);
    EXPECT_THROW(lowest.transpose(-1), std::out_of_range);

    // Test transposition to bounds
    EXPECT_NO_THROW(highest.transpose(0));
    EXPECT_EQ(highest.transpose(0).getMidiNoteNumber(), 127);
    EXPECT_NO_THROW(lowest.transpose(0));
    EXPECT_EQ(lowest.transpose(0).getMidiNoteNumber(), 0);
    EXPECT_EQ(Pitch(1).transpose(-1).getMidiNoteNumber(), 0);
    EXPECT_EQ(Pitch(126).transpose(1).getMidiNoteNumber(), 127);
}

// Test case: Static MiddleC helper
TEST_F(PitchTest, StaticMiddleC) {
    EXPECT_EQ(Pitch::MiddleC().getMidiNoteNumber(), 60);
    EXPECT_EQ(Pitch::MiddleC(), middleC);
}