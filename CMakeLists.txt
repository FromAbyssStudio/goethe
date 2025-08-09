cmake_minimum_required(VERSION 3.20)

project(Goethe
  VERSION 0.1.0
  DESCRIPTION "Goethe Engine — 2D narrative/visual-novel runtime"
  LANGUAGES C CXX)

# Options per design (default to minimal so this builds everywhere)
option(GOETHE_BUILD_SHARED "Build shared library for engine" ON)
option(GOETHE_BUILD_TOOLS  "Build CLI tools (goethec)" OFF)
option(GOETHE_BUILD_TESTS  "Build tests" OFF)

option(GOETHE_BACKEND_SDL3 "Enable SDL3 rendering backend" OFF)
option(GOETHE_BACKEND_CPU  "Enable CPU raster backend" OFF)
option(GOETHE_WITH_LUA      "Embed Lua 5.4" OFF)
option(GOETHE_WITH_HARFBUZZ "Enable HarfBuzz shaping" OFF)
option(GOETHE_WITH_HRTF     "Enable HRTF DSP path" OFF)

# SDL3 integration and install
option(GOETHE_VENDOR_SDL3   "Fetch/build SDL3 with the project (vendored)" OFF)
option(GOETHE_INSTALL_SDL3  "Ensure SDL3 is included in install (via vendoring)" OFF)

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

# Engine library
set(GOETHE_ENGINE_NAME goethe)

set(GOETHE_ENGINE_SOURCES
  engine/core/api.cpp
  engine/core/engine.cpp
)

set(GOETHE_ENGINE_HEADERS
  sdk/goethe.h
  engine/core/engine.hpp
)

if(GOETHE_BUILD_SHARED)
  add_library(${GOETHE_ENGINE_NAME} SHARED ${GOETHE_ENGINE_SOURCES} ${GOETHE_ENGINE_HEADERS})
  target_compile_definitions(${GOETHE_ENGINE_NAME} PRIVATE GOETHE_BUILD_SHARED)
else()
  add_library(${GOETHE_ENGINE_NAME} STATIC ${GOETHE_ENGINE_SOURCES} ${GOETHE_ENGINE_HEADERS})
endif()

target_include_directories(${GOETHE_ENGINE_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sdk>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  target_compile_options(${GOETHE_ENGINE_NAME} PRIVATE /EHsc- /GR-)
else()
  target_compile_options(${GOETHE_ENGINE_NAME} PRIVATE -fno-exceptions -fno-rtti)
endif()

# --- SDL3 wiring (find or vendor) ---
set(_need_sdl3 FALSE)
if(GOETHE_BACKEND_SDL3)
  set(_need_sdl3 TRUE)
endif()
if(GOETHE_INSTALL_SDL3)
  set(_need_sdl3 TRUE)
  set(GOETHE_VENDOR_SDL3 ON CACHE BOOL "" FORCE)
endif()

if(_need_sdl3)
  if(GOETHE_VENDOR_SDL3)
    include(FetchContent)
    # Pin to a known good ref if desired. Using main by default.
    FetchContent_Declare(SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG main)
    # Speed up / avoid tests and examples inside SDL
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    # Ensure SDL exposes install() rules when vendored so `cmake --install` installs SDL too
    set(SDL_INSTALL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
  else()
    find_package(SDL3 REQUIRED CONFIG)
  endif()
  if(GOETHE_BACKEND_SDL3)
    target_link_libraries(${GOETHE_ENGINE_NAME} PRIVATE SDL3::SDL3)
    target_compile_definitions(${GOETHE_ENGINE_NAME} PRIVATE GOETHE_BACKEND_SDL3=1)
  endif()
endif()

# Install rules
install(TARGETS ${GOETHE_ENGINE_NAME}
  EXPORT GoetheTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES sdk/goethe.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT GoetheTargets
  FILE GoetheTargets.cmake
  NAMESPACE Goethe::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Goethe)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoetheConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Goethe
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/GoetheConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Goethe)

# Samples (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples/hello_vn/CMakeLists.txt)
  add_subdirectory(samples/hello_vn)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples/visual_vn/CMakeLists.txt)
  add_subdirectory(samples/visual_vn)
endif()

# Convenience clean targets
# `cmake --build build --target clean` is provided by generators, but we
# also add a `distclean` to wipe the entire build tree (when out-of-source).
add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E echo "Removing build directory: ${CMAKE_BINARY_DIR}"
  # Change out of the build directory before removing it to avoid shell getcwd errors
  COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}" ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}"
  COMMENT "Distclean: remove entire build directory (safe chdir)"
)


