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

# Include any dependencies generated for this target.
include CMakeFiles/MusicTrainerGUI.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/MusicTrainerGUI.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MusicTrainerGUI.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MusicTrainerGUI.dir/flags.make

CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o: MusicTrainerGUI_autogen/mocs_compilation.cpp
CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/MusicTrainerGUI_autogen/mocs_compilation.cpp

CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/MusicTrainerGUI_autogen/mocs_compilation.cpp > CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.i

CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/MusicTrainerGUI_autogen/mocs_compilation.cpp -o CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o: src/presentation/main_gui.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/main_gui.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/main_gui.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/main_gui.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o: src/presentation/MainWindow.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/MainWindow.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/MainWindow.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/MainWindow.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o: src/presentation/ScoreView.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ScoreView.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ScoreView.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ScoreView.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o: src/presentation/ViewportManager.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ViewportManager.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ViewportManager.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ViewportManager.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o: src/presentation/NoteGrid.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/NoteGrid.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/NoteGrid.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/NoteGrid.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o: src/presentation/GridCell.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/GridCell.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/GridCell.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/GridCell.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o: src/presentation/TransportControls.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/TransportControls.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/TransportControls.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/TransportControls.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o: src/presentation/ExercisePanel.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ExercisePanel.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ExercisePanel.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/ExercisePanel.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o: src/presentation/FeedbackArea.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/FeedbackArea.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/FeedbackArea.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/FeedbackArea.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o: src/presentation/viewmodels/ScoreViewModel.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/viewmodels/ScoreViewModel.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/viewmodels/ScoreViewModel.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/viewmodels/ScoreViewModel.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o: src/domain/analysis/VoiceAnalyzer.cpp
CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/domain/analysis/VoiceAnalyzer.cpp

CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/domain/analysis/VoiceAnalyzer.cpp > CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/domain/analysis/VoiceAnalyzer.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.s

CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o: CMakeFiles/MusicTrainerGUI.dir/flags.make
CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o: src/presentation/VoiceChart.cpp
CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o: CMakeFiles/MusicTrainerGUI.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o -MF CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o.d -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o -c /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/VoiceChart.cpp

CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/VoiceChart.cpp > CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.i

CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/seth-n/Documents/GitHub/MusicTrainer/src/presentation/VoiceChart.cpp -o CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.s

# Object files for target MusicTrainerGUI
MusicTrainerGUI_OBJECTS = \
"CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o" \
"CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o"

# External object files for target MusicTrainerGUI
MusicTrainerGUI_EXTERNAL_OBJECTS =

bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/MusicTrainerGUI_autogen/mocs_compilation.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/main_gui.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/MainWindow.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/ScoreView.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/ViewportManager.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/NoteGrid.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/GridCell.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/TransportControls.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/ExercisePanel.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/FeedbackArea.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/viewmodels/ScoreViewModel.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/domain/analysis/VoiceAnalyzer.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/src/presentation/VoiceChart.cpp.o
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/build.make
bin/MusicTrainerGUI: libMusicTrainerLib.a
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6Charts.so.6.4.2
bin/MusicTrainerGUI: _deps/rtmidi-build/librtmidi.so.6.0.0
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libasound.so
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6OpenGLWidgets.so.6.4.2
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6Widgets.so.6.4.2
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6OpenGL.so.6.4.2
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6Gui.so.6.4.2
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libQt6Core.so.6.4.2
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libGLX.so
bin/MusicTrainerGUI: /usr/lib/x86_64-linux-gnu/libOpenGL.so
bin/MusicTrainerGUI: CMakeFiles/MusicTrainerGUI.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Linking CXX executable bin/MusicTrainerGUI"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MusicTrainerGUI.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MusicTrainerGUI.dir/build: bin/MusicTrainerGUI
.PHONY : CMakeFiles/MusicTrainerGUI.dir/build

CMakeFiles/MusicTrainerGUI.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MusicTrainerGUI.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MusicTrainerGUI.dir/clean

CMakeFiles/MusicTrainerGUI.dir/depend:
	cd /home/seth-n/Documents/GitHub/MusicTrainer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer /home/seth-n/Documents/GitHub/MusicTrainer/CMakeFiles/MusicTrainerGUI.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/MusicTrainerGUI.dir/depend

