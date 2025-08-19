cmake_minimum_required(VERSION 3.16)

# Use FetchContent to automatically download and build GoogleTest
include(FetchContent)

# Download and configure GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0  # Use the latest stable version
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Prevent GoogleTest from being installed with our package
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# Suppress warnings from GoogleTest external library (we cannot modify their code)
if(NOT MSVC)
    target_compile_options(gtest PRIVATE -Wno-float-equal)
    target_compile_options(gtest_main PRIVATE -Wno-float-equal)
    target_compile_options(gmock PRIVATE -Wno-float-equal)
    target_compile_options(gmock_main PRIVATE -Wno-float-equal)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Enable GoogleTest discovery for better IDE integration
include(GoogleTest)
include(CTest)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})  # For mocks

# Function to add platform-specific sources
function(add_platform_sources target_name)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(${target_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_platform_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_system_monitor.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_power_manager.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_signal_handler.cpp
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_sources(${target_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_platform_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_system_monitor.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_power_manager.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_signal_handler.cpp
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_sources(${target_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_platform_utils.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_system_monitor.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_power_manager.cpp
            ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_signal_handler.cpp
        )
    endif()
endfunction()

# Function to configure test executable
function(configure_test_executable target_name)
    target_link_libraries(${target_name}
        gtest
        gtest_main
        gmock
        gmock_main
        Threads::Threads
    )
    
    # Platform-specific libraries (same as main executable)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Link Windows-specific libraries for Performance Counters
        target_link_libraries(${target_name} pdh)
    endif()
    
    target_compile_definitions(${target_name} PRIVATE DDOGREEN_VERSION="test")
    
    # Apply coverage flags if coverage is enabled
    if(BUILD_WITH_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE --coverage -g -O0)
        target_link_options(${target_name} PRIVATE --coverage)
    endif()
    
    gtest_discover_tests(${target_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        TIMEOUT 30
    )
endfunction()

# Config tests
add_executable(test_config
    test_config.cpp
    test_config_platform.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/security_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/rate_limiter.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
)
add_platform_sources(test_config)
configure_test_executable(test_config)

# Activity Monitor tests
add_executable(test_activity_monitor
    test_activity_monitor.cpp
    ${CMAKE_SOURCE_DIR}/src/activity_monitor.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/security_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/rate_limiter.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
)
add_platform_sources(test_activity_monitor)
configure_test_executable(test_activity_monitor)

# Logger tests
add_executable(test_logger
    test_logger.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
)
configure_test_executable(test_logger)

# Platform Factory tests
add_executable(test_platform_factory
    test_platform_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/security_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/rate_limiter.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
)
add_platform_sources(test_platform_factory)
configure_test_executable(test_platform_factory)

# Integration tests
add_executable(test_integration
    test_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/activity_monitor.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/security_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/rate_limiter.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
)
add_platform_sources(test_integration)
configure_test_executable(test_integration)

# Security tests
add_executable(test_security
    test_security.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/security_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/rate_limiter.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
)
add_platform_sources(test_security)
configure_test_executable(test_security)
