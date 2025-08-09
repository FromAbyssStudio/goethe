cmake_minimum_required(VERSION 3.20)

# Tests for Goethe

add_executable(goethe_tests
  test_engine_basic.cpp
)

target_include_directories(goethe_tests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/sdk
)

target_link_libraries(goethe_tests
  PRIVATE
    goethe
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(goethe_tests)


