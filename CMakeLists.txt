cmake_minimum_required(VERSION 3.16)

# Set version from command line argument or default to development version
if(NOT DEFINED PROJECT_VERSION_OVERRIDE)
    set(PROJECT_VERSION_OVERRIDE "0.0.0")
endif()

project(ddogreen VERSION ${PROJECT_VERSION_OVERRIDE}
        DESCRIPTION "Intelligent Green Power Management for Sustainable Computing"
        HOMEPAGE_URL "https://www.ddosoft.com")

set(CMAKE_CXX_STANDARD 20)
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
    src/logger.cpp
    src/config.cpp
    src/platform/platform_factory.cpp
)

# Platform-specific source files
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        src/platform/linux/linux_power_manager.cpp
        src/platform/linux/linux_system_monitor.cpp
        src/platform/linux/linux_platform_utils.cpp
        src/platform/linux/linux_signal_handler.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
        src/platform/windows/windows_platform_utils.cpp
        src/platform/windows/windows_signal_handler.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        src/platform/macos/macos_power_manager.cpp
        src/platform/macos/macos_system_monitor.cpp
        src/platform/macos/macos_platform_utils.cpp
        src/platform/macos/macos_signal_handler.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Include directories
include_directories(include)

# Create executable
add_executable(ddogreen ${SOURCES})

# Add version definition for use in source code
target_compile_definitions(ddogreen PRIVATE DDOGREEN_VERSION="${PROJECT_VERSION}")

# Apply coverage flags if coverage is enabled
if(BUILD_WITH_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ddogreen PRIVATE --coverage -g -O0)
    target_link_options(ddogreen PRIVATE --coverage)
endif()

# Platform-specific libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Link Windows-specific libraries for Performance Counters
    target_link_libraries(ddogreen pdh)
endif()

# Installation
install(TARGETS ddogreen DESTINATION bin)

# Install configuration file
# Use share/ddogreen consistently across all platforms for better standardization
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
        DESTINATION share/ddogreen
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Install installer scripts for ZIP packages
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/tgz/installer.sh"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/zip/installer.bat"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    # Install NSSM service wrapper for Windows
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/nssm.exe"
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

# CPack configuration for platform-specific packages
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows-specific packaging with ZIP and MSI
    set(CPACK_GENERATOR "ZIP;WIX")
    set(CPACK_PACKAGE_FILE_NAME "ddogreen-windows")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)  # Prevent nested folder in ZIP

    # WiX-specific settings for MSI generation (WiX v5 compatible)
    set(CPACK_WIX_VERSION "4")  # Use WiX .NET Tools (v4/v5) instead of legacy v3
    set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-7890-ABCD-EF1234567890")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "DDOSoft")
    set(CPACK_WIX_PROPERTY_INSTALLSCOPE "perMachine")

    # Use our custom WiX template with service installation
    set(CPACK_WIX_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/msi/ddogreen.wxs")
else()
    # Linux packaging with DEB, RPM, and TGZ
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_PACKAGE_FILE_NAME "ddogreen-linux")
    set(CPACK_DEBIAN_FILE_NAME "ddogreen-linux.deb")
    set(CPACK_RPM_FILE_NAME "ddogreen-linux.rpm")
endif()

set(CPACK_PACKAGE_NAME "ddogreen")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Intelligent Green Power Management for Sustainable Computing")
set(CPACK_PACKAGE_DESCRIPTION "A smart sustainability-focused service that automatically optimizes your PC and laptop power consumption by intelligently switching between high-performance and power-saving modes based on actual system usage patterns.")
set(CPACK_PACKAGE_VENDOR "DDOSoft Solutions")
set(CPACK_PACKAGE_CONTACT "support@ddosoft.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.ddosoft.com")

# Linux-specific packaging configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Debian-specific settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DDOSoft Solutions <support@ddosoft.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "tlp (>= 1.0)")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/postinst;${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/postrm")

    # RPM-specific settings
    set(CPACK_RPM_PACKAGE_SUMMARY "Intelligent Green Power Management for Sustainable Computing")
    set(CPACK_RPM_PACKAGE_DESCRIPTION "A smart sustainability-focused service that automatically optimizes your PC and laptop power consumption by intelligently switching between high-performance and power-saving modes based on actual system usage patterns.")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_VENDOR "DDOSoft Solutions")
    set(CPACK_RPM_PACKAGE_REQUIRES "tlp >= 1.0")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/post")
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/preun")
    set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/postun")
endif()

include(CPack)

# Testing and Coverage configuration
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_WITH_COVERAGE "Build with coverage reporting" OFF)

# Coverage configuration
if(BUILD_WITH_COVERAGE)
    if(NOT BUILD_TESTS)
        message(FATAL_ERROR "Coverage requires BUILD_TESTS to be ON")
    endif()
    
    # Check if we're using GCC or Clang (gcov compatible)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Add coverage flags for all source files
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g -O0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -g -O0")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        
        # Find required coverage tools
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(NOT GCOV_PATH)
            message(WARNING "gcov not found - coverage reports may not work properly")
        endif()
        
        if(LCOV_PATH AND GENHTML_PATH)
            message(STATUS "Found lcov and genhtml - HTML coverage reports will be available")
            
            # Create coverage target
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
                
                # Cleanup previous coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters
                
                # Initialize coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --initial --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_base.info --ignore-errors inconsistent,negative,unused
                
                # Run tests
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                
                # Capture coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_test.info --ignore-errors inconsistent,negative,unused
                
                # Combine baseline and test coverage data
                COMMAND ${LCOV_PATH} --add-tracefile ${CMAKE_BINARY_DIR}/coverage/coverage_base.info --add-tracefile ${CMAKE_BINARY_DIR}/coverage/coverage_test.info --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_combined.info --ignore-errors inconsistent,negative,unused
                
                # Filter out system headers and test files
                COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage/coverage_combined.info 
                    '/usr/*' 
                    '*/tests/*' 
                    '*/test/*'
                    '*/_deps/*'
                    '*/build/*'
                    '*/.cache/*'
                    --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info --ignore-errors inconsistent,negative,unused
                
                # Generate HTML report
                COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info --output-directory ${CMAKE_BINARY_DIR}/coverage/html --title "DDOGreen Coverage Report" --num-spaces 4 --legend --demangle-cpp
                
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at: ${CMAKE_BINARY_DIR}/coverage/html/index.html"
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ddogreen
                COMMENT "Generating coverage report"
            )
            
            # Create a simpler coverage target that just shows console summary
            add_custom_target(coverage-summary
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
                
                # Cleanup previous coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters
                
                # Run tests
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                
                # Capture and filter coverage data with error handling
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info --ignore-errors inconsistent,negative,unused
                COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info 
                    '/usr/*' 
                    '*/tests/*' 
                    '*/test/*'
                    '*/_deps/*'
                    '*/build/*'
                    '*/.cache/*'
                    --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info --ignore-errors inconsistent,negative,unused
                
                # Show summary
                COMMAND ${LCOV_PATH} --summary ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info --ignore-errors inconsistent,negative,unused
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ddogreen
                COMMENT "Generating coverage summary"
            )
        else()
            message(WARNING "lcov and/or genhtml not found - HTML coverage reports will not be available. Install lcov package.")
            
            # Create a basic gcov target
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage data generated. Use gcov manually to analyze .gcno/.gcda files in build directory."
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ddogreen
                COMMENT "Running tests with coverage"
            )
        endif()
    else()
        message(WARNING "Coverage is only supported with GCC or Clang compilers")
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
