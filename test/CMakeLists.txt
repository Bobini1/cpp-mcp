cmake_minimum_required(VERSION 3.10)

project(mcp_tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Use local Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    FIND_PACKAGE_ARGS NAMES GTest
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test source files
set(TEST_SOURCES
    mcp_test.cpp
)

# Create test executable
add_executable(mcp_tests mcp_test.cpp)

# Link Google Test and MCP library
target_link_libraries(mcp_tests PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    mcp
)

include(GoogleTest)
gtest_discover_tests(mcp_tests)