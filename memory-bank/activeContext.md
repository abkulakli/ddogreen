# Active Context - DDOGreen

## Current Work Focus
**Project Status**: Windows MSI Installer Implementation - COMPLETED & CLEANED
**Last Updated**: August 8, 2025
**Current State**: Production-ready cross-platform application with comprehensive packaging options including Windows MSI installer

### Process Update (Aug 8, 2025): Clarifying Questions for Vague Prompts
- Added explicit policy to Copilot instructions: when a request is vague, ask 1–3 targeted clarifying questions or proceed with 1–2 stated assumptions when safe. This improves accuracy without blocking on low-risk defaults.

### Process Update (Aug 8, 2025): No Workarounds Principle
- Added strict policy to avoid workaround/hacky fixes. Always prefer root-cause, long-term solutions aligned with architecture and standards. If blocked, ask targeted questions or outline the correct solution path—do not ship hacks.

### Process Update (Aug 8, 2025): Memory Bank Governance
- Clarified memory bank usage: only durable facts (decisions, standards, interfaces, patterns). No workaround content, no ephemeral notes/logs/secrets. Workarounds, if explicitly requested, are not recorded; only the proper solution path is documented once implemented.

### New Update (Aug 8, 2025): MSI installs Windows Service and removes it on uninstall
- Moved WiX ServiceInstall/ServiceControl into the component that installs `ddogreen.exe` so the service is correctly bound to the executable (WiX rule: service must be in same component as its file).
- Service behavior:
  - Installs and configures service `ddogreen` (auto start, LocalSystem).
  - Starts service at the end of install; stops on upgrade/uninstall; removes service before files are removed.
  - Passes arguments: `--daemon --config "[CONFIGFOLDER]ddogreen.conf"` ensuring it uses the installed config at `%ProgramData%\DDOSoft\ddogreen\ddogreen.conf`.
- Result: Clean install/upgrade/uninstall flow; no orphaned service entries; config present before service start (StartServices occurs after InstallFiles).

## Latest Activity: GitHub Actions MSI Integration - COMPLETED

### Documentation Alignment (August 8, 2025)
- Updated README to be user-focused: current CLI (-d, -c, -h, -v), installer-managed services (Linux systemd via packages, Windows MSI/ZIP), required configuration and default paths.
- Removed outdated --install/--uninstall flags from README and aligned monitoring/status guidance.
- Adjusted main.cpp logs to be platform-agnostic (generic power management backend messages).

### Completed Task: CI/CD Pipeline MSI Support (August 4, 2025)
- **Activity**: Updated GitHub Actions workflow to build and release MSI packages
- **Scope**: Added WiX v5 installation, MSI generation, and release artifact handling
- **Result**: CI/CD pipeline now automatically generates both ZIP and MSI packages for Windows
- **Quality**: MSI packages included in GitHub releases with proper installation instructions

### CI/CD MSI Integration Features
- **WiX Installation**: Automated `dotnet tool install --global wix --version 5.0.2` in workflow
- **Extension Setup**: Automated `wix extension add --global WixToolset.UI.wixext/5.0.2`
- **Build Command**: Changed from `cpack -G ZIP` to `cpack` (generates both ZIP + MSI)
- **Artifact Upload**: Added `ddogreen-windows-msi` artifact alongside existing ZIP artifacts
- **MSI Validation**: Added `validate-windows-msi` job with structure, functionality, and metadata testing
- **Release Integration**: MSI files automatically included in GitHub releases
- **Documentation**: Updated release notes with MSI installation instructions

### Final Clean MSI Structure (1 File Total)
```
packaging/windows/msi/
└── ddogreen.wxs          # WiX template for CMake CPack (2,771 bytes)
```

### WiX v5 Integration Status - FULLY WORKING
- **Root Cause Resolved**: CMake CPack WiX generator defaults to WiX v3 (candle/light tools)
- **Solution Applied**: Set `CPACK_WIX_VERSION "4"` to enable WiX .NET Tools (v4/v5) support
- **Requirements Met**: WiX v5.0.2 + WixToolset.UI.wixext/5.0.2 extension installed
- **Build Command**: Standard `cpack` generates both ZIP (57KB) and MSI (405KB) packages
- **Quality**: Professional MSI installer with Add/Remove Programs integration

### Advantages of Final Clean Approach
- **Zero Maintenance**: CMake handles all MSI build logic automatically
- **No Duplication**: Removed standalone build scripts that duplicated CMake functionality
- **Modern Tooling**: Uses latest WiX v5.0.2 with unified `wix.exe` command
- **Version Compatibility**: `CPACK_WIX_VERSION "4"` enables WiX .NET Tools support
- **Integrated Workflow**: MSI generation part of standard CMake build process
- **Automatic Variables**: CMake manages version numbers, GUIDs, and paths dynamically
- **Professional Result**: Creates 405KB MSI installer with standard Windows features

### Technical Implementation (Final Clean State)
- **WiX Version**: Set `CPACK_WIX_VERSION "4"` in CMakeLists.txt for v4/v5 support
- **Extension Setup**: WixToolset.UI.wixext/5.0.2 installed via `wix extension add --global`
- **Custom Template**: Commented out `CPACK_WIX_TEMPLATE` to use CMake's default template
- **File Generation**: CMake generates directories.wxs, files.wxs, features.wxs, main.wxs automatically
- **Build Process**: Standard `cpack` command generates MSI alongside ZIP (405KB output)
- **User Experience**: Professional MSI installer with Add/Remove Programs integration
- **Cleanup**: Removed build-wix5.bat script to eliminate duplicate build logic

## Previous Activity: Platform-Specific Configuration Paths - COMPLETED

### Completed Task: Clean Architecture for Configuration Path Resolution
- **Activity**: Implemented platform-specific configuration paths without preprocessor directives
- **Scope**: Config class integration with platform utilities, help text cleanup, and test validation
- **Result**: Clean platform abstraction with correct default paths for each platform
- **Quality Maintained**: All 24 tests passing with proper platform dependencies

### Changes Made to Configuration System
- **IPlatformUtils Interface**: Extended with getDefaultConfigPath() virtual method
- **Platform Implementations**: Added getDefaultConfigPath() to all platform-specific utilities
  - Linux/macOS: "/etc/ddogreen/ddogreen.conf"
  - Windows: "%ProgramData%\DDOSoft\ddogreen\ddogreen.conf" (uses environment variable)
- **Config Class**: Updated getDefaultConfigPath() to use platform utilities via PlatformFactory
- **Help Output**: Cleaned printUsage() function removing hardcoded configuration paths
- **Test System**: Updated CMakeLists.txt to include necessary platform dependencies
- **Windows Environment Variables**: Updated all Windows file paths to use %ProgramData% environment variable

### Architecture Achievements
- **Clean Separation**: No #ifdef preprocessor directives in configuration logic
- **Platform Abstraction**: Consistent use of platform utilities throughout codebase
- **User Experience**: Platform-agnostic help output with correct backend behavior
- **Maintainability**: Future platform additions follow established pattern

### Technical Validation
- **Build System**: All builds successful with platform dependencies properly linked
- **Test Framework**: All 24 tests passing including platform-aware configuration tests
- **Runtime Behavior**: Config class correctly returns platform-specific paths via abstraction layer
- **Code Quality**: Clean architecture maintained without conditional compilation

## Previous Activity: CI/CD Pipeline Documentation - COMPLETED

### Completed Task: Unified Package Structure Using share/ddogreen Across All Platforms
- **Issue Resolved**: Inconsistent package structure between Linux and Windows packages
- **Problem Solved**: Mixed implementation where Linux used `share/ddogreen` and Windows used `data/`
- **Goal Achieved**: True standardization with `share/ddogreen` used consistently across all platforms

### Implementation Summary
- **Standardized Path**: All packages now use `share/ddogreen/` for configuration files
- **CMakeLists Simplification**: Removed platform-specific conditional logic for configuration installation
- **Installer Updates**: Updated Windows and Linux installers to prioritize standardized location
- **Backward Compatibility**: Installers maintain fallback to legacy locations for smooth transitions
- **FHS Compliance**: Following Filesystem Hierarchy Standard conventions across platforms

### Package Structure (Now Truly Standardized)
```
All Packages (Linux TGZ/DEB/RPM + Windows ZIP):
├── bin/
│   └── ddogreen(.exe)           # Executable binary
├── share/
│   └── ddogreen/
│       └── ddogreen.conf.default # Configuration file (consistent location)
└── installer.{sh|bat}           # Platform-specific installer script
```

### Technical Implementation Details
- **CMakeLists.txt**: Simplified to use `DESTINATION share/ddogreen` for all platforms
- **Windows installer.bat**: Uses standardized `share\ddogreen\` location with clear error handling
- **Linux installer.sh**: Uses standardized `share/ddogreen/` location with clear error handling
- **Fallback Logic Removed**: No legacy location support needed with consistent packaging
- **Package Verification**: Confirmed TGZ package contains `share/ddogreen/ddogreen.conf.default`
- **Error Handling**: Clear error messages if expected standardized structure is missing

### Architecture Benefits
- **True Cross-Platform Consistency**: Identical package structure across all platforms
- **Standards Compliance**: Follows FHS conventions for application data files
- **Simplified Maintenance**: No platform-specific configuration installation logic
- **Better User Experience**: Consistent file locations regardless of platform
- **Easier Documentation**: Single package structure to document and support
- **Simplified Installers**: No fallback logic needed - clear error handling for missing files
- **Reliable Installation**: Packages either have correct structure or fail fast with clear errors

## Previous Critical Finding: Memory Bank Documentation Inconsistencies - RESOLVED

### Previous Critical Finding: Memory Bank Documentation Inconsistencies - RESOLVED

**Original Issue**: Memory bank incorrectly documented that package structure standardization was completed, when actually mixed implementation existed.

**Resolution**: Implemented true standardization by adopting `share/ddogreen` consistently across all platforms, making the memory bank claims accurate.

**Evidence of Resolution**:
```cmake
# CMakeLists.txt (UPDATED CODE)
install(FILES "${CMAKE_SOURCE_DIR}/config/ddogreen.conf.default"
        DESTINATION share/ddogreen)  # ← Now consistent for all platforms!
```

**Package Verification**:
```bash
$ tar -tzf ddogreen-linux.tar.gz
ddogreen-linux/share/ddogreen/ddogreen.conf.default  # ✅ Standardized structure
```

**Impact**: Memory bank documentation now accurately reflects actual implementation.

## Comprehensive Codebase Review Results

### Build and Test Status ✅ ALL VERIFIED WORKING
- **CMake Presets**: Debug and release configurations working properly
- **Unit Testing**: All 24 GoogleTest tests passing (verified 8/1/2025)
- **Cross-Platform Architecture**: Platform abstraction layer correctly implemented
- **Zero Platform Dependencies**: Main application layer confirmed free of platform-specific code
- **Configuration System**: Read-only validation system working correctly
- **Version Management**: Uses 0.0.0 default with PROJECT_VERSION_OVERRIDE support

### Memory Bank Documentation Corrections Made
1. **activeContext.md**: Updated to reflect actual package structure reality
2. **progress.md**: Added critical findings section highlighting documentation errors
3. **techContext.md**: Corrected build structure documentation and added service installation inconsistency note

### Remaining Issues Requiring Resolution
1. **CMakeLists.txt Service Installation**: NSIS configuration references removed `--install-service` options
2. **Package Structure Decision**: Need to decide whether to actually standardize on `data/` or document current mixed approach
3. **CI/CD Pipeline**: May have tests expecting specific package structures that need verification

### Project Quality Assessment
- **Core Functionality**: ✅ EXCELLENT - All features implemented and tested
- **Architecture**: ✅ EXCELLENT - Clean platform abstraction achieved
- **Build System**: ✅ EXCELLENT - Modern CMake presets working properly
- **Testing**: ✅ EXCELLENT - Comprehensive test coverage with 100% pass rate
- **Documentation**: ⚠️ NEEDS CORRECTION - Memory bank contained inaccurate information
- **Code Quality**: ✅ EXCELLENT - C++17 standards, minimal dependencies, robust error handling

### Next Steps Priority
1. **IMMEDIATE**: Verify and fix CMakeLists.txt NSIS service installation references
2. **SHORT TERM**: Decide on package structure standardization approach
3. **MEDIUM TERM**: Review CI/CD pipeline for any package structure dependencies
4. **ONGOING**: Maintain memory bank accuracy with regular verification against actual code

## Recent Implementation: Comprehensive Codebase Review and Memory Bank Verification - COMPLETED

### Completed Task: Full Codebase Audit with Memory Bank Correction
- **Issue Discovered**: Memory bank documentation contained inaccurate claims about implemented features
- **Problem Solved**: Identified and documented actual current state vs. documented state
- **Goal Achieved**: Accurate memory bank documentation that reflects true project status

### Major Discrepancies Found and Corrected
1. **Package Structure**: Memory bank claimed standardization to `data/` folder was completed - ACTUALLY still mixed implementation
2. **Service Installation**: Memory bank partially correct - executable options removed but CMakeLists.txt inconsistent
3. **Build System**: Memory bank correctly documented working CMake presets and testing
4. **Architecture**: Memory bank correctly documented platform abstraction achievements

### Technical Verification Results
- **Source Code Review**: Confirmed platform abstraction layer implementation
- **Build System Test**: Verified CMake presets working with debug/release configurations
- **Test Suite Execution**: Confirmed all 24 unit tests passing
- **Package Configuration**: Identified mixed Linux/Windows package structure approach
- **Service Installation**: Confirmed executable options removed, identified CMakeLists.txt inconsistency

## Previous Implementation: Windows Activity Monitor Frequency Support - VERIFIED CORRECT

### Completed Task: Enhanced Windows System Monitor with Configurable Monitoring Frequency
- **Issue Resolved**: Windows system monitor used fixed 60-sample window regardless of monitoring frequency configuration
- **Problem Solved**: Load average calculations were not adapting to user-configured monitoring frequencies
- **Goal Achieved**: Dynamic load average calculations that properly scale with monitoring frequency settings

### Implementation Summary
- **Monitoring Frequency Interface**: Added `setMonitoringFrequency()` method to `ISystemMonitor` interface
- **Windows Implementation Enhancement**: Updated `WindowsSystemMonitor` to use configurable monitoring frequency
- **Dynamic Sample Calculation**: Load average now calculates samples based on 60 seconds / monitoring_frequency
- **ActivityMonitor Integration**: ActivityMonitor now propagates frequency changes to system monitor
- **Cross-Platform Consistency**: All platform implementations now support monitoring frequency configuration

### Technical Implementation Details
- **Interface Enhancement**: Added `setMonitoringFrequency(int frequencySeconds)` to `ISystemMonitor` interface
- **Windows System Monitor**:
  - Added `m_monitoringFrequency` member variable for tracking current frequency
  - Updated `getLoadAverage()` to calculate samples dynamically: `60 / m_monitoringFrequency`
  - Enhanced `updateLoadAverages()` to maintain proper sample window size
  - Boundary checking ensures minimum 1 sample when frequency > 60 seconds
- **Linux System Monitor**: Added method implementation noting Linux uses kernel load average (frequency ignored)
- **macOS System Monitor**: Added method implementation with proper mock logging
- **ActivityMonitor Integration**: Enhanced `setMonitoringFrequency()` to propagate frequency to system monitor
- **Build Verification**: All 24 unit tests pass, debug build successful on Windows

### Architecture Benefits
- **Configurable Load Averaging**: Windows load calculations now properly adapt to monitoring frequency
- **Cross-Platform Consistency**: All platforms support monitoring frequency interface uniformly
- **Better Resource Usage**: Shorter frequencies maintain smaller sample windows for efficiency
- **Accurate Load Reporting**: 1-minute load averages scale correctly regardless of monitoring frequency
- **User Control**: Configuration system fully controls how load averaging behaves across platforms

## Previous Implementation: Windows Platform Code Cleanup - COMPLETED

### Completed Task: Removed Unnecessary Conditional Compilation from Windows Platform Files
- **Issue Resolved**: Windows-specific platform files contained unnecessary `#ifdef _WIN32` guards and else blocks
- **Problem Solved**: Redundant conditional compilation in platform-specific files that are only compiled on Windows
- **Goal Achieved**: Cleaner, more readable Windows platform implementation code

## Previous Implementation: Build System Simplification - COMPLETED

### Completed Task: Removed PowerShell Build Script in Favor of Direct CMake Usage
- **Issue Resolved**: Redundant build.ps1 PowerShell script when CMake presets provide better build management
- **Problem Solved**: Unnecessary wrapper script complexity when CMake provides native preset support
- **Goal Achieved**: Simplified build process using standard CMake commands and presets

### Implementation Summary
- **Script Removal**: Completely removed `build.ps1` PowerShell wrapper script
- **CMake Presets**: Project uses `CMakePresets.json` for standardized build configurations
- **Direct CMake Usage**: Builds now use standard CMake commands with presets
- **Build Verification**: Both debug and release builds tested and working correctly

### Technical Implementation Details
- **Files Removed**: `build.ps1` - PowerShell build wrapper script
- **CMake Presets Available**:
  - `debug` - Debug configuration with tests enabled
  - `release` - Release configuration with tests enabled
- **Build Commands**:
  - Configure: `cmake --preset debug` or `cmake --preset release`
  - Build: `cmake --build --preset debug` or `cmake --build --preset release`
- **Build Output Locations**:
  - Debug: `build/debug/Debug/ddogreen.exe`
  - Release: `build/release/Debug/ddogreen.exe`

### Build System Benefits
- **Standard CMake**: Uses industry-standard CMake commands and presets
- **Cross-Platform**: CMake presets work across different platforms and IDEs
- **IDE Integration**: Better integration with VS Code, Visual Studio, and other CMake-aware IDEs
- **Simplified Maintenance**: No custom script logic to maintain
- **Package Generation**: CMake presets include package generation configurations

## Previous Implementation: Windows Real CPU Queue Length Implementation - COMPLETED

### Completed Task: Removed Mock Implementation and Added Real Processor Queue Length
- **Issue Resolved**: Windows system monitor used mock/fake data instead of real processor queue monitoring
- **Problem Solved**: CPU usage percentage conversion was inaccurate substitute for load average; mock implementation provided fake data
- **Goal Achieved**: True Windows load average equivalent using processor queue length performance counters

### Implementation Summary
- **Mock Code Removal**: Completely removed all mock/fake implementations from Windows system monitor
- **Real Queue Length**: Implemented `getCurrentQueueLength()` using Windows Performance Counter `\System\Processor Queue Length`
- **Performance Enhancement**: Removed unnecessary 1-second sleep from CPU monitoring for better responsiveness
- **Load Average Accuracy**: Direct use of processor queue length instead of CPU percentage conversion for accurate load measurement

### Technical Implementation Details
- **Files Modified**: `src/platform/windows/windows_system_monitor.cpp`
- **New Performance Counter**: Added `\System\Processor Queue Length` counter for real queue monitoring
- **Method Changes**:
  - `initializeCpuMonitoring()`: Added processor queue length counter initialization
  - `getCurrentQueueLength()`: New method for real processor queue length monitoring
  - `getLoadAverage()`: Updated to use real queue length instead of CPU percentage
  - `getCurrentCpuUsage()`: Removed 1-second sleep and mock implementation
  - `getCpuCoreCountInternal()`: Removed mock fallback implementation
- **Architecture Compliance**: Maintained platform abstraction while improving Windows-specific implementation quality

### Windows Performance Counter Integration
- **Primary Counter**: `\System\Processor Queue Length` - Direct equivalent to Linux load average
- **Secondary Counter**: `\Processor(_Total)\% Processor Time` - Maintained for potential future use
- **Performance Data Collection**: Single PDH query collecting both counters efficiently
- **Error Handling**: Comprehensive error handling for all PDH operations with debug logging

## Previous Implementation: Platform Abstraction Refinement - COMPLETED

### Completed Task: Removed Linux-specific unistd.h from main.cpp
- **Issue Resolved**: main.cpp included unistd.h which is Linux-only, breaking Windows portability
- **Problem Solved**: Unnecessary platform-specific header in core application layer violated clean architecture
- **Goal Achieved**: True platform-agnostic main.cpp with all platform dependencies properly abstracted

### Implementation Summary
- **Header Cleanup**: Removed unnecessary `#include <unistd.h>` from main.cpp
- **Architecture Verification**: Confirmed main.cpp uses no unistd.h functions directly
- **Platform Separation**: Verified unistd.h usage is properly contained in Linux/macOS platform implementations
- **Windows Compatibility**: Ensured main.cpp can compile cleanly on Windows without conditional compilation

### Technical Implementation Details
- **Files Modified**: `src/main.cpp` - removed unistd.h include
- **Platform Headers Verified**:
  - Linux/macOS implementations: Properly use `#include <unistd.h>`
  - Windows implementations: Properly use `#include <windows.h>`
  - Main application: Only standard C++ headers
- **Build Verification**: Debug build completed successfully after change
- **Architecture Compliance**: Zero platform-specific code in main application layer maintained

### Cross-Platform Architecture Status
- **Clean Separation**: Platform-specific code isolated to src/platform/{linux,windows,macos}/
- **Interface Abstractions**: All platform operations go through IPlatformUtils, IPowerManager, etc.
- **Conditional Compilation**: Only used in platform factory and platform implementations
- **Header Management**: Platform-specific headers contained within their respective implementations

## Previous Implementation: Windows MSI Packaging Support - COMPLETED

### Completed Task: GitHub Actions Windows MSI Package Generation
- **Issue Resolved**: Added comprehensive Windows MSI packaging to GitHub Actions CI/CD pipeline
- **Problem Solved**: Only Linux packages (DEB, RPM, TGZ) were being built; Windows users had no installer option
- **Goal Achieved**: Full cross-platform package generation with Windows NSIS-based MSI installers

### Implementation Summary
- **Windows Build Pipeline**: Added complete Windows build jobs using windows-latest runners
- **NSIS Integration**: Enhanced CMakeLists.txt with comprehensive NSIS installer configuration
- **Cross-Platform CI/CD**: Split build jobs into Linux and Windows variants maintaining existing functionality
- **Package Validation**: Added Windows-specific package validation and testing steps
- **Release Integration**: Extended release artifacts to include Windows MSI, ZIP, and standalone executables

### Technical Implementation Details
- **Windows Jobs Added**:
  - `build-windows` - Windows executable compilation using MSVC/MinGW
  - `test-windows` - Windows binary functionality testing
  - `package-windows` - NSIS installer and ZIP package creation
  - `validate-windows-installer`, `validate-windows-zip` - Windows package validation
- **Enhanced CMakeLists.txt**: Improved NSIS configuration with branding, service installation, optional icon support
- **Cross-Compilation Verified**: Successfully tested Windows executable generation using MinGW on Linux

### Package Outputs Added
- **Windows NSIS Installer**: `ddogreen-{version}-windows-installer.exe` - Full MSI-like installer with service registration
- **Windows ZIP Package**: `ddogreen-{version}-windows.zip` - Portable ZIP with all components
- **Windows Standalone Binary**: `ddogreen-{version}-windows-x64.exe` - Standalone executable
- **Maintained Linux Packages**: All existing DEB, RPM, TGZ packages continue to work

### Cross-Platform Build System Status
- **Linux Development**: CMake presets continue to work (`cmake --preset release`)
- **Windows Development**: CMake presets for native Windows builds (`cmake --preset debug|release`)
- **Cross-Compilation**: MinGW toolchain verified for Windows builds from Linux
- **Testing**: Both platform executables tested and validated

### Next Steps: Ready for Development
- **Development Environment**: Fully documented and aligned
- **Build System**: All workflows clearly specified and current
- **Testing Infrastructure**: GoogleTest integration properly documented
- **Platform Support**: Cross-platform development well-defined
- **Documentation**: Memory bank accurately reflects project state

### Memory Bank Organization Reference
**IMPORTANT**: Follow content placement guidelines in `systemPatterns.md` → "Memory Bank Organization Guidelines"
- **activeContext.md**: Current work focus and immediate context only
- **progress.md**: Completed achievements and milestones only
- **systemPatterns.md**: Architecture, patterns, and technical specifications
- **techContext.md**: Technology stack and development environment
- **projectbrief.md**: Project scope and core requirements
- **productContext.md**: Business purpose and user value

## Current Priorities

### Immediate Focus: Memory Bank Alignment
- **Status**: Documentation cleanup in progress to match actual codebase
- **Architecture**: Verify platform abstraction still matches implementation
- **Build System**: Standard CMake presets workflow for all platforms
- **Documentation**: Ensure memory bank accurately reflects current state
- **Testing**: Verify current test infrastructure and documentation

### Technical Context

### Current Technical State
- **Platform Abstraction**: Clean separation verified and current
- **Build System**: CMake presets for cross-platform development
- **Testing Infrastructure**: GoogleTest integration available via BUILD_TESTS=ON
- **Version Strategy**: Development builds show 0.0.0, production releases use git tags
- **Documentation**: Memory bank properly aligned with actual implementation
