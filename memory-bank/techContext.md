# Tech Context - ddops

## Technology Stack

### Core Technologies
- **Language**: C++17 (Modern C++ features, standard library only)
- **Build System**: CMake 3.16+ (Modern CMake practices)
- **Target OS**: Linux primary, Windows support via platform abstraction
- **Architecture**: Cross-platform with compile-time platform selection
- **Service Framework**: systemd (Linux), Windows Service Manager (Windows)
- **Logging**: Custom implementation with logrotate integration

### Platform Abstraction
- **Design**: Generic interfaces with platform-specific implementations
- **Compilation**: Compile-time platform selection using preprocessor directives
- **Linux Support**: TLP integration, /proc filesystem, systemd services
- **Windows Support**: Mock implementations (powercfg, Performance Counters, sc.exe)
- **Binary Optimization**: Only target platform code included

### Dependencies

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
- ❌ X11 libraries (libx11-dev, libxss-dev)
- ❌ GUI frameworks (Qt, GTK)
- ❌ External C++ libraries (Boost, etc.)
- ❌ Scripting languages (Python, Shell beyond basic scripts)

## Development Environment Setup

### Platform-Specific Build Configuration

#### Linux Build
```bash
# Automatic platform detection in CMakeLists.txt
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        src/platform/linux/linux_power_manager.cpp
        src/platform/linux/linux_service_manager.cpp
        src/platform/linux/linux_system_monitor.cpp
    )
endif()
```

#### Windows Build (Mock)
```bash
# Windows platform source files
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        src/platform/windows/windows_power_manager.cpp
        src/platform/windows/windows_service_manager.cpp
        src/platform/windows/windows_system_monitor.cpp
    )
```

#### Platform Testing
```bash
# Standard build process
./build.sh                  # Build for current platform
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

# Clone and build
git clone <repository>
cd ddotlp
./build.sh
```

### Build Configuration
```cmake
# CMakeLists.txt key settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
```

## System Integration

### Systemd Service Integration
```ini
# /etc/systemd/system/ddotlp.service
[Unit]
Description=Dynamic TLP Power Management Daemon
After=multi-user.target

[Service]
Type=forking
ExecStart=/usr/local/bin/ddotlp --daemon
PIDFile=/var/run/ddotlp.pid
Restart=always
User=root
```

**Security Hardening**:
- `NoNewPrivileges=yes`
- `ProtectSystem=strict`
- `ProtectHome=yes`
- `PrivateTmp=yes`

### File System Layout
```
/usr/local/bin/ddotlp              # Main executable
/etc/systemd/system/ddotlp.service # Service definition
/etc/ddotlp/ddotlp.conf           # Configuration file
/etc/logrotate.d/ddotlp           # Log rotation config
/var/log/ddotlp.log               # Main log file
/var/run/ddotlp.pid               # Process ID file
```

### Configuration Management
- **Format**: Key-value pairs (shell-style)
- **Location**: `/etc/ddotlp/ddotlp.conf`
- **Parsing**: Simple string parsing (no external config libraries)
- **Validation**: Runtime validation with error logging

## Build System Details

### CMake Structure
```cmake
# Project definition
cmake_minimum_required(VERSION 3.16)
project(ddotlp VERSION 1.0.0)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source organization
set(SOURCES src/main.cpp src/activity_monitor.cpp ...)
include_directories(include)

# Installation rules
install(TARGETS ddotlp DESTINATION /usr/local/bin)
install(FILES systemd/ddotlp.service DESTINATION /etc/systemd/system)
```

### Build Scripts
- **build.sh**: Primary build script using CMake
- **install.sh**: Complete installation with dependency checking
- **uninstall.sh**: Clean removal of all installed components

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
├── ddotlp.conf          # Runtime configuration
└── ddotlp-logrotate     # Log rotation settings

systemd/
└── ddotlp.service       # Service definition
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
- **Location**: `/var/log/ddotlp.log`

### System Integration
- **Systemd Status**: `systemctl status ddotlp`
- **Journal Logs**: `journalctl -u ddotlp -f`
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
