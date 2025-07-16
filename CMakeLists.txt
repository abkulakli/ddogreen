cmake_minimum_required(VERSION 3.16)
project(ddops VERSION 1.0.0
        DESCRIPTION "Dynamic Device Optimization Power Switcher"
        HOMEPAGE_URL "https://www.ddosoft.com")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Source files
set(SOURCES
    src/main.cpp
    src/activity_monitor.cpp
    src/tlp_manager.cpp
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
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_service_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        src/platform/macos/macos_power_manager.cpp
        src/platform/macos/macos_service_manager.cpp
        src/platform/macos/macos_system_monitor.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Include directories
include_directories(include)

# Create executable
add_executable(ddops ${SOURCES})

# Installation
install(TARGETS ddops DESTINATION /usr/local/bin)
install(FILES systemd/ddops.service DESTINATION /etc/systemd/system)
