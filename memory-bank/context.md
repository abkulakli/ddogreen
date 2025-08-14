# DDOGreen - Project Context

## Project Overview
**ddogreen** - A production-ready sustainability-focused power management service for PCs and laptops that automatically optimizes power settings based on intelligent system load monitoring.

**Product of DDOSoft Solutions** - Apache 2.0 licensed, production-ready cross-platform application.

## Architecture
### Core Design
```
Application Layer (main.cpp, config.cpp, logger.cpp) - ZERO platform-specific code
├── Platform Abstraction Layer (Abstract Interfaces)
│   ├── IPowerManager (power control abstraction)
│   ├── ISystemMonitor (load monitoring abstraction)  
│   ├── IPlatformUtils (path resolution, privilege checking)
│   └── ISignalHandler (signal handling and graceful shutdown)
└── Platform Implementations (Platform-Specific Code)
    ├── Linux (TLP, /proc/loadavg, systemd, Unix signals)
    ├── Windows (Power Plans, Performance Counters, SCM, Console handlers)
    └── macOS (pmset, system APIs, launchd, Unix signals)
```

### CRITICAL ARCHITECTURAL RULE: NO #ifdef IN COMMON CODE
- **FORBIDDEN**: `#ifdef` preprocessor directives in any common/application layer code
- **ALLOWED LOCATIONS**: Platform-specific implementation files ONLY (`src/platform/[platform]/`)
- **FACTORY EXCEPTION**: `platform_factory.cpp` may use `#ifdef` for compile-time platform selection
- **INTERFACE RULE**: Application code must use ONLY abstract base class interfaces

## Technology Stack
- **Language**: C++20 (Modern C++ features, standard library only)
- **Build System**: CMake 3.16+ with presets (debug/release/test/coverage)
- **Target OS**: Cross-platform with equal Linux and Windows support
- **Architecture**: Cross-platform with compile-time platform selection
- **Service Framework**: systemd (Linux), Windows Service Manager (Windows)
- **Testing**: GoogleTest framework (70 tests, 100% passing)
- **Dependencies**: None (only standard C++ and platform-specific system calls)

## Build System
```bash
# CMake presets workflow
cmake --preset debug                # Configure debug build
cmake --build --preset debug        # Build debug executable
cmake --preset release              # Configure release build  
cmake --build --preset release      # Build release executable

# Testing
cmake --build --preset debug --target test
ctest --preset debug-tests

# Coverage analysis
cmake --build build/coverage --target coverage         # Full HTML report
cmake --build build/coverage --target coverage-summary # Quick summary
```

## Project Status: PRODUCTION-READY
- **Complete Application**: All features implemented and working on Linux and Windows
- **Enhanced Windows Support**: Real Performance Counter implementation with dynamic load averaging
- **Configurable Monitoring**: User-configurable monitoring frequency (1-300 seconds) with proper scaling
- **Clean Architecture**: Zero platform-specific code in main application layer
- **Quality Assurance**: Clean build, all 70 unit tests passing, zero compiler warnings
- **Service Integration**: Full systemd (Linux) and SCM (Windows) service management
- **Cross-Platform Packaging**: DEB/RPM/TGZ (Linux), ZIP/MSI (Windows)

## Core Functionality
1. **Load Average Monitoring**: Cross-platform load monitoring (Linux: /proc/loadavg, Windows: Performance Counters)
2. **Power Management**: Platform-specific power control (Linux: TLP, Windows: powercfg)
3. **Service Management**: Platform-specific service control (Linux: systemd, Windows: SC)
4. **Configuration System**: Read-only configuration with comprehensive validation
5. **Cross-Platform Support**: Generic interfaces with platform-specific implementations

## Key Constraints
- Zero external dependencies (standard C++ only)
- Zero platform-specific code in application layer
- Energy efficiency as primary design principle
- Zero compiler warnings policy
- 100% test coverage maintenance
- Apache 2.0 licensing requirements
