# Project Brief - ddogreen

## Project Overview
**ddogreen** is an ultra-minimal sustainability-focused power management service for PCs and laptops that automatically optimizes TLP (ThinkPad-Linux-Power) power settings based on intelligent system load monitoring.

**Product of DDOSoft Sustainability Solutions** - where "ddo" represents the company heritage and "green" represents our commitment to environmental sustainability and energy efficiency.

## Core Objective
Create the simplest possible power management daemon that:
- Automatically switches between TLP auto and battery modes based on system load
- Uses 1-minute load average monitoring for stable activity detection
- Runs with hardcoded settings for maximum reliability and minimal resource usage
- Operates as a proper Linux systemd service with zero configuration required

## Project Type
- **Language**: C++ (C++17 standard)
- **Build System**: CMake 3.16+
- **Target Platform**: Linux primary, Windows support via platform abstraction
- **Architecture**: Cross-platform system daemon/service with compile-time optimization
- **Dependencies**: None (only standard C++ and platform-specific system calls)

## Key Requirements

### Functional Requirements
1. **Load Average Monitoring**: Cross-platform load monitoring (Linux: /proc/loadavg, Windows: Performance Counters)
2. **Power Management**: Platform-specific power control (Linux: TLP, Windows: powercfg)
3. **Service Management**: Platform-specific service control (Linux: systemd, Windows: SC)
4. **Hardcoded Configuration**: All settings built into binary for maximum reliability
5. **Cross-Platform Support**: Generic interfaces with platform-specific implementations

### Technical Requirements
1. **Zero Configuration**: No configuration files, minimal command-line options
2. **Compile-Time Platform Selection**: Only target platform code included in binaries
2. **Minimal Dependencies**: Only standard C++ libraries and Linux system calls
3. **Root Privileges**: Must run as root to execute TLP commands
4. **Signal Handling**: Proper shutdown handling (no configuration reload)
5. **Error Handling**: Robust error handling without configuration complexity

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
