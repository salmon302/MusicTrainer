# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-src"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-build"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/tmp"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src"
  "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build/_deps/rtmidi-subbuild/rtmidi-populate-prefix/src/rtmidi-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
