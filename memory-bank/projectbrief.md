# Project Brief - ddogreen

## Project Overview
**ddogreen** - COMPLETE - A production-ready sustainability-focused power management service for PCs and laptops that automatically optimizes TLP (ThinkPad-Linux-Power) power settings based on intelligent system load monitoring.

**Product of DDOSoft Sustainability Solutions** - where "ddo" represents the company heritage and "green" represents our commitment to environmental sustainability and energy efficiency.

## Final Status: Cross-Platform Production Ready with Clean Architecture
- **Complete Application**: All features implemented and working on Linux and Windows
- **Clean Architecture**: Zero platform-specific code in main application layer
- **User Documentation**: Comprehensive README with cross-platform installation and usage
- **Service Integration**: Full systemd (Linux) and SCM (Windows) service management
- **Cross-Platform Architecture**: Platform abstraction layer complete with real implementations
- **Windows Functionality**: Complete Windows power plan and service management implementation
- **Build System**: Unified dual-platform build system producing both Linux and Windows binaries
- **Quality Assurance**: Clean build, verified executables, ready for cross-platform deployment

## Core Objective: ACHIEVED - Cross-Platform with Clean Architecture
Successfully created a robust power management daemon that:
- Automatically switches between performance and power-saving modes based on system load
- Uses configurable load average monitoring for stable activity detection  
- Operates with read-only configuration system for reliability and flexibility
- Operates as a proper system service (Linux systemd, Windows SCM)
- Provides complete user documentation and installation procedures for multiple platforms
- **ENHANCED**: Full Windows support with power plan management and Windows service integration
- **NEW**: Clean platform abstraction with zero conditional compilation in main application
- **NEW**: Unified dual-platform build system supporting simultaneous Linux and Windows compilation

## Project Type: COMPLETED
- **Language**: C++ (C++17 standard)
- **Build System**: CMake 3.16+  
- **Target Platform**: Linux primary, Windows support via platform abstraction
- **Architecture**: Cross-platform system daemon/service with compile-time optimization
- **Dependencies**: None (only standard C++ and platform-specific system calls)
- **Deliverable**: Production-ready executable with full functionality

## Key Requirements

### Functional Requirements
1. **Load Average Monitoring**: Cross-platform load monitoring (Linux: /proc/loadavg, Windows: Performance Counters)
2. **Power Management**: Platform-specific power control (Linux: TLP, Windows: powercfg)
3. **Service Management**: Platform-specific service control (Linux: systemd, Windows: SC)
4. **Configuration System**: Read-only configuration with comprehensive validation
5. **Cross-Platform Support**: Generic interfaces with platform-specific implementations

### Technical Requirements - All Achieved
1. **Configuration File**: `/etc/ddogreen/ddogreen.conf` with validated parameters
2. **Cross-Platform Compilation**: Platform-specific binaries with zero conditional compilation in main app
3. **Minimal Dependencies**: Only standard C++ libraries and platform-specific system calls
4. **Root Privileges**: Must run as root to execute power management commands
5. **Signal Handling**: Proper shutdown handling
6. **Error Handling**: Robust error handling with comprehensive configuration validation
7. **Platform Abstraction**: Clean separation with interface-based platform implementations
8. **Build System**: Unified dual-platform build supporting Linux and Windows simultaneously

### Performance Requirements
1. **Ultra-Low Resource Usage**: Minimal CPU, memory, and I/O footprint
2. **Efficient Monitoring**: Load average checks every 60 seconds only
3. **Fast Response**: Quick mode switching when activity state changes
4. **No Overhead**: No configuration parsing, validation, or file monitoring

## Hardcoded Settings
- **Load Threshold**: 0.15 (15% of one CPU core)
- **Idle Timeout**: 300 seconds (5 minutes)
- **Check Interval**: 60 seconds (1 minute)
- **Log File**: `/var/log/ddogreen.log`

## Target Users
- PC and laptop users seeking zero-maintenance automatic green power management
- Organizations focused on sustainability and energy efficiency
- Future expansion planned for server environments
- System administrators who need reliable, config-free power optimization
- Users of ThinkPad laptops with TLP who want "set and forget" operation
- Developers who need minimal-overhead background power management

## Success Criteria
1. Successful automatic switching between TLP modes based on load average
2. Reliable operation as a system service with zero configuration
3. Clear, timestamped logging with correct TLP terminology
4. Ultra-minimal resource usage (single load check per minute)
5. Warning-free compilation and deployment simplicity
4. Easy installation and configuration
5. Stable operation across different Linux distributions

## Project Scope
- **In Scope**: CPU-based activity monitoring, TLP integration, systemd service, logging
- **Out of Scope**: GUI interface, X11 event monitoring, complex power profiles, hardware-specific optimizations beyond TLP
