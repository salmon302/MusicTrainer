#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "GTest::gtest" for configuration ""
set_property(TARGET GTest::gtest APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(GTest::gtest PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libgtest.so.1.13.0"
  IMPORTED_SONAME_NOCONFIG "libgtest.so.1.13.0"
  )

list(APPEND _cmake_import_check_targets GTest::gtest )
list(APPEND _cmake_import_check_files_for_GTest::gtest "${_IMPORT_PREFIX}/lib/libgtest.so.1.13.0" )

# Import target "GTest::gtest_main" for configuration ""
set_property(TARGET GTest::gtest_main APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(GTest::gtest_main PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libgtest_main.so.1.13.0"
  IMPORTED_SONAME_NOCONFIG "libgtest_main.so.1.13.0"
  )

list(APPEND _cmake_import_check_targets GTest::gtest_main )
list(APPEND _cmake_import_check_files_for_GTest::gtest_main "${_IMPORT_PREFIX}/lib/libgtest_main.so.1.13.0" )

# Import target "GTest::gmock" for configuration ""
set_property(TARGET GTest::gmock APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(GTest::gmock PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libgmock.so.1.13.0"
  IMPORTED_SONAME_NOCONFIG "libgmock.so.1.13.0"
  )

list(APPEND _cmake_import_check_targets GTest::gmock )
list(APPEND _cmake_import_check_files_for_GTest::gmock "${_IMPORT_PREFIX}/lib/libgmock.so.1.13.0" )

# Import target "GTest::gmock_main" for configuration ""
set_property(TARGET GTest::gmock_main APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(GTest::gmock_main PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libgmock_main.so.1.13.0"
  IMPORTED_SONAME_NOCONFIG "libgmock_main.so.1.13.0"
  )

list(APPEND _cmake_import_check_targets GTest::gmock_main )
list(APPEND _cmake_import_check_files_for_GTest::gmock_main "${_IMPORT_PREFIX}/lib/libgmock_main.so.1.13.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
