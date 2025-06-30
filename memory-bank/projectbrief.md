# Project Brief - ddotlp

## Project Overview
**ddotlp** (Dynamic DOT TLP) is a Linux system service that automatically manages TLP (ThinkPad-Linux-Power) power settings based on real-time system activity monitoring.

## Core Objective
Create an intelligent power management daemon that:
- Automatically switches between performance and battery-saving modes
- Uses CPU usage monitoring to detect system activity (no GUI dependencies)
- Runs as a proper Linux systemd service
- Provides comprehensive logging and monitoring capabilities

## Project Type
- **Language**: C++ (C++17 standard)
- **Build System**: CMake 3.16+
- **Target Platform**: Linux systems (Ubuntu, Debian, Fedora, RHEL)
- **Architecture**: System daemon/service
- **Dependencies**: Minimal (no external libraries, only standard C++ and system calls)

## Key Requirements

### Functional Requirements
1. **Activity Detection**: Monitor CPU usage via `/proc/stat` to determine system activity
2. **Power Management**: Execute TLP commands (`tlp start` for performance, `tlp bat` for battery)
3. **Configuration**: Support configurable idle timeout and CPU usage thresholds
4. **Service Integration**: Full systemd integration with proper daemon lifecycle
5. **Logging**: Detailed logging with millisecond timestamps

### Technical Requirements
1. **No GUI Dependencies**: Must work on headless servers and any display system
2. **Minimal Dependencies**: Only standard C++ libraries and Linux system calls
3. **Root Privileges**: Must run as root to execute TLP commands
4. **Signal Handling**: Proper shutdown and configuration reload support
5. **Error Handling**: Robust error handling and recovery mechanisms

### Performance Requirements
1. **Low Resource Usage**: Minimal CPU and memory footprint
2. **Efficient Monitoring**: CPU checks every 5 seconds to avoid overhead
3. **Fast Response**: Quick mode switching when activity state changes

## Target Users
- Linux laptop users wanting automatic power management
- System administrators managing multiple Linux systems
- Users of ThinkPad laptops with TLP installed
- Developers working on Linux systems who want seamless power optimization

## Success Criteria
1. Successful automatic switching between TLP modes based on activity
2. Reliable operation as a system service
3. Clear, timestamped logging for monitoring and debugging
4. Easy installation and configuration
5. Stable operation across different Linux distributions

## Project Scope
- **In Scope**: CPU-based activity monitoring, TLP integration, systemd service, logging
- **Out of Scope**: GUI interface, X11 event monitoring, complex power profiles, hardware-specific optimizations beyond TLP
