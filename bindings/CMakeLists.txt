# pybind11 bindings for the Qt-free MusicTrainer core.
#
# Produces the importable Python module `musictrainer_core` which the PyQt6
# frontend (see ../python/) uses to drive the C++ domain/rules/midi/io/commands
# logic. The core library itself contains no Qt, so these bindings have no Qt
# dependency either.
#
# Configure with:
#   cmake -DMUSICTRAINER_BUILD_PYTHON_BINDINGS=ON ..
# then build the `musictrainer_core` target.

# --- pybind11 dependency (fetched, pinned for reproducible builds) ---
include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.12.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pybind11)

# --- Binding module ---
pybind11_add_module(musictrainer_core
    core_bindings.cpp
)

# Link the Qt-free core library. `rtmidi` is linked as well so that the
# transitive <RtMidi.h> include path is available to this translation unit
# (RtMidiSystem.h pulls it in).
target_link_libraries(musictrainer_core
    PRIVATE
        MusicTrainer_Core
        rtmidi
)

# Output the compiled extension next to the PyQt package for convenience during
# development. Override with -DCMAKE_LIBRARY_OUTPUT_DIRECTORY if desired.
set_target_properties(musictrainer_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/src/musictrainer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/src/musictrainer"
)

message(STATUS "Configuring pybind11 module: musictrainer_core")
