# Minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Project definition
project(MusicTrainer VERSION 4.0 LANGUAGES CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer standard C++ over compiler extensions

# --- Qt6 Setup ---
# Find the Qt6 package and specify required components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)
# Add other components like Concurrent, Network, Test if needed later

# Enable automatic handling of Qt's meta-object compiler, resource compiler, and UI compiler
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Other Dependencies ---
# Include FetchContent module to handle dependencies
include(FetchContent)

# Declare Google Test dependency
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip # Specify a stable version
  # Or use GIT_REPOSITORY and GIT_TAG
  # GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_TAG        release-1.14.0
)

# Make Google Test available (downloads and prepares build)
# Set QUIET to avoid excessive output during configuration
# Set SYSTEM to treat GTest headers as system headers (suppress warnings)
# Set EXCLUDE_FROM_ALL to not build gtest targets by default unless needed by tests
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Needed on Windows sometimes
FetchContent_MakeAvailable(googletest)

# Declare nlohmann/json dependency (header-only)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3 # Specify a stable version
  GIT_SHALLOW TRUE
)
# Make nlohmann_json available (just sets variables for include path)
FetchContent_MakeAvailable(nlohmann_json)


# Declare RtMidi dependency
FetchContent_Declare(
  rtmidi
  GIT_REPOSITORY https://github.com/thestk/rtmidi.git
  GIT_TAG master # Or specify a release tag like 6.0.0
  GIT_SHALLOW TRUE # Fetch only the specified tag/branch history
)
# Make RtMidi available (downloads and prepares build)
# You might need to set RTMIDI_API_XXX options here if needed, e.g.,
# set(RTMIDI_API_ALSA ON CACHE BOOL "" FORCE) # Example for Linux ALSA
FetchContent_MakeAvailable(rtmidi)


# --- Subdirectories ---
# Add the source code directory
add_subdirectory(src)

# Add the tests directory (optionally enable testing)
enable_testing() # Basic CTest enablement
add_subdirectory(tests)

# --- Application Executable ---
add_executable(MusicTrainerApp
    src/main.cpp
    src/ui/MainWindow.h # Headers with Q_OBJECT need to be listed for AUTOMOC
    src/ui/MainWindow.cpp
    src/ui/ScoreViewportWidget.h # Headers with Q_OBJECT need to be listed for AUTOMOC
    src/ui/ScoreViewportWidget.cpp
    # Add other application-specific source files here
)

# Link the application against the core library and Qt modules
target_link_libraries(MusicTrainerApp
    PRIVATE
        MusicTrainer_Core # Link against our core library
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Multimedia # Needed for MIDI support in Qt potentially
)

# --- Optional: Global Settings ---
# Example: Set output directories
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Message for configuration status
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}") # Often set via -D CMAKE_BUILD_TYPE=Release/Debug