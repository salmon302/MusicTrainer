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

# Utility rule file for NightlyTest.

# Include any custom commands dependencies for this target.
include _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/compiler_depend.make

# Include the progress variables for this target.
include _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/progress.make

_deps/rtmidi-build/CMakeFiles/NightlyTest:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && /usr/bin/ctest -D NightlyTest

NightlyTest: _deps/rtmidi-build/CMakeFiles/NightlyTest
NightlyTest: _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/build.make
.PHONY : NightlyTest

# Rule to build all files generated by this target.
_deps/rtmidi-build/CMakeFiles/NightlyTest.dir/build: NightlyTest
.PHONY : _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/build

_deps/rtmidi-build/CMakeFiles/NightlyTest.dir/clean:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && $(CMAKE_COMMAND) -P CMakeFiles/NightlyTest.dir/cmake_clean.cmake
.PHONY : _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/clean

_deps/rtmidi-build/CMakeFiles/NightlyTest.dir/depend:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src /home/seth-n/Documents/GitHub/MusicTrainer/build /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build/CMakeFiles/NightlyTest.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : _deps/rtmidi-build/CMakeFiles/NightlyTest.dir/depend

