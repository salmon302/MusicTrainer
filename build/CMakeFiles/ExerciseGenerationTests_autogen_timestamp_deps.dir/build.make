# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/seth-n/Documents/GitHub/MusicTrainer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/seth-n/Documents/GitHub/MusicTrainer/build

# Utility rule file for ExerciseGenerationTests_autogen_timestamp_deps.

# Include any custom commands dependencies for this target.
include CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/progress.make

CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: _deps/rtmidi-build/librtmidi.so.6.0.0
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: lib/libgtest.a
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: lib/libgtest_main.a
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: /usr/lib/qt6/libexec/moc
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: /usr/lib/qt6/libexec/uic
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps: libMusicTrainerLib.a

ExerciseGenerationTests_autogen_timestamp_deps: CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps
ExerciseGenerationTests_autogen_timestamp_deps: CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/build.make
.PHONY : ExerciseGenerationTests_autogen_timestamp_deps

# Rule to build all files generated by this target.
CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/build: ExerciseGenerationTests_autogen_timestamp_deps
.PHONY : CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/build

CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/clean

CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/depend:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer/build /home/seth-n/Documents/GitHub/MusicTrainer/build /home/seth-n/Documents/GitHub/MusicTrainer/build/CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/ExerciseGenerationTests_autogen_timestamp_deps.dir/depend

