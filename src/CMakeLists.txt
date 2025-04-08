# Define the library for the core domain model and rules
# Changed to STATIC since we now have .cpp files
add_library(MusicTrainer_Core STATIC) # Renamed for broader scope

# Specify include directories needed by consumers of this library
# Consumers need access to headers in src/domain and src/rules
target_include_directories(MusicTrainer_Core
    PUBLIC # PUBLIC makes includes available to consumers linking to this lib
        ${CMAKE_CURRENT_SOURCE_DIR} # Makes src/ accessible (for domain/, rules/)
)

# Add source files
target_sources(MusicTrainer_Core
    PRIVATE
        # Domain source files (if any added later)
        # domain/SomeClass.cpp

        # Rules source files
        rules/ParallelFifthsRule.cpp
        rules/ParallelOctavesRule.cpp # Added new rule
        rules/ConcreteRuleEngine.cpp
        # Add other rule .cpp files here

        # MIDI source files
        midi/RtMidiSystem.cpp
        # Add other midi .cpp files here

        # IO source files
        io/PresetLoader.cpp
        io/ScoreLoader.cpp # Added score loader
        # Add other io .cpp files here

        # Command source files
        commands/AddNoteCommand.cpp
        commands/DeleteNoteCommand.cpp
        commands/MoveNotesCommand.cpp
        commands/ResizeNoteCommand.cpp # Added resize command
        commands/CommandHistory.cpp
        # Add other command .cpp files here
)

# Link dependencies (if any needed internally)
target_link_libraries(MusicTrainer_Core
    PRIVATE
        # Link against RtMidi fetched in the parent scope
        rtmidi
        # Qt::Core is linked by the final application executable, not the core library
)

message(STATUS "Configuring MusicTrainer_Core library (STATIC)")