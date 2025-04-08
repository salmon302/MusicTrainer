#include <gtest/gtest.h>
#include "../../src/domain/TimeSignature.h"
#include <stdexcept> // For testing exceptions

using namespace MusicTrainer::Domain;

// Test fixture for TimeSignature class
class TimeSignatureTest : public ::testing::Test {
protected:
    TimeSignature ts4_4; // Common Time
    TimeSignature ts3_4; // Waltz Time
    TimeSignature ts6_8; // Compound Meter
    TimeSignature ts2_2; // Cut Time

    void SetUp() override {
        ts4_4 = TimeSignature(4, 4);
        ts3_4 = TimeSignature(3, 4);
        ts6_8 = TimeSignature(6, 8);
        ts2_2 = TimeSignature(2, 2);
    }
     const double tolerance = 1e-9; // Tolerance for float comparisons
};

// Test case: Default Construction
TEST_F(TimeSignatureTest, DefaultConstruction) {
    TimeSignature ts_default;
    EXPECT_EQ(ts_default.getNumerator(), 4);
    EXPECT_EQ(ts_default.getDenominator(), 4);
    EXPECT_TRUE(ts_default == TimeSignature::CommonTime());
}

// Test case: Value Construction and Getters
TEST_F(TimeSignatureTest, ValueConstruction) {
    EXPECT_EQ(ts4_4.getNumerator(), 4);
    EXPECT_EQ(ts4_4.getDenominator(), 4);

    EXPECT_EQ(ts3_4.getNumerator(), 3);
    EXPECT_EQ(ts3_4.getDenominator(), 4);

    EXPECT_EQ(ts6_8.getNumerator(), 6);
    EXPECT_EQ(ts6_8.getDenominator(), 8);

    EXPECT_EQ(ts2_2.getNumerator(), 2);
    EXPECT_EQ(ts2_2.getDenominator(), 2);
}

// Test case: Construction Validation
TEST_F(TimeSignatureTest, ConstructionValidation) {
    // Invalid Numerator (zero)
    EXPECT_THROW(TimeSignature(0, 4), std::invalid_argument);

    // Invalid Denominator (zero)
    EXPECT_THROW(TimeSignature(4, 0), std::invalid_argument);
    // Invalid Denominator (not power of 2)
    EXPECT_THROW(TimeSignature(4, 3), std::invalid_argument);
    EXPECT_THROW(TimeSignature(4, 6), std::invalid_argument);
    EXPECT_THROW(TimeSignature(4, 7), std::invalid_argument);

    // Valid construction should not throw
    EXPECT_NO_THROW(TimeSignature(1, 1)); // 1/1 (Valid power of 2)
    EXPECT_NO_THROW(TimeSignature(4, 2)); // 4/2
    EXPECT_NO_THROW(TimeSignature(5, 4)); // 5/4
    EXPECT_NO_THROW(TimeSignature(7, 8)); // 7/8
    EXPECT_NO_THROW(TimeSignature(12, 16)); // 12/16
}

// Test case: Comparison Operators
TEST_F(TimeSignatureTest, ComparisonOperators) {
    TimeSignature ts4_4_copy(4, 4);

    // Equality
    EXPECT_TRUE(ts4_4 == ts4_4_copy);
    EXPECT_FALSE(ts4_4 == ts3_4);
    EXPECT_FALSE(ts4_4 == ts6_8);
    EXPECT_FALSE(ts4_4 == ts2_2);

    // Inequality
    EXPECT_FALSE(ts4_4 != ts4_4_copy);
    EXPECT_TRUE(ts4_4 != ts3_4);
    EXPECT_TRUE(ts4_4 != ts6_8);
    EXPECT_TRUE(ts4_4 != ts2_2);
}

// Test case: Measure Duration Calculation
TEST_F(TimeSignatureTest, MeasureDuration) {
    // Duration in terms of quarter notes
    EXPECT_NEAR(ts4_4.getMeasureDurationInBeats(), 4.0, tolerance); // 4 * (4/4) = 4
    EXPECT_NEAR(ts3_4.getMeasureDurationInBeats(), 3.0, tolerance); // 3 * (4/4) = 3
    EXPECT_NEAR(ts6_8.getMeasureDurationInBeats(), 3.0, tolerance); // 6 * (4/8) = 3
    EXPECT_NEAR(ts2_2.getMeasureDurationInBeats(), 4.0, tolerance); // 2 * (4/2) = 4
    EXPECT_NEAR(TimeSignature(5, 4).getMeasureDurationInBeats(), 5.0, tolerance); // 5 * (4/4) = 5
    EXPECT_NEAR(TimeSignature(7, 8).getMeasureDurationInBeats(), 3.5, tolerance); // 7 * (4/8) = 3.5
}

// Test case: Static Helpers
TEST_F(TimeSignatureTest, StaticHelpers) {
    EXPECT_EQ(TimeSignature::CommonTime(), ts4_4);
    EXPECT_EQ(TimeSignature::CutTime(), ts2_2);
    EXPECT_EQ(TimeSignature::WaltzTime(), ts3_4);
}