# Define the test executable

# Google Test integration (assuming FetchContent in root CMakeLists.txt)
# Enable CTest integration provided by Google Test module
include(GoogleTest)

# List test source files
set(TEST_SOURCES
    # Domain tests
    domain/Position_test.cpp
    domain/Pitch_test.cpp
    domain/Note_test.cpp
    domain/TimeSignature_test.cpp
    domain/KeySignature_test.cpp
    domain/Score_test.cpp
    domain/ScoreJson_test.cpp # JSON (de)serialization round-trip

    # Rules tests
    rules/ParallelFifthsRule_test.cpp
    rules/ParallelOctavesRule_test.cpp

    # Command / undo-redo tests
    commands/AddNoteCommand_test.cpp
    commands/DeleteNoteCommand_test.cpp
    commands/ResizeNoteCommand_test.cpp
    commands/MoveNotesCommand_test.cpp
    commands/CommandHistory_test.cpp
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