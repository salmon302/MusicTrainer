# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-src"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-build"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix/tmp"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src"
  "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/salmo/CLionProjects/MusicTrainerV3/cmake-build-debug/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp/${subDir}")
endforeach()
