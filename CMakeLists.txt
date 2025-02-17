cmake_minimum_required(VERSION 3.24)
project(MusicTrainerV3)

set(CMAKE_CXX_STANDARD 17)

if(MINGW)
	# Memory optimization during compilation
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-inline -fno-elide-constructors")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fno-inline -fno-elide-constructors")
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

# Add RtMidi
FetchContent_Declare(
    rtmidi
    URL https://github.com/thestk/rtmidi/archive/refs/tags/5.0.0.zip
)
FetchContent_MakeAvailable(rtmidi)

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
	src/adapters/InMemoryScoreRepository.cpp
	src/adapters/MockMidiAdapter.cpp
	src/adapters/TemplateBasedExerciseGenerator.cpp
	src/adapters/EventSourcedRepository.cpp
	src/adapters/CachingScoreRepository.cpp
	src/adapters/RtMidiAdapter.cpp
	src/domain/progression/ProgressionSystem.cpp
	src/domain/plugins/PluginManager.cpp
	src/domain/crossdomain/CrossDomainValidator.cpp
	src/domain/statesync/StateSynchronizationManager.cpp
	src/utils/DebugUtils.cpp
	src/utils/LockTracker.cpp)

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
	include/utils/DebugUtils.h
	include/utils/LockTracker.h)

# Test-only sources
set(TEST_SOURCES
)


# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp ${LIB_SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE rtmidi)

# Add main test suite
add_executable(TestSuite 
	tests/TestSuite.cpp
	${TEST_SOURCES}
	${LIB_SOURCES})
target_include_directories(TestSuite PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TestSuite PRIVATE gtest rtmidi)

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
	StateSynchronizationManagerTests
	LockingTests)

# Configure test executables
foreach(TEST_TARGET ${TEST_TARGETS})
	if(${TEST_TARGET} STREQUAL "SystemIntegrationTests")
		add_executable(${TEST_TARGET} 
			tests/integration/${TEST_TARGET}.cpp 
			${TEST_SOURCES}
			${LIB_SOURCES})
	elseif(${TEST_TARGET} STREQUAL "PluginSystemTests")
		add_executable(${TEST_TARGET} 
			tests/plugins/${TEST_TARGET}.cpp 
			tests/plugins/TestPlugin.cpp
			${TEST_SOURCES}
			${LIB_SOURCES})
	elseif(${TEST_TARGET} STREQUAL "CrossDomainValidatorTests")
		add_executable(${TEST_TARGET} 
			tests/crossdomain/${TEST_TARGET}.cpp 
			${TEST_SOURCES}
			${LIB_SOURCES})
	elseif(${TEST_TARGET} STREQUAL "StateSynchronizationManagerTests")
		add_executable(${TEST_TARGET} 
			tests/statesync/${TEST_TARGET}.cpp 
			${TEST_SOURCES}
			${LIB_SOURCES})
	else()
		add_executable(${TEST_TARGET} 
			tests/${TEST_TARGET}.cpp 
			${TEST_SOURCES}
			${LIB_SOURCES})
	endif()
	
	target_include_directories(${TEST_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/include)
	target_compile_definitions(${TEST_TARGET} PRIVATE TESTING)
	target_link_libraries(${TEST_TARGET} PRIVATE gtest gtest_main rtmidi)
	add_test(NAME ${TEST_TARGET} 
			 COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET}
			 WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach()

# Enable testing
enable_testing()

# Set test properties for timeout and environment
set_tests_properties(${TEST_TARGETS} PROPERTIES
	TIMEOUT 30
	ENVIRONMENT "GTEST_BREAK_ON_FAILURE=1;GTEST_FILTER=*")





