cmake_minimum_required(VERSION 3.10)
project(MCP VERSION 2024.11.05 LANGUAGES CXX)

if (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

option(MCP_SSL "Enable SSL support" OFF)
if (MCP_SSL)
    find_package(OpenSSL 3.0.0 COMPONENTS Crypto SSL REQUIRED)
    add_compile_definitions(MCP_SSL CPPHTTPLIB_OPENSSL_SUPPORT)
endif ()

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(base64 REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

add_library(mcp STATIC
        src/mcp_message.cpp
        src/mcp_resource.cpp
        src/mcp_server.cpp
        src/mcp_tool.cpp
        src/mcp_stdio_client.cpp
        src/mcp_sse_client.cpp
)

target_link_libraries(mcp PUBLIC Threads::Threads httplib::httplib nlohmann_json::nlohmann_json base64::base64 spdlog::spdlog)

target_include_directories(mcp PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include/cpp-mcp>"
)

# If OpenSSL is found, link the OpenSSL libraries
if (OPENSSL_FOUND)
    target_link_libraries(mcp PUBLIC OpenSSL::Crypto OpenSSL::SSL)
endif ()

if (MSVC)
    target_compile_options(mcp PUBLIC "$<$<COMPILE_LANGUAGE:C,CXX>:/utf-8>")
    target_compile_options(mcp PUBLIC "$<$<COMPILE_LANGUAGE:C,CXX>:/bigobj>")
endif ()

target_compile_features(mcp PUBLIC cxx_std_17)

option(MCP_BUILD_EXAMPLES "Build the examples" OFF)
if (MCP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

# Add test directory
option(MCP_BUILD_TESTS "Build the tests" OFF)
if (MCP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS mcp
        EXPORT mcpTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpp-mcp)
install(EXPORT mcpTargets
        FILE mcpTargets.cmake
        NAMESPACE mcp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcp
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mcpConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/mcpConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcp
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/mcpConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mcpConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mcpConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mcp
)
