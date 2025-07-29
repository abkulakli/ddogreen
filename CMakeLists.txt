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
# Use different paths based on the packaging generator
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # For DEB and RPM packages: use standard FHS paths
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
            DESTINATION share/ddogreen
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
else()
    # For Windows packages: use data directory
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config/ddogreen.conf.default"
            DESTINATION data
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
    # Windows-specific packaging with NSIS
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_PACKAGE_FILE_NAME "ddogreen-windows")
    
    # NSIS-specific settings for Windows installer
    set(CPACK_NSIS_DISPLAY_NAME "DDOGreen ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "DDOGreen ${PROJECT_VERSION}")
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "ddogreen.exe")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://www.ddosoft.com")
    set(CPACK_NSIS_CONTACT "support@ddosoft.com")
    
    # Icon and branding (optional, uses default NSIS icons if not found)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/ddogreen.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\packaging\\\\windows\\\\ddogreen.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\packaging\\\\windows\\\\ddogreen.ico")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\ddogreen.exe")
    endif()
    
    # License file (optional)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_NSIS_MENU_LINKS "LICENSE" "License")
    endif()
    
    # Windows service installation commands
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        ; Install Windows service
        ExecWait '$INSTDIR\\\\bin\\\\ddogreen.exe --install-service' $0
        DetailPrint 'Service installation exit code: $0'
        ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        ; Uninstall Windows service
        ExecWait '$INSTDIR\\\\bin\\\\ddogreen.exe --uninstall-service' $0
        DetailPrint 'Service uninstallation exit code: $0'
        ")
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
set(CPACK_PACKAGE_VENDOR "DDOSoft Sustainability Solutions")
set(CPACK_PACKAGE_CONTACT "support@ddosoft.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.ddosoft.com")

# Linux-specific packaging configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Debian-specific settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DDOSoft Sustainability Solutions <support@ddosoft.com>")
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
    set(CPACK_RPM_PACKAGE_VENDOR "DDOSoft Sustainability Solutions")
    set(CPACK_RPM_PACKAGE_REQUIRES "tlp >= 1.0")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/post")
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/preun")
    set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/postun")
endif()

include(CPack)

# Testing configuration
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
