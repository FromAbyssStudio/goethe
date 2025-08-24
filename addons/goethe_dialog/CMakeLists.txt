
cmake_minimum_required(VERSION 3.20)

project(goethe_dialog_extension)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find yaml-cpp (required)
find_package(yaml-cpp REQUIRED)

# Find zstd (optional)
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

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/engine
)

# Source files from the main library
set(LIBRARY_SOURCES
    ../../src/engine/core/dialog.cpp
    ../../src/engine/core/compression/backend.cpp
    ../../src/engine/core/compression/factory.cpp
    ../../src/engine/core/compression/manager.cpp
    ../../src/engine/core/compression/register_backends.cpp
    ../../src/engine/core/compression/implementations/null.cpp
    ../../src/engine/core/statistics.cpp
)

# Add zstd implementation if available
if(ZSTD_FOUND)
    list(APPEND LIBRARY_SOURCES ../../src/engine/core/compression/implementations/zstd.cpp)
endif()

# Create a static library
add_library(goethe_lib STATIC ${LIBRARY_SOURCES})

# Link libraries
target_link_libraries(goethe_lib
    PRIVATE
    yaml-cpp
)

if(ZSTD_FOUND)
    target_link_libraries(goethe_lib PRIVATE ${ZSTD_LIBRARIES})
    target_include_directories(goethe_lib PRIVATE ${ZSTD_INCLUDE_DIRS})
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_definitions(goethe_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(goethe_lib PRIVATE -fvisibility=hidden)
endif()

# Create a simple test program
add_executable(goethe_test test_main.cpp)
target_link_libraries(goethe_test goethe_lib)

message(STATUS "Building basic Goethe library and test executable")
message(STATUS "Godot extension will be added once godot-cpp is properly set up")
