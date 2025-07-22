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
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
)

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
