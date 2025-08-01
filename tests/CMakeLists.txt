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

# Find required packages
find_package(Threads REQUIRED)

# Enable GoogleTest discovery for better IDE integration
include(GoogleTest)
include(CTest)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create test executable
add_executable(test_config
    test_config.cpp
    test_config_platform.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/idaemon.cpp
)

# Add platform-specific source files based on the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(test_config PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_platform_utils.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_system_monitor.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_power_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/linux/linux_daemon.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(test_config PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_platform_utils.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_system_monitor.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_power_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/windows/windows_daemon.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(test_config PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_platform_utils.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_system_monitor.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_power_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/macos/macos_daemon.cpp
    )
endif()

# Link libraries
target_link_libraries(test_config
    gtest
    gtest_main
    Threads::Threads
)

# Add compiler definitions
target_compile_definitions(test_config PRIVATE DDOGREEN_VERSION="test")

# Discover individual GoogleTest cases for automatic IDE integration
gtest_discover_tests(test_config
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 30
)
