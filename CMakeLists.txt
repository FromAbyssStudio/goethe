cmake_minimum_required(VERSION 3.20)

# Try to use Clang as the compiler, fall back to GCC if not available
find_program(CLANG_CXX clang++)
find_program(CLANG_C clang)

if(CLANG_CXX AND CLANG_C)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    message(STATUS "Using Clang compiler")
else()
    message(STATUS "Clang not found, using default compiler (GCC)")
endif()

project(GoetheDialog
  VERSION 0.1.0
  DESCRIPTION "Goethe Dialog System — Shared library for visual novel dialog management"
  LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)

# Visibility defaults hidden; only GOETHE_API is exported
if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-fvisibility=hidden)
endif()

# Find yaml-cpp (required for dialog module)
find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
  message(STATUS "yaml-cpp not found via find_package, trying pkg-config")
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(YAML_CPP QUIET yaml-cpp)
    if(YAML_CPP_FOUND)
      message(STATUS "Found yaml-cpp via pkg-config: ${YAML_CPP_VERSION}")
      set(yaml-cpp_FOUND TRUE)
    endif()
  endif()
endif()

if(NOT yaml-cpp_FOUND)
  message(FATAL_ERROR "yaml-cpp is required but not found. Please install yaml-cpp.")
endif()

# Find zstd (optional for compression)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(ZSTD QUIET libzstd)
endif()

if(ZSTD_FOUND)
  message(STATUS "Found zstd: ${ZSTD_VERSION}")
  add_compile_definitions(GOETHE_ZSTD_AVAILABLE)
else()
  message(STATUS "zstd not found - compression will use null backend only")
endif()

# Find OpenSSL (required for package encryption and signing)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(OPENSSL QUIET openssl)
endif()

if(OPENSSL_FOUND)
  message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
  add_compile_definitions(GOETHE_OPENSSL_AVAILABLE)
else()
  message(STATUS "OpenSSL not found - package encryption and signing will be disabled")
endif()

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
  message(STATUS "Google Test not found, attempting to find gtest/gmock")
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(GTEST QUIET gtest gmock)
    if(GTEST_FOUND)
      message(STATUS "Found Google Test via pkg-config: ${GTEST_VERSION}")
      set(GTest_FOUND TRUE)
    endif()
  endif()
endif()

if(GTest_FOUND)
  message(STATUS "Google Test found - tests will be built")
  add_compile_definitions(GOETHE_GTEST_AVAILABLE)
else()
  message(STATUS "Google Test not found - tests will be disabled")
  message(STATUS "Install with: sudo pacman -S gtest (Arch Linux) or equivalent")
endif()

# Dialog library sources
set(GOETHE_DIALOG_SOURCES
  src/engine/core/dialog.cpp
  src/engine/core/compression/backend.cpp
  src/engine/core/compression/factory.cpp
  src/engine/core/compression/manager.cpp
  src/engine/core/compression/register_backends.cpp
  src/engine/core/compression/implementations/null.cpp
  src/engine/core/compression/implementations/zstd.cpp
  src/engine/core/statistics.cpp
)

# Dialog library headers
set(GOETHE_DIALOG_HEADERS
  include/goethe/dialog.hpp
  include/goethe/backend.hpp
  include/goethe/factory.hpp
  include/goethe/manager.hpp
  include/goethe/register_backends.hpp
  include/goethe/null.hpp
  include/goethe/zstd.hpp
  include/goethe/statistics.hpp
  include/goethe/goethe_dialog.h
)

# Create the shared library
add_library(goethe_dialog SHARED ${GOETHE_DIALOG_SOURCES} ${GOETHE_DIALOG_HEADERS})

# Set include directories
target_include_directories(goethe_dialog
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link yaml-cpp
if(yaml-cpp_FOUND)
  if(YAML_CPP_FOUND)
    # Found via pkg-config
    target_link_libraries(goethe_dialog PUBLIC ${YAML_CPP_LIBRARIES})
    target_include_directories(goethe_dialog PRIVATE ${YAML_CPP_INCLUDE_DIRS})
    target_compile_options(goethe_dialog PRIVATE ${YAML_CPP_CFLAGS_OTHER})
  else()
    # Found via find_package
    target_link_libraries(goethe_dialog PUBLIC yaml-cpp)
  endif()
endif()

# Link zstd if available
if(ZSTD_FOUND)
  target_link_libraries(goethe_dialog PRIVATE ${ZSTD_LIBRARIES})
  target_include_directories(goethe_dialog PRIVATE ${ZSTD_INCLUDE_DIRS})
  target_compile_options(goethe_dialog PRIVATE ${ZSTD_CFLAGS_OTHER})
endif()

# Link OpenSSL if available
if(OPENSSL_FOUND)
  target_link_libraries(goethe_dialog PRIVATE ${OPENSSL_LIBRARIES})
  target_include_directories(goethe_dialog PRIVATE ${OPENSSL_INCLUDE_DIRS})
  target_compile_options(goethe_dialog PRIVATE ${OPENSSL_CFLAGS_OTHER})
endif()

# Set library properties
set_target_properties(goethe_dialog PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  OUTPUT_NAME "goethe"
)

# Compiler options
if(MSVC)
  target_compile_options(goethe_dialog PRIVATE /EHsc- /GR-)
  target_compile_definitions(goethe_dialog PRIVATE GOETHE_EXPORTS)
else()
  target_compile_options(goethe_dialog PRIVATE -fexceptions)
  target_compile_definitions(goethe_dialog PRIVATE GOETHE_EXPORTS)
endif()

# Test executables
if(GTest_FOUND)
  # Google Test based tests
  add_executable(test_basic ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_basic.cpp)
  target_link_libraries(test_basic PRIVATE GTest::gtest GTest::gmock)
  
  add_executable(test_dialog ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_dialog.cpp)
  target_link_libraries(test_dialog PRIVATE goethe_dialog GTest::gtest GTest::gmock)
  
  add_executable(test_compression ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_compression.cpp)
  target_link_libraries(test_compression PRIVATE goethe_dialog GTest::gtest GTest::gmock)
  
  add_executable(minimal_compression_test ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/minimal_compression_test.cpp)
  target_link_libraries(minimal_compression_test PRIVATE GTest::gtest GTest::gmock)
  
  # Add tests to CTest
  add_test(NAME BasicTests COMMAND test_basic)
  add_test(NAME DialogTests COMMAND test_dialog)
  add_test(NAME CompressionTests COMMAND test_compression)
  add_test(NAME MinimalCompressionTests COMMAND minimal_compression_test)
  
  # Set test properties
  set_tests_properties(BasicTests PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "GTEST_COLOR=1"
  )
  set_tests_properties(DialogTests PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "GTEST_COLOR=1"
  )
  set_tests_properties(CompressionTests PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "GTEST_COLOR=1"
  )
  set_tests_properties(MinimalCompressionTests PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "GTEST_COLOR=1"
  )
else()
  # Fallback simple test (without gtest)
  add_executable(simple_test ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/simple_test.cpp)
  target_link_libraries(simple_test PRIVATE goethe_dialog)
  
  # Statistics test
  add_executable(statistics_test ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/statistics_test.cpp)
  target_link_libraries(statistics_test PRIVATE goethe_dialog)
  
  # Simple statistics test
  add_executable(simple_statistics_test ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/simple_statistics_test.cpp)
  target_link_libraries(simple_statistics_test PRIVATE goethe_dialog)
  
  # Minimal statistics test
  add_executable(minimal_statistics_test ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/minimal_statistics_test.cpp)
  target_link_libraries(minimal_statistics_test PRIVATE goethe_dialog)
endif()

# Package tool executable (commented out until package.hpp is implemented)
# add_executable(test_package src/tests/test_package.cpp)
# target_link_libraries(test_package PRIVATE goethe_dialog)

# Package tool executable (commented out until package.hpp is implemented)
# add_executable(gdkg_tool src/tools/gdkg_tool.cpp)
# target_link_libraries(gdkg_tool PRIVATE goethe_dialog)

# Statistics tool executable
add_executable(statistics_tool ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/statistics_tool.cpp)
target_link_libraries(statistics_tool PRIVATE goethe_dialog)

# Install rules for goethe_dialog
install(TARGETS goethe_dialog
  EXPORT GoetheDialogTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(FILES ${GOETHE_DIALOG_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/goethe
)

# Export targets
install(EXPORT GoetheDialogTargets
  FILE GoetheDialogTargets.cmake
  NAMESPACE Goethe::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GoetheDialog
)

# Create config file
include(CMakePackageConfigHelpers)

# Simple config file content
set(GOETHE_DIALOG_CONFIG_CONTENT
"@PACKAGE_INIT@

include(\"\${CMAKE_CURRENT_LIST_DIR}/GoetheDialogTargets.cmake\")

check_required_components(GoetheDialog)
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GoetheDialogConfig.cmake "${GOETHE_DIALOG_CONFIG_CONTENT}")

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheDialogConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheDialogConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheDialogConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GoetheDialog
)

# Install schemas
install(DIRECTORY schemas/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/goethe/schemas
  FILES_MATCHING PATTERN "*.yaml" PATTERN "*.yml")

# Install scripts
install(DIRECTORY scripts/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/goethe/scripts
  FILES_MATCHING PATTERN "*.sh")

# Convenience clean targets
add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E echo "Removing build directory: ${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}" ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}"
  COMMENT "Distclean: remove entire build directory (safe chdir)"
)


# --- Tests (GoogleTest + CTest) ---
if(GOETHE_BUILD_TESTS)
  include(CTest)
  enable_testing()

  include(FetchContent)
  # Use a stable, modern release of GoogleTest
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  # Do not install gtest when installing this project
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_subdirectory(tests)
endif()


