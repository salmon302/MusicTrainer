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

# Include any dependencies generated for this target.
include _deps/rtmidi-build/CMakeFiles/qmidiin.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include _deps/rtmidi-build/CMakeFiles/qmidiin.dir/compiler_depend.make

# Include the progress variables for this target.
include _deps/rtmidi-build/CMakeFiles/qmidiin.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/rtmidi-build/CMakeFiles/qmidiin.dir/flags.make

_deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o: _deps/rtmidi-build/CMakeFiles/qmidiin.dir/flags.make
_deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o: _deps/rtmidi-src/tests/qmidiin.cpp
_deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o: _deps/rtmidi-build/CMakeFiles/qmidiin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object _deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o"
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT _deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o -MF CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o.d -o CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src/tests/qmidiin.cpp

_deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.i"
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src/tests/qmidiin.cpp > CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.i

_deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.s"
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src/tests/qmidiin.cpp -o CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.s

# Object files for target qmidiin
qmidiin_OBJECTS = \
"CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o"

# External object files for target qmidiin
qmidiin_EXTERNAL_OBJECTS =

_deps/rtmidi-build/tests/qmidiin: _deps/rtmidi-build/CMakeFiles/qmidiin.dir/tests/qmidiin.cpp.o
_deps/rtmidi-build/tests/qmidiin: _deps/rtmidi-build/CMakeFiles/qmidiin.dir/build.make
_deps/rtmidi-build/tests/qmidiin: _deps/rtmidi-build/librtmidi.so.6.0.0
_deps/rtmidi-build/tests/qmidiin: _deps/rtmidi-build/CMakeFiles/qmidiin.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tests/qmidiin"
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/qmidiin.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/rtmidi-build/CMakeFiles/qmidiin.dir/build: _deps/rtmidi-build/tests/qmidiin
.PHONY : _deps/rtmidi-build/CMakeFiles/qmidiin.dir/build

_deps/rtmidi-build/CMakeFiles/qmidiin.dir/clean:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build && $(CMAKE_COMMAND) -P CMakeFiles/qmidiin.dir/cmake_clean.cmake
.PHONY : _deps/rtmidi-build/CMakeFiles/qmidiin.dir/clean

_deps/rtmidi-build/CMakeFiles/qmidiin.dir/depend:
	cd /home/seth-n/Documents/GitHub/MusicTrainer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src /home/seth-n/Documents/GitHub/MusicTrainer/build /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build /home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build/CMakeFiles/qmidiin.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : _deps/rtmidi-build/CMakeFiles/qmidiin.dir/depend

