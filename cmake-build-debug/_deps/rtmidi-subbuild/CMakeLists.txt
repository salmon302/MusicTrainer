# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.24.0-rc1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(rtmidi-populate NONE)



include(ExternalProject)
ExternalProject_Add(rtmidi-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://github.com/thestk/rtmidi/archive/refs/tags/5.0.0.zip" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE"
                    SOURCE_DIR          "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-src"
                    BINARY_DIR          "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


