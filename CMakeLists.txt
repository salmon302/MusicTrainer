cmake_minimum_required(VERSION 3.24)
project(MusicTrainerV3)

# Set C++17 as required and ensure proper compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
endif()

# Add a global compile definition to fix RtMidi namespace conflicts
# These need to be set before any targets are defined
add_definitions(-DRTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES)

# Add Qt6 support with proper configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Explicitly set Qt version
set(QT_VERSION_MAJOR 6)

# Find Qt packages - with explicit paths if needed
# If you have installed Qt in a non-standard location, you can set Qt6_DIR here
# set(Qt6_DIR "/path/to/your/Qt/6.x.x/gcc_64/lib/cmake/Qt6")

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Charts)

# Check if Qt6 was found and configure accordingly
if(Qt${QT_VERSION_MAJOR}_FOUND)
    message(STATUS "Qt${QT_VERSION_MAJOR} found. Building GUI applications.")
else()
    message(WARNING "Qt${QT_VERSION_MAJOR} not found. GUI applications will not be built.")
endif()

# Linux-specific configuration
if(UNIX AND NOT APPLE)
	# ALSA is required for Linux
	find_package(ALSA REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	
	# JACK is optional
	find_package(PkgConfig QUIET)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(JACK jack)
	endif()
	
	# Configure RtMidi based on available backends
	set(RTMIDI_API_ALSA ON CACHE BOOL "Enable ALSA API" FORCE)
	if(JACK_FOUND)
		set(RTMIDI_API_JACK ON CACHE BOOL "Enable JACK API" FORCE)
	else()
		set(RTMIDI_API_JACK OFF CACHE BOOL "Disable JACK API" FORCE)
		add_definitions(-DRTMIDI_NO_JACK)
	endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add Google Test
include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add RtMidi with Linux-specific options
FetchContent_Declare(
	rtmidi
	URL https://github.com/thestk/rtmidi/archive/refs/tags/5.0.0.zip
)
FetchContent_MakeAvailable(rtmidi)

# Configure RtMidi to avoid deprecated function warnings being treated as errors
if(TARGET rtmidi)
    target_compile_options(rtmidi PRIVATE -Wno-deprecated-declarations -Wno-error=deprecated-declarations)
endif()

# Apply patch to RtMidi if patch file exists
#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/patches/rtmidi_fix.patch")
#    message(STATUS "Applying rt-midi fix patch...")
#    # Modify to apply patch with the correct path to RtMidi.cpp
#    execute_process(
#        COMMAND patch -p0 -i "${CMAKE_CURRENT_SOURCE_DIR}/patches/rtmidi_fix.patch"
#        WORKING_DIRECTORY ${rtmidi_SOURCE_DIR}
#        RESULT_VARIABLE patch_result
#        ERROR_VARIABLE patch_error
#        OUTPUT_VARIABLE patch_output
#    )
#    if(NOT patch_result EQUAL 0)
#        message(STATUS "Patch output: ${patch_output}")
#        message(STATUS "Patch error: ${patch_error}")
#        message(STATUS "Trying alternative patch location...")
#        # Try to find the file and patch it directly
#        file(GLOB_RECURSE rtmidi_files ${rtmidi_SOURCE_DIR}/RtMidi.cpp)
#        if(rtmidi_files)
#            list(GET rtmidi_files 0 rtmidi_file)
#            get_filename_component(rtmidi_dir ${rtmidi_file} DIRECTORY)
#            execute_process(
#                COMMAND patch -p0 -i "${CMAKE_CURRENT_SOURCE_DIR}/patches/rtmidi_fix.patch"
#                WORKING_DIRECTORY ${rtmidi_dir}
#                RESULT_VARIABLE patch_result
#            )
#            if(NOT patch_result EQUAL 0)
#                message(WARNING "Failed to apply rt-midi patch, continuing without the patch")
#            endif()
#        else()
#            message(WARNING "RtMidi.cpp not found, continuing without the patch")
#        endif()
#    endif()
#endif()

# Library sources (excluding main.cpp and test-only files)
set(LIB_SOURCES
	src/domain/monitoring/PerformanceMonitor.cpp

	src/domain/errors/DefaultErrorHandlers.cpp
	src/domain/errors/ErrorHandler.cpp
	src/domain/errors/ErrorLogger.cpp
	src/domain/errors/RecoveryStrategy.cpp

	src/domain/music/Pitch.cpp
	src/domain/music/Duration.cpp
	src/domain/music/Interval.cpp
	src/domain/music/Voice.cpp
	src/domain/music/Score.cpp
	src/domain/music/Note.cpp
	src/domain/music/MelodicTemplate.cpp
	src/domain/rules/ParallelFifthsRule.cpp
	src/domain/rules/ParallelOctavesRule.cpp
	src/domain/rules/VoiceLeadingRule.cpp
	src/domain/rules/ValidationPipeline.cpp
	src/domain/events/NoteAddedEvent.cpp
	src/domain/events/EventStore.cpp
	src/domain/events/EventBus.cpp
	src/domain/events/NoteEventHandler.cpp
	src/domain/events/Snapshot.cpp
	src/domain/exercises/Exercise.cpp
	src/adapters/InMemoryScoreRepository.cpp
	src/adapters/InMemoryExerciseRepository.cpp
	src/adapters/MockMidiAdapter.cpp
	src/adapters/TemplateBasedExerciseGenerator.cpp
	src/adapters/EventSourcedRepository.cpp
	src/adapters/CachingScoreRepository.cpp
	src/adapters/RtMidiAdapter.cpp
	src/domain/progression/ProgressionSystem.cpp
	src/domain/plugins/PluginManager.cpp
	src/domain/crossdomain/CrossDomainValidator.cpp
	src/domain/statesync/StateSynchronizationManager.cpp
	src/utils/DebugUtils.cpp)


# Header files
set(HEADERS
	include/domain/monitoring/PerformanceMonitor.h
	include/domain/errors/ErrorBase.h
	include/domain/errors/DomainErrors.h
	include/domain/errors/ErrorHandler.h
	include/domain/errors/DefaultErrorHandlers.h
	include/domain/errors/ErrorLogger.h
	include/domain/errors/RecoveryStrategy.h
	include/domain/music/Pitch.h
	include/domain/music/Duration.h
	include/domain/music/Interval.h
	include/domain/music/Voice.h
	include/domain/music/Score.h
	include/domain/music/MelodicTemplate.h
	include/domain/rules/Rule.h
	include/domain/rules/ParallelFifthsRule.h
	include/domain/rules/ParallelOctavesRule.h
	include/domain/rules/VoiceLeadingRule.h
	include/domain/rules/ValidationPipeline.h
	include/domain/events/Event.h
	include/domain/events/NoteAddedEvent.h
	include/domain/events/EventStore.h
	include/domain/events/EventBus.h
	include/domain/events/NoteEventHandler.h
	include/domain/events/Snapshot.h
	include/domain/ports/ScoreRepository.h
	include/domain/ports/MidiPort.h
	include/domain/ports/ExercisePort.h
	include/adapters/InMemoryScoreRepository.h
	include/adapters/MockMidiAdapter.h
	include/adapters/LockFreeEventQueue.h
	include/adapters/TemplateBasedExerciseGenerator.h
	include/adapters/RtMidiAdapter.h
	include/domain/plugins/PluginInterface.h
	include/domain/plugins/PluginManager.h
	include/utils/DebugUtils.h)

# GUI sources
set(GUI_SOURCES
    src/presentation/MainWindow.cpp
    src/presentation/ScoreView.cpp
    src/presentation/ViewportManager.cpp
    src/presentation/NoteGrid.cpp
    src/presentation/GridCell.cpp
    src/presentation/TransportControls.cpp
    src/presentation/ExercisePanel.cpp
    src/presentation/FeedbackArea.cpp
    src/presentation/ExerciseBrowser.cpp
    src/presentation/SettingsDialog.cpp
    src/presentation/viewmodels/ScoreViewModel.cpp
    src/domain/analysis/VoiceAnalyzer.cpp
    src/presentation/VoiceChart.cpp
)

# GUI headers
set(GUI_HEADERS
    include/presentation/MainWindow.h
    include/presentation/ScoreView.h
    include/presentation/ViewportManager.h
    include/presentation/NoteGrid.h
    include/presentation/GridCell.h
    include/presentation/TransportControls.h
    include/presentation/ExercisePanel.h
    include/presentation/FeedbackArea.h
    include/presentation/ExerciseBrowser.h
    include/presentation/SettingsDialog.h
    include/presentation/viewmodels/ScoreViewModel.h
    include/domain/analysis/VoiceAnalyzer.h
    include/presentation/VoiceChart.h
    include/domain/exercises/Exercise.h
    include/domain/exercises/ExerciseRepository.h
    include/adapters/InMemoryExerciseRepository.h
)

# Test-only sources
set(TEST_SOURCES
    tests/ValidationPipelineTests.cpp
    tests/ExerciseGenerationTests.cpp
    tests/MidiSystemTests.cpp
    tests/RepositoryTests.cpp
    tests/ProgressionSystemTests.cpp
    tests/ErrorHandlingTests.cpp
    tests/PerformanceMonitoringTests.cpp
    tests/integration/SystemIntegrationTests.cpp
    tests/plugins/PluginSystemTests.cpp
    tests/plugins/TestPlugin.cpp
    tests/crossdomain/CrossDomainValidatorTests.cpp
    tests/statesync/StateSynchronizationManagerTests.cpp
)

set(MUSICTRAINERGUI_SOURCES
    src/presentation/MainWindow.cpp
    src/presentation/ScoreView.cpp
    src/presentation/ViewportManager.cpp
    src/presentation/NoteGrid.cpp
    src/presentation/GridCell.cpp
    src/presentation/TransportControls.cpp
    src/presentation/ExercisePanel.cpp
    src/presentation/FeedbackArea.cpp
    src/presentation/ExerciseBrowser.cpp
    src/presentation/SettingsDialog.cpp
    src/presentation/viewmodels/ScoreViewModel.cpp
    src/domain/analysis/VoiceAnalyzer.cpp
    src/presentation/VoiceChart.cpp
)

set(MUSICTRAINERGUI_HEADERS
    include/presentation/MainWindow.h
    include/presentation/ScoreView.h
    include/presentation/ViewportManager.h
    include/presentation/NoteGrid.h
    include/presentation/GridCell.h
    include/presentation/TransportControls.h
    include/presentation/ExercisePanel.h
    include/presentation/FeedbackArea.h
    include/presentation/ExerciseBrowser.h
    include/presentation/SettingsDialog.h
    include/presentation/viewmodels/ScoreViewModel.h
    include/presentation/VoiceChart.h
    include/domain/analysis/VoiceAnalyzer.h
    include/domain/exercises/Exercise.h
    include/domain/exercises/ExerciseRepository.h
    include/adapters/InMemoryExerciseRepository.h
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main library target (not executable)
add_library(MusicTrainerLib STATIC ${LIB_SOURCES} ${HEADERS})
target_link_libraries(MusicTrainerLib PRIVATE rtmidi ${ALSA_LIBRARIES})

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE MusicTrainerLib)

# Check if Qt is available and create GUI applications
if(Qt${QT_VERSION_MAJOR}_FOUND)
    # Add the full GUI application
    add_executable(MusicTrainerGUI src/presentation/main_gui.cpp ${MUSICTRAINERGUI_SOURCES} ${MUSICTRAINERGUI_HEADERS})
    target_link_libraries(MusicTrainerGUI PRIVATE 
        MusicTrainerLib 
        Qt${QT_VERSION_MAJOR}::Core 
        Qt${QT_VERSION_MAJOR}::Widgets 
        Qt${QT_VERSION_MAJOR}::Charts 
        rtmidi 
        ${ALSA_LIBRARIES}
    )
    target_include_directories(MusicTrainerGUI PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )
    set_target_properties(MusicTrainerGUI PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
    )
    
    # Add the test GUI application - use the same headers as MusicTrainerGUI
    add_executable(TestGUI src/presentation/test_gui.cpp ${MUSICTRAINERGUI_SOURCES} ${MUSICTRAINERGUI_HEADERS})
    target_link_libraries(TestGUI PRIVATE 
        MusicTrainerLib 
        Qt${QT_VERSION_MAJOR}::Core 
        Qt${QT_VERSION_MAJOR}::Widgets 
        Qt${QT_VERSION_MAJOR}::Charts 
        rtmidi 
        ${ALSA_LIBRARIES}
    )
    target_include_directories(TestGUI PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )
    set_target_properties(TestGUI PROPERTIES
        WIN32_EXECUTABLE FALSE
        MACOSX_BUNDLE FALSE
    )
endif()

# Add main test suite
add_executable(TestSuite
	tests/main.cpp
	tests/TestSuite.cpp
	${TEST_SOURCES})
target_include_directories(TestSuite PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TestSuite PRIVATE gtest gtest_main MusicTrainerLib rtmidi ${ALSA_LIBRARIES})
target_compile_definitions(TestSuite PRIVATE TESTING)

# Define test targets
set(TEST_TARGETS
	ValidationPipelineTests
	ExerciseGenerationTests
	MidiSystemTests
	RepositoryTests
	ProgressionSystemTests
	SystemIntegrationTests
	PluginSystemTests
	ErrorHandlingTests
	PerformanceMonitoringTests
	CrossDomainValidatorTests
	StateSynchronizationManagerTests)


# Configure test executables
foreach(TEST_TARGET ${TEST_TARGETS})
	if(${TEST_TARGET} STREQUAL "SystemIntegrationTests")
		add_executable(${TEST_TARGET} 
			tests/main.cpp
			tests/integration/${TEST_TARGET}.cpp)
	elseif(${TEST_TARGET} STREQUAL "PluginSystemTests")
		add_executable(${TEST_TARGET} 
			tests/main.cpp
			tests/plugins/${TEST_TARGET}.cpp 
			tests/plugins/TestPlugin.cpp)
	elseif(${TEST_TARGET} STREQUAL "CrossDomainValidatorTests")
		add_executable(${TEST_TARGET} 
			tests/main.cpp
			tests/crossdomain/${TEST_TARGET}.cpp)
	elseif(${TEST_TARGET} STREQUAL "StateSynchronizationManagerTests")
		add_executable(${TEST_TARGET} 
			tests/main.cpp
			tests/statesync/${TEST_TARGET}.cpp)
	else()
		add_executable(${TEST_TARGET} 
			tests/main.cpp
			tests/${TEST_TARGET}.cpp)
	endif()
	
	target_include_directories(${TEST_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/include)
	target_compile_definitions(${TEST_TARGET} PRIVATE TESTING)
	target_link_libraries(${TEST_TARGET} PRIVATE gtest gtest_main MusicTrainerLib rtmidi ${ALSA_LIBRARIES})
	add_test(NAME ${TEST_TARGET}
			 COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET}
			 WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach()

if(CLEAR_CACHE)
	 add_definitions(-DCLEAR_CACHE)
endif()

# Enable testing
enable_testing()

# Set test properties for timeout and environment
set_tests_properties(${TEST_TARGETS} PROPERTIES
	TIMEOUT 30
	ENVIRONMENT "GTEST_FILTER=*")





