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
CMAKE_BINARY_DIR = /home/seth-n/Documents/GitHub/MusicTrainer

# Utility rule file for MidiSystemTests_autogen_timestamp_deps.

# Include any custom commands dependencies for this target.
include CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/progress.make

CMakeFiles/MidiSystemTests_autogen_timestamp_deps: /usr/lib/qt6/libexec/uic
CMakeFiles/MidiSystemTests_autogen_timestamp_deps: /usr/lib/qt6/libexec/moc
CMakeFiles/MidiSystemTests_autogen_timestamp_deps: lib/libgtest.so.1.13.0
CMakeFiles/MidiSystemTests_autogen_timestamp_deps: lib/libgtest_main.so.1.13.0
CMakeFiles/MidiSystemTests_autogen_timestamp_deps: _deps/rtmidi-build/librtmidi.so.6.0.0
CMakeFiles/MidiSystemTests_autogen_timestamp_deps: libMusicTrainerLib.a

MidiSystemTests_autogen_timestamp_deps: CMakeFiles/MidiSystemTests_autogen_timestamp_deps
MidiSystemTests_autogen_timestamp_deps: CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/build.make
.PHONY : MidiSystemTests_autogen_timestamp_deps

# Rule to build all files generated by this target.
CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/build: MidiSystemTests_autogen_timestamp_deps
.PHONY : CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/build

CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/clean

CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/depend:
	cd /home/seth-n/Documents/GitHub/MusicTrainer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/MidiSystemTests_autogen_timestamp_deps.dir/depend

