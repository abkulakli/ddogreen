cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(GTest REQUIRED)
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
    GTest::GTest
    GTest::Main
    Threads::Threads
)

# Add compiler definitions
target_compile_definitions(test_config PRIVATE DDOGREEN_VERSION="test")

# Discover individual GoogleTest cases for automatic IDE integration
gtest_discover_tests(test_config
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 30
)
