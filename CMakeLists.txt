# Minimum CMake version required
cmake_minimum_required(VERSION 3.24)

# Project definition
project(MusicTrainer VERSION 4.0 LANGUAGES CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer standard C++ over compiler extensions

# --- Qt6 Setup (optional) ---
# Qt is only required for the legacy C++/Qt6 GUI application. The core library
# and the test suite are fully Qt-free and build without it (e.g. on CI runners).
# AUTOMOC/AUTORCC/AUTOUIC are intentionally NOT set globally: they are enabled
# only on the MusicTrainerApp target below so non-Qt targets are unaffected.
option(MUSICTRAINER_BUILD_QT_APP
    "Build the legacy C++/Qt6 GUI application (MusicTrainerApp)" ON)
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia)

# --- Python bindings (optional) ---
# When enabled, a pybind11 module exposing the Qt-free core to Python is built.
# This is the bridge consumed by the PyQt6 frontend (see python/ and
# docs/current/PyQtMigration.md).
option(MUSICTRAINER_BUILD_PYTHON_BINDINGS
    "Build Python (pybind11) bindings for the core library (musictrainer_core)" OFF)

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
  GIT_TAG 6.0.0 # Pin to a released tag for reproducible builds
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

# --- Python bindings (optional) ---
# Builds the `musictrainer_core` pybind11 module consumed by the PyQt6 frontend.
if(MUSICTRAINER_BUILD_PYTHON_BINDINGS)
    add_subdirectory(bindings)
endif()

# --- Application Executable (legacy C++/Qt GUI; optional) ---
# Qt is ONLY used here. Everything else (core library, tests) is Qt-free.
if(MUSICTRAINER_BUILD_QT_APP AND Qt6_FOUND)
    add_executable(MusicTrainerApp
        src/main.cpp
        src/ui/MainWindow.h # Headers with Q_OBJECT need to be listed for AUTOMOC
        src/ui/MainWindow.cpp
        src/ui/ScoreViewportWidget.h # Headers with Q_OBJECT need to be listed for AUTOMOC
        src/ui/ScoreViewportWidget.cpp
        # Add other application-specific source files here
    )

    # Enable Qt's code generators for this target only.
    set_target_properties(MusicTrainerApp PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    # Link the application against the core library and Qt modules
    target_link_libraries(MusicTrainerApp
        PRIVATE
            MusicTrainer_Core # Link against our core library
            Qt::Core
            Qt::Gui
            Qt::Widgets
            Qt::Multimedia # Needed for multimedia (audio) support
    )
elseif(MUSICTRAINER_BUILD_QT_APP)
    message(WARNING "Qt6 not found - the legacy GUI application (MusicTrainerApp) will not be built. The test suite can still be configured and built.")
endif()

# --- Optional: Global Settings ---
# Example: Set output directories
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Message for configuration status
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}") # Often set via -D CMAKE_BUILD_TYPE=Release/Debug
message(STATUS "Build legacy C++/Qt GUI: ${MUSICTRAINER_BUILD_QT_APP}")
message(STATUS "Build Python (pybind11) bindings: ${MUSICTRAINER_BUILD_PYTHON_BINDINGS}")