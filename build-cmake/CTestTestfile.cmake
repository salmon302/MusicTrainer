# CMake generated Testfile for 
# Source directory: /home/seth-n/Documents/GitHub/MusicTrainer
# Build directory: /home/seth-n/Documents/GitHub/MusicTrainer/build-cmake
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[ValidationPipelineTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/ValidationPipelineTests")
set_tests_properties([=[ValidationPipelineTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[ExerciseGenerationTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/ExerciseGenerationTests")
set_tests_properties([=[ExerciseGenerationTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[MidiSystemTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/MidiSystemTests")
set_tests_properties([=[MidiSystemTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[RepositoryTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/RepositoryTests")
set_tests_properties([=[RepositoryTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[ProgressionSystemTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/ProgressionSystemTests")
set_tests_properties([=[ProgressionSystemTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[SystemIntegrationTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/SystemIntegrationTests")
set_tests_properties([=[SystemIntegrationTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[PluginSystemTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/PluginSystemTests")
set_tests_properties([=[PluginSystemTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[ErrorHandlingTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/ErrorHandlingTests")
set_tests_properties([=[ErrorHandlingTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[PerformanceMonitoringTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/PerformanceMonitoringTests")
set_tests_properties([=[PerformanceMonitoringTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[CrossDomainValidatorTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/CrossDomainValidatorTests")
set_tests_properties([=[CrossDomainValidatorTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
add_test([=[StateSynchronizationManagerTests]=] "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin/StateSynchronizationManagerTests")
set_tests_properties([=[StateSynchronizationManagerTests]=] PROPERTIES  ENVIRONMENT "GTEST_FILTER=*" TIMEOUT "30" WORKING_DIRECTORY "/home/seth-n/Documents/GitHub/MusicTrainer/build-cmake/bin" _BACKTRACE_TRIPLES "/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;387;add_test;/home/seth-n/Documents/GitHub/MusicTrainer/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
subdirs("_deps/rtmidi-build")
