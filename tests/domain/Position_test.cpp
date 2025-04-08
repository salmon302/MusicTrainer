#include <gtest/gtest.h> // Google Test header
#include "../../src/domain/Position.h" // Header for the class under test

// Use the namespace for convenience
using namespace MusicTrainer::Domain;

// Test fixture for Position class (optional, but good practice)
class PositionTest : public ::testing::Test {
protected:
    // Setup code here (if needed)
    void SetUp() override {
        // Example: Initialize common Position objects
        p0 = Position(0.0);
        p1 = Position(1.0);
        p1_5 = Position(1.5);
    }

    // Teardown code here (if needed)
    // void TearDown() override {}

    // Member variables accessible in tests
    Position p0, p1, p1_5;
    const double tolerance = 1e-9; // Tolerance for float comparisons
};

// Test case: Default Construction
TEST_F(PositionTest, DefaultConstruction) {
    Position p_default;
    EXPECT_NEAR(p_default.beats, 0.0, tolerance); // Default should be 0.0
}

// Test case: Value Construction
TEST_F(PositionTest, ValueConstruction) {
    Position p_val(4.75);
    EXPECT_NEAR(p_val.beats, 4.75, tolerance);
}

// Test case: Comparison Operators
TEST_F(PositionTest, ComparisonOperators) {
    Position p1_copy(1.0);
    Position p_slightly_less(1.0 - tolerance * 0.5);
    Position p_slightly_more(1.0 + tolerance * 0.5);
    Position p_different(2.0);

    // Equality
    EXPECT_TRUE(p1 == p1_copy);
    EXPECT_TRUE(p1 == p_slightly_less); // Should be equal within tolerance
    EXPECT_TRUE(p1 == p_slightly_more); // Should be equal within tolerance
    EXPECT_FALSE(p1 == p_different);

    // Inequality
    EXPECT_FALSE(p1 != p1_copy);
    EXPECT_FALSE(p1 != p_slightly_less);
    EXPECT_FALSE(p1 != p_slightly_more);
    EXPECT_TRUE(p1 != p_different);

    // Less than
    EXPECT_FALSE(p1 < p1_copy);
    EXPECT_FALSE(p1 < p_slightly_less); // Not strictly less
    EXPECT_TRUE(p0 < p1);
    EXPECT_FALSE(p1 < p0);

    // Greater than
    EXPECT_FALSE(p1 > p1_copy);
    EXPECT_FALSE(p1 > p_slightly_more); // Not strictly greater
    EXPECT_TRUE(p1 > p0);
    EXPECT_FALSE(p0 > p1);

    // Less than or equal to
    EXPECT_TRUE(p1 <= p1_copy);
    EXPECT_TRUE(p1 <= p_slightly_more);
    EXPECT_TRUE(p0 <= p1);
    EXPECT_FALSE(p1 <= p0);

    // Greater than or equal to
    EXPECT_TRUE(p1 >= p1_copy);
    EXPECT_TRUE(p1 >= p_slightly_less);
    EXPECT_TRUE(p1 >= p0);
    EXPECT_FALSE(p0 >= p1);
}

// Test case: Arithmetic Operators
TEST_F(PositionTest, ArithmeticOperators) {
    Position p_start(2.0);

    // Addition
    Position p_plus = p_start + 1.5;
    EXPECT_NEAR(p_plus.beats, 3.5, tolerance);

    // Subtraction (offset)
    Position p_minus = p_start - 0.5;
    EXPECT_NEAR(p_minus.beats, 1.5, tolerance);

    // Subtraction (difference)
    double diff = p_plus - p_minus;
    EXPECT_NEAR(diff, 2.0, tolerance);

    // Add assign
    p_start += 0.25;
    EXPECT_NEAR(p_start.beats, 2.25, tolerance);

    // Subtract assign
    p_start -= 1.25;
    EXPECT_NEAR(p_start.beats, 1.0, tolerance);
}

// Main function to run the tests (usually in a separate file, but okay here for simplicity)
// int main(int argc, char **argv) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }