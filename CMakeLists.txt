cmake_minimum_required(VERSION 3.16)
project(ddotlp VERSION 1.0.0
        DESCRIPTION "Dynamic TLP Power Management Daemon"
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
)

# Include directories
include_directories(include)

# Create executable
add_executable(ddotlp ${SOURCES})

# Installation
install(TARGETS ddotlp DESTINATION /usr/local/bin)
install(FILES systemd/ddotlp.service DESTINATION /etc/systemd/system)
