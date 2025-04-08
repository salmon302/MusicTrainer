#include <gtest/gtest.h>
#include "../../src/domain/KeySignature.h"
#include "../../src/domain/Pitch.h"
#include <stdexcept> // For testing exceptions
#include <vector>
#include <algorithm> // For std::sort

using namespace MusicTrainer::Domain;

// Test fixture for KeySignature class
class KeySignatureTest : public ::testing::Test {
protected:
    KeySignature ksCMaj; // C Major (0 sharps/flats)
    KeySignature ksAMin; // A Minor (0 sharps/flats)
    KeySignature ksGMaj; // G Major (1 sharp - F#)
    KeySignature ksEMin; // E Minor (1 sharp - F#)
    // Add more key signatures as needed for testing accidentals logic

    void SetUp() override {
        ksCMaj = KeySignature(0, Mode::Major); // Tonic C = 0
        ksAMin = KeySignature(9, Mode::Minor); // Tonic A = 9
        ksGMaj = KeySignature(7, Mode::Major); // Tonic G = 7
        ksEMin = KeySignature(4, Mode::Minor); // Tonic E = 4
    }
};

// Test case: Default Construction
TEST_F(KeySignatureTest, DefaultConstruction) {
    KeySignature ks_default; // Should default to C Major
    EXPECT_EQ(ks_default.getTonic().getMidiNoteNumber() % 12, 0); // Tonic C
    EXPECT_EQ(ks_default.getMode(), Mode::Major);
    EXPECT_TRUE(ks_default == ksCMaj);
}

// Test case: Value Construction (Pitch Class) and Getters
TEST_F(KeySignatureTest, ValueConstructionPitchClass) {
    EXPECT_EQ(ksCMaj.getTonic().getMidiNoteNumber() % 12, 0);
    EXPECT_EQ(ksCMaj.getMode(), Mode::Major);

    EXPECT_EQ(ksAMin.getTonic().getMidiNoteNumber() % 12, 9);
    EXPECT_EQ(ksAMin.getMode(), Mode::Minor);

    EXPECT_EQ(ksGMaj.getTonic().getMidiNoteNumber() % 12, 7);
    EXPECT_EQ(ksGMaj.getMode(), Mode::Major);

    EXPECT_EQ(ksEMin.getTonic().getMidiNoteNumber() % 12, 4);
    EXPECT_EQ(ksEMin.getMode(), Mode::Minor);
}

// Test case: Value Construction (Pitch Object)
TEST_F(KeySignatureTest, ValueConstructionPitchObject) {
    Pitch tonicG4(67); // G4
    KeySignature ksG4Maj(tonicG4, Mode::Major);
    EXPECT_EQ(ksG4Maj.getTonic(), tonicG4); // Checks exact pitch object
    EXPECT_EQ(ksG4Maj.getTonic().getMidiNoteNumber() % 12, 7); // Checks pitch class
    EXPECT_EQ(ksG4Maj.getMode(), Mode::Major);
    EXPECT_TRUE(ksG4Maj == ksGMaj); // Should be equal based on pitch class and mode
}


// Test case: Construction Validation
TEST_F(KeySignatureTest, ConstructionValidation) {
    // Invalid Tonic Pitch Class
    EXPECT_THROW(KeySignature(-1, Mode::Major), std::invalid_argument);
    EXPECT_THROW(KeySignature(12, Mode::Minor), std::invalid_argument);

    // Valid construction should not throw
    EXPECT_NO_THROW(KeySignature(0, Mode::Major));  // C Major
    EXPECT_NO_THROW(KeySignature(11, Mode::Minor)); // B Minor
}

// Test case: Comparison Operators
TEST_F(KeySignatureTest, ComparisonOperators) {
    KeySignature ksCMaj_copy(0, Mode::Major);
    KeySignature ksGMaj_copy(Pitch(67), Mode::Major); // G Major using Pitch object

    // Equality (based on tonic pitch class and mode)
    EXPECT_TRUE(ksCMaj == ksCMaj_copy);
    EXPECT_TRUE(ksGMaj == ksGMaj_copy);
    EXPECT_FALSE(ksCMaj == ksAMin); // Same accidentals, different mode/tonic class
    EXPECT_FALSE(ksCMaj == ksGMaj);

    // Inequality
    EXPECT_FALSE(ksCMaj != ksCMaj_copy);
    EXPECT_FALSE(ksGMaj != ksGMaj_copy);
    EXPECT_TRUE(ksCMaj != ksAMin);
    EXPECT_TRUE(ksCMaj != ksGMaj);
}

// Test case: Accidentals Calculation (Placeholder Logic)
TEST_F(KeySignatureTest, AccidentalsCalculation) {
    // Using the placeholder logic currently in KeySignature.h
    // C Major / A Minor -> 0 sharps, 0 flats
    EXPECT_TRUE(ksCMaj.getSharpPitchClasses().empty());
    EXPECT_TRUE(ksCMaj.getFlatPitchClasses().empty());
    EXPECT_TRUE(ksAMin.getSharpPitchClasses().empty());
    EXPECT_TRUE(ksAMin.getFlatPitchClasses().empty());

    // G Major / E Minor -> 1 sharp (F# = pitch class 6)
    std::vector<int> gMajSharps = ksGMaj.getSharpPitchClasses();
    std::vector<int> eMinSharps = ksEMin.getSharpPitchClasses();
    std::vector<int> expectedSharp = {6}; // F#
    std::sort(gMajSharps.begin(), gMajSharps.end()); // Sort for consistent comparison
    std::sort(eMinSharps.begin(), eMinSharps.end());
    EXPECT_EQ(gMajSharps, expectedSharp);
    EXPECT_EQ(eMinSharps, expectedSharp);
    EXPECT_TRUE(ksGMaj.getFlatPitchClasses().empty());
    EXPECT_TRUE(ksEMin.getFlatPitchClasses().empty());

    // TODO: Add more tests here once the calculateAccidentals logic is fully implemented
    // e.g., F Major (Bb), D Major (F#, C#), etc.
}

// Test case: Static Helpers
TEST_F(KeySignatureTest, StaticHelpers) {
    EXPECT_EQ(KeySignature::CMajor(), ksCMaj);
    EXPECT_EQ(KeySignature::AMinor(), ksAMin);
}