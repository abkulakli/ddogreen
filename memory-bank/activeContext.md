# Active Context - ddops

## Current Work Focus
**Project Status**: UPDATED - Console Logging for Non-Daemon Mode Implemented  
**Last Updated**: July 17, 2025
**Current Priority**: Complete executable with enhanced logging and service management

## Latest Update: Documentation Consolidation ✅

### Unified Documentation Strategy
- **README.md**: Primary user-facing documentation with complete usage guide
- **Memory Bank**: Development context and technical implementation details  
- **Removed USAGE.md**: Redundant file eliminated, all usage info moved to README.md
- **GitHub-First Approach**: README.md is what users see first, contains everything needed

### Enhanced README.md Content
- **Command-line options**: Complete reference for all flags (-d, -h, -v, -i, -u)
- **Service management**: Built-in install/uninstall commands documented
- **Interactive vs Daemon modes**: Clear explanation of both logging approaches
- **Installation methods**: Both script-based and manual installation paths
- **Real-world benefits**: Energy savings, cost reduction, environmental impact

### Console Logging for Non-Daemon Mode
- **Dual Logging Mode**: Automatic selection between console and file logging
- **Non-Daemon Mode**: All log messages (DEBUG, INFO, WARNING, ERROR) printed to console
- **Daemon Mode**: All messages logged to file (`/var/log/ddops.log`), only errors/warnings to stderr
- **Smart Initialization**: Logger automatically detects daemon mode and configures output accordingly
- **Real-Time Monitoring**: When running interactively, users can see live activity monitoring and TLP switching

### Logging Behavior
1. **Interactive Mode** (`ddops` without `-d`):
   - All logs displayed on console with timestamps
   - Real-time visibility of load monitoring and TLP mode changes
   - Perfect for testing and debugging
   - Example: `[2025-07-17 01:21:30.728] [INFO] System idle - switching to TLP battery mode`

2. **Daemon Mode** (`ddops -d`):
   - Silent console operation
   - All logs written to `/var/log/ddops.log`
   - Errors and warnings still shown on stderr for critical issues
   - Suitable for production service deployment

## Latest Update: Service Management Integration ✅

### Built-in Service Installation/Uninstallation
- **Command-Line Options**: 
  - `-i, --install`: Install system service
  - `-u, --uninstall`: Uninstall system service
- **Platform Integration**: Uses existing IServiceManager interface through PlatformFactory
- **Root Privilege Checking**: Proper validation for service management operations
- **User-Friendly Output**: Clear instructions for enabling and starting services
- **Executable Path Detection**: Automatic detection of current executable path for service installation

### Service Management Features
1. **Install Service**: `sudo ddops --install` or `sudo ddops -i`
   - Creates systemd service file in `/etc/systemd/system/ddops.service`
   - Configures proper daemon mode execution with `--daemon` flag
   - Sets up PID file management and automatic restart
   - Reloads systemd daemon
   - Provides clear next steps for user

2. **Uninstall Service**: `sudo ddops --uninstall` or `sudo ddops -u`
   - Stops and disables service if running
   - Removes systemd service file
   - Reloads systemd daemon
   - Clean removal with proper cleanup

3. **Security**: Both operations require root privileges with clear error messages
4. **Error Handling**: Comprehensive validation and user feedback

### Enhanced Usage Information
```
Usage: ddops [OPTIONS]
Options:
  -d, --daemon           Run as daemon
  -h, --help             Show this help message
  -v, --version          Show version information
  -i, --install          Install system service
  -u, --uninstall        Uninstall system service

Service Management:
  Install:   sudo ddops --install (or -i)
  Uninstall: sudo ddops --uninstall (or -u)
```

## Latest Update: Platform Abstraction Layer ✅

### Cross-Platform Architecture Implemented
- **Generic Interfaces**: IPowerManager, ISystemMonitor, IServiceManager in `include/platform/`
- **Platform Implementations**: Source files only in `src/platform/linux/` and `src/platform/windows/`
- **Compile-Time Selection**: Preprocessor directives ensure only target platform code is compiled
- **Windows Mock Support**: Complete mock implementations for Windows platform testing
- **Binary Optimization**: Linux binaries contain 45 Linux symbols, 0 Windows symbols (1.8M size)

### Platform Capabilities
1. **Get CPU Load**: `systemMonitor->getLoadAverages()` - cross-platform load monitoring
2. **Switch Mode**: `powerManager->setPerformanceMode()` / `setPowerSavingMode()` - unified power management  
3. **Install/Uninstall Service**: `serviceManager->installService()` / `uninstallService()` - service management

### Implementation Benefits
- **Zero Runtime Overhead**: All platform selection happens at compile time
- **Optimal Binary Size**: No unused platform code included in binaries
- **Clean Architecture**: Application code only uses generic interfaces
- **Mock Testing**: Windows implementations log exact commands for future implementation
- **Maintainable**: Adding new platforms doesn't affect existing code

### Enhanced Load Monitoring Algorithm
- **Check Interval**: 30 seconds for responsive monitoring
- **Threshold Logic**: 
  - **1-minute load > 10%** → Switch to TLP AC mode (`tlp ac`)
  - **Both 5-minute AND 15-minute load ≤ 10%** → Switch to TLP battery mode (`tlp bat`)
  - **Overlap zone** (1-min ≤ 10% AND either 5-min > 10% OR 15-min > 10%) → Maintain current state
- **Load Average Source**: Reads 1-minute, 5-minute, and 15-minute values from `/proc/loadavg`
- **Conservative Battery Switch**: Requires both medium-term (5-min) and long-term (15-min) low load

### Implementation Details
- **Load Average Parsing**: Modified to read 1-min, 5-min, and 15-min from `/proc/loadavg`
- **TLP Mode Selection**: Uses `setACMode()` and `setBatteryMode()` (not auto mode)
- **Dynamic Threshold**: Calculates absolute load threshold as 10% × core count
- **Enhanced Logging**: Shows 1-min, 5-min, and 15-min load averages with percentages
- **State Machine**: Intelligent switching logic prevents mode flapping
- **Conservative Approach**: Battery mode only when system is idle across multiple time scales

### Logging Format
```
[2025-07-03 20:45:30.123] [INFO] Initial state: 1-min load: 1.8 (22.5% avg per core), 5-min load: 0.9 (11.2% avg per core), 15-min load: 0.7 (8.8% avg per core)
[2025-07-03 20:45:30.124] [INFO] System active - switching to TLP AC mode
[2025-07-03 20:45:30.125] [INFO] Activity monitor started (1-min > 10% = AC mode, both 5-min AND 15-min <= 10% = battery mode)
[2025-07-03 20:46:00.456] [INFO] System became idle (5-min load: 1.1 = 13.7% avg per core <= 10% AND 15-min load: 0.8 = 10.0% avg per core <= 10%) - switching to TLP battery mode
```

## Recent Major Refactoring (Latest)

### Ultra-Simplification Complete ✅
- **Removed ALL Configuration**: Eliminated all config file support, parsing, and related code
- **Load Average Monitoring**: Switched from CPU usage to 1-minute load average from `/proc/loadavg`
- **Hardcoded Defaults**: All settings now hardcoded for maximum simplicity and reliability
- **Minimal Resource Usage**: System checks load average only once per minute (60-second intervals)
- **Config-Free Design**: No configuration files, no command-line parameters except --daemon, --help, --version

### Implementation Details
- **Load Threshold**: Hardcoded to 0.10 (10% of one CPU core)
- **Idle Timeout**: Hardcoded to 300 seconds (5 minutes)
- **Check Interval**: Hardcoded to 60 seconds (1 minute)
- **Log File**: Hardcoded to `/var/log/ddotlp.log`
- **No Sampling**: Direct load average reading with no averaging or circular buffers

### Terminology Corrections ✅
- **TLP Auto Mode**: `tlp start` - correctly referred to as "auto mode" (not "performance mode")
- **TLP Battery Mode**: `tlp bat` - correctly referred to as "battery mode"
- **Accurate Logging**: All log messages now reflect TLP's actual mode names
- **Documentation**: README and help output updated to reflect correct TLP terminology

### Code Cleanup ✅
- **Function Naming**: Renamed `setPerformanceMode()` to `setAutoMode()` for accurate terminology
- **Mode Tracking**: Internal mode state uses "auto" and "battery" to match TLP's actual modes
- **Config Removal**: Deleted `config.cpp`, `config.h`, and entire `config/` directory
- **CMake Cleanup**: Removed all config file installation rules
- **Warning-Free Build**: All compiler warnings eliminated
- **Signal Handling**: Updated SIGHUP handling to reflect no-config design
- **Main Simplification**: Removed all config-related command-line options and parsing

### Final User-Facing Text Simplification ✅ (Latest Update)
- **Help Output**: Simplified help text to remove developer jargon like "Hardcoded settings"
- **Single Sentence Summary**: Help now shows "Automatically switches between TLP auto and battery modes based on system load"
- **README Cleanup**: Changed "hardcoded sensible defaults" to "works out of the box with sensible defaults"
- **Settings Section**: Renamed "Hardcoded Settings" to simply "Settings" in documentation
- **User-Friendly Language**: All user-facing text now uses clear, non-technical language

## Current Implementation Details

### Implementation Details
- **Load Average Source**: Reads 1-minute load average directly from `/proc/loadavg`
- **Core Count Detection**: Reads `/proc/cpuinfo` to count processor entries (no external commands)
- **Dynamic Threshold**: Calculates absolute load threshold as 10% × core count
- **Decision Logic**: Load average > (0.10 × cores) = active; load average ≤ (0.10 × cores) = idle
- **Check Frequency**: Every 60 seconds for minimal system impact
- **Immediate Response**: Mode switches immediately based on current load average (no idle timeout)
- **No External Dependencies**: Uses only `/proc` filesystem, completely self-contained

### Logging Format
```
[2025-01-15 14:23:45.123] [INFO] Detected 8 CPU core(s)
[2025-01-15 14:23:45.124] [INFO] Load threshold: 0.10 (10% per core)
[2025-01-15 14:23:45.125] [INFO] Absolute load threshold: 0.8 (for 8 cores)
[2025-01-15 14:23:45.126] [INFO] 1-minute load average: 1.8 (threshold: 0.8 = 10% of 8 cores)
[2025-01-15 14:23:45.127] [INFO] System became active (load: 1.8 = 22.5% avg per core) - switching to TLP auto mode
[2025-01-15 14:23:45.128] [INFO] Switching to auto mode (tlp start)
[2025-01-15 14:23:45.456] [INFO] TLP output: TLP started in AC mode (auto).
[2025-01-15 14:23:45.457] [INFO] Successfully switched to auto mode
```

### Settings
- `LOAD_THRESHOLD_PERCENT`: 0.10 (10% per CPU core)
- `CHECK_INTERVAL`: 60 seconds (1 minute)
- `LOG_FILE`: `/var/log/ddotlp.log`
- `CORE_COUNT`: Auto-detected using `nproc` command

## Project Status: COMPLETE ✅

### All Major Objectives Achieved ✅
- ✅ **Ultra-Simple Implementation**: Load average monitoring with hardcoded settings
- ✅ **Config-Free Design**: No configuration files or complex setup
- ✅ **Minimal Resource Usage**: 1-minute load checks for maximum efficiency
- ✅ **Correct TLP Terminology**: All references use proper "auto" and "battery" mode names
- ✅ **Warning-Free Build**: Clean compilation with no warnings or errors
- ✅ **Production Ready**: Fully tested and deployment-ready systemd service
- ✅ **Complete Documentation**: README and help output reflect simplified design
### Eliminated Complexity
- ❌ **No Configuration Files**: Completely removed config parsing, validation, and file management
- ❌ **No Command-Line Options**: Only --daemon, --help, --version supported
- ❌ **No CPU Sampling**: No averaging, circular buffers, or complex CPU calculations
- ❌ **No Dependencies**: Zero external libraries beyond standard C++ and Linux system calls

## Development Patterns Established

### Code Organization
- **Ultra-Minimal Design**: Only essential functionality with hardcoded defaults
- **Load Average Focus**: Single-purpose monitoring via `/proc/loadavg`
- **Direct TLP Integration**: Simple command execution with output capture
- **Clean Error Handling**: Comprehensive logging without configuration complexity

### Quality Practices
- **Hardcoded Reliability**: No configuration means no config-related failures
- **Minimal Resource Usage**: 60-second intervals minimize system impact
- **Warning-Free Compilation**: Clean code that builds without any warnings
- **Production Simplicity**: Easy deployment with zero configuration required

## Project Health - Ultra-Simplified and Complete ✅
- **Build Status**: ✅ Clean compilation with zero warnings
- **Dependencies**: ✅ Only standard C++ and Linux system calls
- **Configuration**: ✅ None required - hardcoded sensible defaults
- **Resource Usage**: ✅ Minimal - checks load average once per minute
- **Documentation**: ✅ Updated to reflect simplified design
- **Deployment**: ✅ Single binary with systemd service integration
- **Testing**: ✅ Verified load average monitoring and TLP integration

### Code Cleanup ✅ (Latest)
- **Removed Unused Method**: Deleted `setAutoMode()` from TLPManager (header and implementation)
- **Updated Comments**: Fixed header comment to reflect load monitoring (not CPU usage)
- **Optimized Main Loop**: Changed sleep from 1 second to 30 seconds for efficiency
- **Mode Consistency**: Updated getCurrentMode() to use "ac" instead of "auto" internally
- **File Cleanup**: Removed empty `test_parsing.cpp` and `main.cpp.backup` files
- **Clean Build**: All changes compile successfully with no warnings
