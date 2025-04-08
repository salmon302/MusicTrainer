# Define the test executable

# Google Test integration (assuming FetchContent in root CMakeLists.txt)
# Enable CTest integration provided by Google Test module
include(GoogleTest)

# List test source files
set(TEST_SOURCES
    domain/Position_test.cpp
    domain/Pitch_test.cpp
    domain/Note_test.cpp
    domain/TimeSignature_test.cpp
    domain/KeySignature_test.cpp
    domain/Score_test.cpp

    # Rules tests
    rules/ParallelFifthsRule_test.cpp
    rules/ParallelOctavesRule_test.cpp # Added new rule test
    # Add other rule test files here
)

# Add the test executable
add_executable(MusicTrainer_Tests ${TEST_SOURCES})

# Link the test executable against the core library and Google Test
target_link_libraries(MusicTrainer_Tests
    PRIVATE
        MusicTrainer_Core # Link against our core library (domain + rules)
        GTest::gtest      # Link against Google Test library
        GTest::gtest_main # Link against Google Test main (provides main() and CTest integration)
)

# Discover tests from the executable and add them to CTest
gtest_discover_tests(MusicTrainer_Tests)

message(STATUS "Configuring MusicTrainer_Tests executable with Google Test")