cmake_minimum_required(VERSION 3.10)
project(MCP VERSION 2024.11.05 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "build shared libraries" ${BUILD_SHARED_LIBS_DEFAULT})

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

option(MCP_SSL "Enable SSL support" OFF)

if(MCP_SSL)
    find_package(OpenSSL 3.0.0 COMPONENTS Crypto SSL REQUIRED)
    add_compile_definitions(MCP_SSL CPPHTTPLIB_OPENSSL_SUPPORT)
endif()

include(FetchContent)
FetchContent_Declare(
        cpphttplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
        GIT_TAG v0.30.2
        FIND_PACKAGE_ARGS NAMES httplib
)
FetchContent_MakeAvailable(cpphttplib)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.12.0
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_Declare(
        base64
        GIT_REPOSITORY https://github.com/Bobini1/base64
        GIT_TAG master
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(base64)
find_package(Threads REQUIRED)

add_library(mcp STATIC
        src/mcp_message.cpp
        src/mcp_resource.cpp
        src/mcp_server.cpp
        src/mcp_tool.cpp
        src/mcp_stdio_client.cpp
        src/mcp_sse_client.cpp
)

target_link_libraries(mcp PUBLIC Threads::Threads httplib::httplib nlohmann_json::nlohmann_json base64)

target_include_directories(mcp PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include/cpp-mcp>"
)
target_include_directories(mcp PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/common"
)

# If OpenSSL is found, link the OpenSSL libraries
if(OPENSSL_FOUND)
    target_link_libraries(mcp PUBLIC OpenSSL::Crypto OpenSSL::SSL)
endif()

if (MSVC)
    target_compile_options(mcp PUBLIC "$<$<COMPILE_LANGUAGE:C,CXX>:/utf-8>")
    target_compile_options(mcp PUBLIC "$<$<COMPILE_LANGUAGE:C,CXX>:/bigobj>")
endif()

target_compile_features(mcp PUBLIC cxx_std_17)

# Add examples
add_subdirectory(examples)

# Add test directory
option(MCP_BUILD_TESTS "Build the tests" OFF)
if(MCP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
