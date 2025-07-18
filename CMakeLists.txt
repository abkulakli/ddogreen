cmake_minimum_required(VERSION 3.16)
project(ddogreen VERSION 1.0.0
        DESCRIPTION "Intelligent Green Power Management for Sustainable Computing"
        HOMEPAGE_URL "https://www.ddosoft.com")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific flags
if(MSVC)
    # Microsoft Visual C++ flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
    # Disable specific warnings that are too verbose
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")  # Disable deprecation warnings
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/activity_monitor.cpp
    src/daemon.cpp
    src/logger.cpp
    src/platform/platform_factory.cpp
)

# Platform-specific source files
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        src/platform/linux/linux_power_manager.cpp
        src/platform/linux/linux_service_manager.cpp
        src/platform/linux/linux_system_monitor.cpp
        src/platform/linux/linux_platform_utils.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_service_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
        src/platform/windows/windows_platform_utils.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        src/platform/macos/macos_power_manager.cpp
        src/platform/macos/macos_service_manager.cpp
        src/platform/macos/macos_system_monitor.cpp
        src/platform/macos/macos_platform_utils.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Include directories
include_directories(include)

# Create executable
add_executable(ddogreen ${SOURCES})

# Platform-specific libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Link Windows-specific libraries for Performance Counters
    target_link_libraries(ddogreen pdh)
endif()

# Installation
install(TARGETS ddogreen DESTINATION /usr/local/bin)
