#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Set test filter to run tests sequentially
    testing::FLAGS_gtest_filter = "*";
    testing::FLAGS_gtest_break_on_failure = true;
    
    return RUN_ALL_TESTS();
}