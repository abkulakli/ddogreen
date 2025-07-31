# Tech Context - ddogreen

## Technology Stack

### Core Technologies
- **Language**: C++17 (Modern C++ features, standard library only)
- **Build System**: CMake 3.16+ (Modern CMake practices)
- **Target OS**: Cross-platform with equal Linux and Windows support
- **Architecture**: Cross-platform with compile-time platform selection
- **Service Framework**: systemd (Linux), Windows Service Manager (Windows)
- **Logging**: Custom implementation with logrotate integration

### Cross-Platform Build System (Current - August 2025)

#### CMake Presets Build Commands  
```bash
# Standard CMake preset workflow (VERIFIED WORKING)
cmake --preset debug                # Configure debug build
cmake --build --preset debug        # Build debug
cmake --preset release              # Configure release build  
cmake --build --preset release      # Build release

# Testing with presets (24 tests confirmed passing)
cmake --build --preset debug --target test
ctest --preset debug-tests

# Packaging from build directories
cd build/release && cpack
```

#### Windows CMake Build
```powershell
# Windows-specific CMake commands
cmake --preset debug               # Configure debug build
cmake --build --preset debug       # Build debug executable
cmake --preset release             # Configure release build  
cmake --build --preset release     # Build release executable

# Windows packaging (ZIP archive)
cd build/release && cpack          # Generate ZIP package
```

#### Build Directories Structure (VERIFIED CURRENT)
```
build/
├── debug/          # Debug configuration with tests
├── release/        # Release configuration (not present in current workspace)
└── test/           # Testing configuration (currently active)
    ├── test_config # Compiled test executable
    └── lib/        # GoogleTest libraries
```

#### Package Structure Reality (STANDARDIZED IMPLEMENTATION)
**CURRENT UNIFIED IMPLEMENTATION**:
```cmake
# All platforms: Use standardized FHS-compliant paths
install(FILES "${CMAKE_SOURCE_DIR}/config/ddogreen.conf.default"
        DESTINATION share/ddogreen)  # Consistent across all platforms
```

**Package Output Structure (STANDARDIZED)**:
```
All Packages (Linux DEB/RPM/TGZ + Windows ZIP):
├── bin/ddogreen(.exe)                        # Executable binary
├── share/ddogreen/ddogreen.conf.default      # Configuration file (standardized location)
└── installer.{sh|bat}                        # Platform-specific installer (TGZ/ZIP only)
```

**Installer Behavior**:
- **Windows installer**: Expects `share\ddogreen\ddogreen.conf.default` - fails fast if missing
- **Linux installer**: Expects `share/ddogreen/ddogreen.conf.default` - fails fast if missing  
- **Package installers (DEB/RPM)**: Use standardized `/usr/share/ddogreen/` location
- **Error Handling**: Clear error messages if expected standardized structure is corrupted

### Build System

#### Current Build System (Updated July 2025)
```bash
# CMake presets workflow
cmake --preset debug                # Debug build configuration
cmake --build --preset debug        # Build debug executable
cmake --preset release              # Release build configuration  
cmake --build --preset release      # Build release executable

# Cross-platform standard commands
cmake --preset debug|release        # Configure build
cmake --build --preset debug|release # Build executable
```

#### Build Requirements
- **cmake**: 3.16 or later
- **make**: GNU make or compatible (Linux)
- **gcc/g++**: C++17 support required
- **Visual Studio or MinGW**: For Windows builds with CMake

#### Current Build Features
- **CMake Presets**: VS Code integration with standard workflow (VERIFIED WORKING)
- **Separated Builds**: Debug and release in separate directories
- **Testing Integration**: GoogleTest via `BUILD_TESTS=ON` (24 tests passing)
- **Cross-Platform**: Native builds per platform
- **Windows Packaging**: ZIP archives with custom installer scripts
- **Service Installation**: Executable service options removed; package installers handle service setup
  - **Windows Service Installation**: Uses custom installer.bat script for service management

### Platform Abstraction
- **Design**: Generic interfaces with platform-specific implementations
- **Compilation**: Compile-time platform selection using preprocessor directives
- **Linux Support**: TLP integration, /proc filesystem, systemd services, realpath
- **Windows Support**: Power Plans, Performance Counters, SCM, GetFullPathName
- **Binary Optimization**: Only target platform code included
- **Real Implementation**: No mock code - all platforms have functional implementations
- **Dynamic Load Calculation**: Windows calculates proper load averages using configurable frequency

### Cross-Platform Runtime Dependencies
```yaml
Linux Runtime:
  - TLP (ThinkPad Linux Advanced Power Management)
  - systemd (service management)
  - /proc filesystem (load average, CPU info)

Windows Runtime:
  - Windows Performance Toolkit (built-in)
  - Windows Service Control Manager (built-in)
  - powercfg command (built-in)
  - PDH (Performance Data Helper) API (built-in)

macOS Runtime (Future):
  - pmset command (built-in)
  - launchd (service management, built-in)
  - system APIs (load monitoring, built-in)
```
- **Visual Studio 2019+** or **MinGW-w64** with C++17 support
- **CMake** 3.16 or later
- **Git** for version control
- **vcpkg** (optional, for dependency management)

#### macOS Development Dependencies
```bash
# Using Homebrew
brew install cmake googletest
```

#### Cross-Compilation Dependencies (Linux Host → Windows Target)
```bash
# For Windows cross-compilation (if needed via MinGW)
sudo apt install mingw-w64 mingw-w64-tools

# ZIP packaging for Windows (NSIS removed)
sudo apt install zip

# Note: Primary Windows development uses CMake presets with native tools
```

#### Build Dependencies
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake

# Fedora/RHEL
sudo dnf install gcc-c++ cmake
```

#### Runtime Dependencies
- **TLP**: Power management backend (installed separately)
- **Linux Kernel**: Access to `/proc/stat` (standard on all Linux systems)
- **systemd**: For service management (standard on modern Linux distributions)

#### Explicitly NO Dependencies
- X11 libraries (libx11-dev, libxss-dev)
- GUI frameworks (Qt, GTK)
- External C++ libraries (Boost, etc.)
- Scripting languages (Python, Shell beyond basic scripts)

## Development Environment Setup

### Platform-Specific Build Configuration

#### Linux Build
```bash
# Automatic platform detection in CMakeLists.txt
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        src/platform/linux/linux_power_manager.cpp
        src/platform/linux/linux_system_monitor.cpp
        src/platform/linux/linux_platform_utils.cpp
        src/platform/linux/linux_daemon.cpp
    )
endif()
```

#### Windows Build (Mock)
```bash
# Windows platform source files
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
        src/platform/windows/windows_platform_utils.cpp
        src/platform/windows/windows_daemon.cpp
    )
```

#### Platform Testing
```bash
# Standard build process
cmake --preset debug && cmake --build --preset debug    # Build for current platform
# Platform abstraction automatically detected and compiled
```

### Minimum Requirements
- **GCC**: 7.0+ (C++17 support)
- **CMake**: 3.16+ (Modern CMake features)
- **Linux Kernel**: 2.6+ (for `/proc/stat` format)
- **systemd**: Any recent version (for service features)

### Recommended Development Setup
```bash
# Install development tools
sudo apt update
sudo apt install build-essential cmake git

# Install TLP for testing
sudo apt install tlp

# Install testing dependencies (optional)
sudo apt install libgtest-dev

# Clone and build
git clone <repository>
cd ddogreen
cmake --preset release
cmake --build --preset release
```

### Build Configuration
```cmake
# CMakeLists.txt key settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Manual CMake Build Process
```bash
# Standard CMake build (current approach)
mkdir -p build/release
cd build/release

# Configure for release
cmake ../.. -DCMAKE_BUILD_TYPE=Release

# Configure for development with tests
cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .
```

### Available Build Targets
```bash
make ddogreen                     # Main executable
make test_config                  # Unit tests
make package                      # Create distribution packages
```

### Package Generation
```bash
# Linux packages
cpack -G DEB                      # Debian/Ubuntu package
cpack -G RPM                      # Fedora/RHEL package  
cpack -G TGZ                      # Generic tarball

# Windows packages
cpack -G ZIP                      # Windows ZIP package
cpack -G ZIP                      # Windows ZIP archive
```

### Testing Integration
```bash
# Using CMake presets
cmake --preset debug               # Configure with tests enabled
cmake --build --preset debug       # Build with tests
ctest --preset debug-tests         # Run tests

# Manual test execution
cd build/debug
ctest --output-on-failure

# Run specific tests
./tests/test_config
./tests/test_config --gtest_filter="TestConfig.test_load_from_file_*"
```

### Performance Targets
- **Memory**: Target <5MB RSS during operation
- **CPU**: Minimal impact with 60-second check intervals
- **Disk**: Log rotation to prevent unbounded growth
- **Network**: No network dependencies

### Debugging Setup
```bash
# Debug build using presets
cmake --preset debug
cmake --build --preset debug
gdb ./build/debug/ddogreen
```

### Platform-Specific Debugging

#### Linux Debug Commands
```bash
# Check TLP status
sudo tlp-stat -s

# Monitor system logs
sudo journalctl -f -u ddogreen

# Check load average
cat /proc/loadavg
```

#### Windows Debug Commands
```cmd
# Check power plans
powercfg /list
powercfg /getactivescheme

# Check service status
sc query ddogreen

# Monitor Windows Event Log
eventvwr.msc
```

## System Integration

### Systemd Service Integration
```ini
# /etc/systemd/system/ddogreen.service
[Unit]
Description=Dynamic TLP Power Management Daemon
After=multi-user.target

[Service]
Type=forking
ExecStart=/usr/local/bin/ddogreen --daemon
PIDFile=/var/run/ddogreen.pid
Restart=always
User=root
```

**Security Hardening**:
- `NoNewPrivileges=yes`
- `ProtectSystem=strict`
- `ProtectHome=yes`
- `PrivateTmp=yes`

## Configuration System

### Configuration Architecture
- **Read-Only Configuration**: Values loaded once at startup from file
- **File Location**: `/etc/ddogreen/ddogreen.conf` (Linux)
- **Format**: Key-value pairs with validation and error handling
- **No Auto-Creation**: Configuration file must be manually created
- **No Runtime Modification**: All changes require application restart

### Configuration Parameters
```ini
# CPU load thresholds (decimal percentages per core)
high_performance_threshold=0.70  # Switch to high performance (0.1-1.0)
power_save_threshold=0.30        # Switch to power save (0.05-0.9)

# Monitoring frequency in seconds
monitoring_frequency=10          # Check interval (1-300 seconds)
```

### Validation Rules
- **high_performance_threshold**: 0.1 to 1.0 (10% to 100%)
- **power_save_threshold**: 0.05 to 0.9 (5% to 90%)
- **monitoring_frequency**: 1 to 300 seconds
- **Missing Parameters**: Application fails with clear error messages
- **Invalid Values**: Application fails with validation errors

### Configuration Setup Process
1. **Create Directory**: `sudo mkdir -p /etc/ddogreen`
2. **Copy Template**: `sudo cp example-config/ddogreen.conf /etc/ddogreen/`
3. **Edit Values**: `sudo nano /etc/ddogreen/ddogreen.conf`
4. **Start Service**: Configuration automatically loaded at startup

### File System Layout
```
/usr/local/bin/ddogreen              # Main executable
/etc/systemd/system/ddogreen.service # Service definition
/etc/ddogreen/ddogreen.conf             # Configuration file
/etc/logrotate.d/ddogreen            # Log rotation config
/var/log/ddogreen.log                # Main log file
/var/run/ddogreen.pid                # Process ID file
```

### Configuration Management
- **Format**: Key-value pairs (shell-style)
- **Location**: `/etc/ddogreen/ddogreen.conf`
- **Parsing**: Simple string parsing (no external config libraries)
- **Validation**: Runtime validation with error logging

## Build System Details

### CMake Structure
```cmake
# Project definition
cmake_minimum_required(VERSION 3.16)
project(ddogreen VERSION 1.0.0)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source organization
set(SOURCES src/main.cpp src/activity_monitor.cpp ...)
include_directories(include)

# Installation rules
install(TARGETS ddogreen DESTINATION /usr/local/bin)
install(FILES systemd/ddogreen.service DESTINATION /etc/systemd/system)
```

### Build Configuration
- **CMakePresets.json**: Defines debug and release build configurations for cross-platform development
- **Standard CMake**: Uses industry-standard CMake commands and presets for all platforms
- **VS Code Tasks**: Integrated build, test, and debug workflows

## Code Organization

### Header Structure
```
include/
├── activity_monitor.h    # CPU monitoring and activity detection
├── config.h             # Compile-time configuration constants
├── daemon.h             # Daemon lifecycle management
├── logger.h             # Logging system interface
└── tlp_manager.h        # TLP command execution
```

### Source Structure
```
src/
├── main.cpp             # Entry point and command-line handling
├── activity_monitor.cpp # CPU monitoring implementation
├── daemon.cpp           # Daemonization and signal handling
├── logger.cpp           # Logging implementation
└── tlp_manager.cpp      # TLP command execution
```

### Configuration Files
```
config/
├── ddogreen.conf          # Runtime configuration
└── ddogreen-logrotate     # Log rotation settings

systemd/
└── ddogreen.service       # Service definition
```

## Technical Constraints

### Platform Limitations
- **Linux Only**: Relies on `/proc/stat` and systemd
- **Root Required**: TLP commands require root privileges
- **systemd Required**: Service management assumes systemd

### Performance Constraints
- **Memory Usage**: Target <10MB resident memory
- **CPU Usage**: <1% CPU usage during normal operation
- **Disk I/O**: Minimal (only logging and periodic `/proc/stat` reads)
- **Network**: None (entirely local operation)

### Security Constraints
- **Root Privileges**: Required for TLP execution
- **File Access**: Limited to specific directories via systemd
- **Network Access**: None required or permitted
- **Process Isolation**: Protected by systemd security features

## Deployment Considerations

### Installation Methods
1. **Manual Build**: Clone, build, install from source
2. **Package Installation**: Future DEB/RPM packages
3. **Container Deployment**: Docker/Podman for testing

### Distribution Compatibility
- **Ubuntu**: 18.04+ (CMake 3.16+ requirement)
- **Debian**: 10+ (Buster and newer)
- **Fedora**: 30+ (Modern GCC and CMake)
- **RHEL/CentOS**: 8+ (Developer toolset may be required)

### Runtime Environment
- **TLP Installation**: Must be installed and configured separately
- **Systemd**: Must be running and available
- **Permissions**: Service must run as root
- **Log Rotation**: Automatic via logrotate integration

## Monitoring and Observability

### Logging System
- **Format**: Structured logs with millisecond timestamps
- **Levels**: DEBUG, INFO, WARNING, ERROR
- **Rotation**: Daily rotation, 30-day retention
- **Location**: `/var/log/ddogreen.log`

### System Integration
- **Systemd Status**: `systemctl status ddogreen`
- **Journal Logs**: `journalctl -u ddogreen -f`
- **Process Monitoring**: Standard process monitoring tools
- **Health Checks**: Service restart on failure

### Debugging Tools
- **Debug Mode**: Run without daemon mode for interactive debugging
- **Verbose Logging**: DEBUG level for detailed operation traces
- **Configuration Testing**: Command-line parameter validation
- **TLP Integration Testing**: Manual command execution verification

## Future Technical Considerations

### Potential Enhancements
- **Configuration Hot Reload**: SIGHUP handler for config changes
- **Metrics Export**: Prometheus metrics endpoint
- **Advanced Activity Detection**: Multiple detection strategies
- **Power Profile Management**: Custom power profiles beyond TLP

### Scalability Considerations
- **Multi-System Management**: Central configuration management
- **Container Support**: Privileged container deployment
- **Cloud Integration**: Cloud instance power management
- **Embedded Systems**: ARM and embedded Linux support

### Technology Evolution
- **C++20 Migration**: Future language standard adoption
- **Alternative Builds**: Meson build system support
- **Package Management**: Native distribution packages
- **Alternative Backends**: Support for other power management systems
