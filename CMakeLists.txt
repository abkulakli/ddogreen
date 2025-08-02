cmake_minimum_required(VERSION 3.16)

# Set version from command line argument or default to development version
if(NOT DEFINED PROJECT_VERSION_OVERRIDE)
    set(PROJECT_VERSION_OVERRIDE "0.0.0")
endif()

project(ddogreen VERSION ${PROJECT_VERSION_OVERRIDE}
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
    src/config.cpp
    src/platform/platform_factory.cpp
    src/platform/idaemon.cpp
)

# Platform-specific source files
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        src/platform/linux/linux_power_manager.cpp
        src/platform/linux/linux_system_monitor.cpp
        src/platform/linux/linux_platform_utils.cpp
        src/platform/linux/linux_daemon.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
        src/platform/windows/windows_platform_utils.cpp
        src/platform/windows/windows_daemon.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        src/platform/macos/macos_power_manager.cpp
        src/platform/macos/macos_system_monitor.cpp
        src/platform/macos/macos_platform_utils.cpp
        src/platform/macos/macos_daemon.cpp
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

# Platform-specific libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Link Windows-specific libraries for Performance Counters
    target_link_libraries(ddogreen pdh)
endif()

# Installation
install(TARGETS ddogreen DESTINATION bin)

# Install configuration file
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # For Windows, install config file both in share/ddogreen (for consistency) and at root (for MSI)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
            DESTINATION share/ddogreen
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
else()
    # Use share/ddogreen consistently for Linux platforms
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
            DESTINATION share/ddogreen
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()

# Install installer scripts for ZIP packages
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/tgz/installer.sh"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/zip/installer.bat"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()

# CPack configuration for platform-specific packages
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows-specific packaging with ZIP and WIX (for MSI installer)
    set(CPACK_GENERATOR "ZIP;WIX")
    set(CPACK_PACKAGE_FILE_NAME "ddogreen-windows")
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
set(CPACK_PACKAGE_VENDOR "DDOSoft Software Solutions")
set(CPACK_PACKAGE_CONTACT "support@ddosoft.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.ddosoft.com")

# Linux-specific packaging configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Debian-specific settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DDOSoft Software Solutions <support@ddosoft.com>")
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
    set(CPACK_RPM_PACKAGE_VENDOR "DDOSoft Software Solutions")
    set(CPACK_RPM_PACKAGE_REQUIRES "tlp >= 1.0")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/post")
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/preun")
    set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/postun")
endif()

# Windows-specific WIX packaging configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # WIX MSI installer configuration
    set(CPACK_WIX_UPGRADE_GUID "8B9A5C1D-7F2E-4A6B-9C3D-E5F8A1B4C7D2")  # Fixed GUID for upgrades
    set(CPACK_WIX_UI_REF "WixUI_InstallDir")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "DDOSoft")
    set(CPACK_WIX_CULTURES "en-us")
    
    # MSI Properties for Add/Remove Programs
    set(CPACK_WIX_PROPERTY_ARPCOMMENTS "Intelligent Green Power Management for Sustainable Computing")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://www.ddosoft.com/support")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://www.ddosoft.com")
    set(CPACK_WIX_PROPERTY_ARPURLUPDATEINFO "https://www.ddosoft.com/updates")
    
    # Service installation via WIX patch file
    set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/wix/service-patch.xml")
    
    # Set install directory
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "DDOSoft/DDOGreen")
endif()

include(CPack)

# Testing configuration
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
