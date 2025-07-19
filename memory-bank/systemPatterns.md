# System Patterns - ddogreen

## Architecture Overview

### High-Level Design
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   ActivityMonitor   │────│   Main Daemon    │────│  PowerManager   │
│  (CPU Monitoring)   │    │  (Coordination)  │    │ (Power Control) │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                        │                        │
         │                        │                        │
    ┌─────────┐              ┌─────────┐              ┌─────────┐
    │/proc/stat│              │ Logger  │              │   TLP   │
    │         │              │         │              │Commands │
    └─────────┘              └─────────┘              └─────────┘
```

### Cross-Platform Architecture (Refactored - July 2025)
```
┌─────────────────────────────────────────┐
│           Application Layer             │
│        (main.cpp - ZERO ifdefs)         │
│     Pure platform-agnostic logic       │
└─────────────────────────────────────────┘
                     │
┌─────────────────────────────────────────┐
│         Platform Abstraction            │
│  IPowerManager | IServiceManager |      │
│  ISystemMonitor | IPlatformUtils |      │
│         PlatformFactory                 │
└─────────────────────────────────────────┘
                     │
         Compile-time Selection
┌──────────────────┬──────────────────────┐
│   Linux Platform │   Windows Platform   │
│   (when building │   (when building     │
│    on Linux)     │    on Windows)       │
└──────────────────┴──────────────────────┘
```

### Component Responsibilities

#### main.cpp (Refactored - Clean Architecture)
- **Purpose**: Pure platform-agnostic orchestration and coordination
- **Zero ifdefs**: No platform-specific code whatsoever
- **No platform includes**: Only standard C++ and platform abstraction headers
- **Platform delegation**: All platform-specific logic delegated to platform layer
- **Clean separation**: Business logic separated from platform concerns

#### IPlatformUtils (New Interface)
- **Purpose**: Platform-specific utility functions abstraction
- **Privilege checking**: `hasRequiredPrivileges()` - replaces ifdef privilege checks
- **Executable path**: `getExecutablePath()` - replaces ifdef path detection
- **Command line parsing**: `parseCommandLine()` - unified across platforms
- **Default paths**: Platform-specific installation, log, and PID file paths
- **Error messages**: Platform-appropriate privilege escalation messages

#### ActivityMonitor
- **Purpose**: Monitor system activity through CPU usage
- **Data Source**: `/proc/stat` filesystem
- **Decision Logic**: CPU threshold-based activity detection
- **Callback Pattern**: Notifies main daemon of state changes
- **Threading**: Runs monitoring loop in separate thread

#### PowerManager (Platform Abstraction)
- **Purpose**: Cross-platform power management interface
- **Implementation**: Platform-specific power control (Linux: TLP, Windows: powercfg)
- **Methods**: `setPerformanceMode()`, `setPowerSavingMode()`, `getCurrentMode()`
- **Abstraction**: Hides platform-specific details from application code
- **Command Execution**: Executes `tlp start` and `tlp bat` commands
- **Output Capture**: Captures and processes TLP command output
- **State Tracking**: Maintains current power mode state
- **Error Handling**: Robust command failure handling

#### ServiceManager (Platform Abstraction)
- **Purpose**: Cross-platform service management
- **Responsibility**: Abstract service installation, uninstallation, and lifecycle management
- **Interface**: Provides uniform service operations across platforms
- **Implementation**: Platform-specific service management (systemd, SCM, launchd)
- **Encapsulation**: Hides platform-specific service details from application code

#### Platform Abstraction Layer
- **Purpose**: Provides cross-platform support for all platform-specific operations
- **Generic Interfaces**: IPowerManager, ISystemMonitor, IServiceManager, IPlatformUtils
- **Platform Implementations**: Platform-specific code in dedicated directories
- **Compile-Time Selection**: Only target platform code included in binaries
- **Zero Runtime Overhead**: All platform decisions made at compile time
- **Complete Abstraction**: Application layer has no platform knowledge

#### Logger
- **Purpose**: Centralized logging with precise timestamps
- **Format**: `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message`
- **Outputs**: File logging with stderr for errors/warnings
- **Rotation**: Integrated with system logrotate
- **Thread Safety**: Safe for concurrent access

#### Daemon
- **Purpose**: System service lifecycle management
- **Daemonization**: Proper double-fork daemon process
- **Signal Handling**: Graceful shutdown on SIGTERM, SIGINT
- **PID Management**: Creates and manages PID file
- **Security**: Drops unnecessary privileges where possible

## Key Design Patterns

### 1. Platform Abstraction Pattern (Enhanced - July 2025)
```cpp
// WRONG - Never do this in main.cpp:
#ifdef _WIN32
    // Windows-specific code
#else
    // Linux-specific code
#endif

// CORRECT - Always use platform abstraction:
auto platformUtils = PlatformFactory::createPlatformUtils();
if (!platformUtils->hasRequiredPrivileges()) {
    std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
    return 1;
}
```

**Critical Rule**: **NEVER use ifdefs in main.cpp or application layer**
**Rationale**: Complete separation of platform concerns from business logic
**Benefits**:
- Clean, maintainable code
- Platform-specific logic properly encapsulated
- Easy to add new platforms without touching main.cpp
- Testable platform implementations

### 2. Platform Delegation Pattern
```cpp
// Application layer delegates platform concerns to abstractions:
class IPlatformUtils {
public:
    virtual bool hasRequiredPrivileges() const = 0;
    virtual std::string getExecutablePath() const = 0;
    virtual ParsedArgs parseCommandLine(int argc, char* argv[]) const = 0;
    virtual std::string getDefaultInstallPath() const = 0;
    virtual std::string getPrivilegeEscalationMessage() const = 0;
};

// Abstractions encapsulate all platform complexity:
class IServiceManager {
public:
    virtual bool installService(const std::string& serviceName,
                               const std::string& executablePath,
                               const std::string& description) = 0;
    // Platform implementation handles all required operations internally
};
```

**Pattern**: Delegate all platform-specific operations to platform abstractions
**Benefits**:
- Application layer remains platform-agnostic
- Platform complexity encapsulated in dedicated components
- Easy to extend and maintain

### 3. Observer Pattern (Activity Monitoring)
```cpp
// ActivityMonitor notifies daemon of state changes
activityMonitor.setActivityCallback([&powerManager](bool isActive) {
    if (isActive) {
        powerManager->setPerformanceMode();     // High performance when active
    } else {
        powerManager->setPowerSavingMode();     // Power saving when idle
    }
});
```

**Rationale**: Decouples activity detection from power management actions
**Benefits**: Easy to extend with additional observers, clean separation of concerns

### 4. Command Pattern (TLP Execution)
```cpp
// TLPManager encapsulates command execution
bool TLPManager::setPerformanceMode() {
    std::string output = executeCommandWithOutput("sudo tlp start 2>&1");
    // Process output and handle results
}
```

**Rationale**: Encapsulates TLP command execution with proper error handling
**Benefits**: Consistent command execution, output capture, and error handling

### 5. Factory Pattern (Platform Creation)
```cpp
// Platform-agnostic creation
auto powerManager = PlatformFactory::createPowerManager();
auto systemMonitor = PlatformFactory::createSystemMonitor();
auto platformUtils = PlatformFactory::createPlatformUtils();

// Compile-time selection ensures only target platform code is included
```

**Rationale**: Single creation point for platform objects with compile-time optimization
**Benefits**: Clean object creation, compile-time platform selection, no unused code

### 6. State Machine (Power Mode Management)
```
[Unknown] → [Performance] ⟷ [Battery]
     ↓            ↑              ↓
  TLP Check   CPU Active    CPU Idle (5min)
```

**Rationale**: Clear state transitions based on activity detection
**Benefits**: Prevents unnecessary mode switches, clear behavior definition

### 7. Strategy Pattern (Activity Detection)
Current implementation uses CPU monitoring, but architecture supports different strategies:
```cpp
class ActivityMonitor {
    virtual bool isSystemActive() = 0;  // Could be CPU, network, disk, etc.
};
```

**Rationale**: Future extensibility for different activity detection methods
**Benefits**: Easy to switch or combine detection strategies

## Implementation Details (July 2025 Refactoring)

### Platform Abstraction Implementation
- **IPlatformUtils Interface**: Abstracts privilege checking, executable path detection, command line parsing
- **Enhanced Service Managers**: Handle file operations (copying, directory creation, permissions) internally
- **Complete Platform Delegation**: All platform-specific operations moved from main.cpp to platform layer
- **File Organization**: Platform-specific code isolated in `src/platform/[platform]/` directories
- **Zero Application Layer Coupling**: main.cpp contains no platform-specific includes or logic

### Service Installation Enhancement
- **Linux Implementation**: Service manager handles executable installation to `/usr/local/bin/`
- **File Operations**: Platform implementations manage directory creation, file copying, permission setting
- **Path Management**: Platform-specific paths encapsulated in platform utilities
- **Cleanup Operations**: Platform implementations handle complete uninstallation cleanup

### Command Line Processing
- **Unified Interface**: `parseCommandLine()` method provides consistent parsing across platforms
- **Platform-Specific Parsing**: Linux uses `getopt_long`, Windows uses simple string parsing
- **Return Structure**: `ParsedArgs` struct provides platform-agnostic argument representation

## Critical Implementation Decisions

### 1. Platform Abstraction Architecture (July 2025 Refactoring)
**Decision**: Complete separation of platform concerns from application logic
**Rationale**:
- Clean architecture principles
- Platform concerns should not leak into business logic
- Maintainable and testable code
- Easy to add new platforms

**Implementation Strategy**: Created comprehensive platform abstraction layer with complete delegation

### 2. Interface-Based Platform Operations (July 2025 Enhancement)
**Decision**: All platform operations must go through abstract interfaces
**Rationale**:
- Application layer should be completely platform-agnostic
- Platform layer should be complete and self-contained
- Enables independent testing of platform implementations

**Implementation Strategy**: Enhanced existing abstractions and added IPlatformUtils interface

### 3. CPU-Based Activity Detection
**Decision**: Use `/proc/stat` CPU usage instead of X11 events
**Rationale**:
- Universal compatibility (works without GUI)
- More representative of actual system activity
- No external dependencies
- Works on servers and containers

**Trade-offs**:
- May not detect user presence without CPU activity
- Requires tuning of CPU threshold for different systems

### 2. Polling vs Event-Driven Monitoring
**Decision**: 5-second polling interval for CPU checks
**Rationale**:
- Balance between responsiveness and system overhead
- CPU usage calculation requires time intervals
- Simpler implementation than complex event systems

**Trade-offs**:
- Not instantaneous response to activity changes
- Constant low-level CPU usage for monitoring

### 3. Synchronous TLP Command Execution
**Decision**: Block on TLP command completion
**Rationale**:
- TLP commands are fast (typically <1 second)
- Need to capture output for logging
- Ensures mode changes complete before continuing

**Trade-offs**:
- Brief blocking during mode switches
- Simpler error handling and state management

### 4. Millisecond Timestamp Precision
**Decision**: Include milliseconds in all log timestamps
**Rationale**:
- Better correlation with system events
- Improved debugging capability
- Minimal performance overhead

**Benefits**:
- Clear sequence of rapid events
- Better troubleshooting of timing issues

## Error Handling Patterns

### 1. Graceful Degradation
```cpp
if (!tlpManager.isTLPAvailable()) {
    Logger::warning("TLP not available, continuing without power management");
    // Continue running for monitoring/logging
}
```

### 2. Retry Logic
```cpp
// TLP commands may fail temporarily
if (!setPerformanceMode()) {
    Logger::warning("TLP command failed, will retry on next activity");
    // Don't exit, just log and continue
}
```

### 3. Resource Cleanup
```cpp
// Proper cleanup in destructors and signal handlers
~ActivityMonitor() {
    stop();  // Ensure monitoring thread stops
}
```

## Performance Considerations

### 1. CPU Monitoring Efficiency
- 5-second intervals to minimize overhead
- Simple calculation avoiding complex parsing
- Minimal memory allocation in monitoring loop

### 2. Logging Performance
- Buffered file I/O for log writes
- Avoid logging in tight loops
- Debug level logging only when needed

### 3. Thread Management
- Single monitoring thread, not per-check threads
- Detached thread for fire-and-forget monitoring

## Architectural Rules (Critical - July 2025)

### 🚫 NEVER Do These (Anti-Patterns):

#### 1. Platform-Specific Code in Application Layer
```cpp
// ❌ NEVER - Platform ifdefs in main.cpp or application layer
#ifdef _WIN32
    // Windows-specific code
#else
    // Linux-specific code
#endif

// ❌ NEVER - Platform-specific includes in main.cpp
#include <windows.h>
#include <unistd.h>
```

#### 2. Direct Platform Operations in Application Layer
```cpp
// ❌ NEVER - Direct platform calls in main.cpp
if (geteuid() != 0) { /* Linux-specific */ }
if (!CheckTokenMembership(...)) { /* Windows-specific */ }
system("cp file dest"); // Platform-specific commands
```

#### 3. Hard-Coded Platform Paths in Application Layer
```cpp
// ❌ NEVER - Hard-coded paths in main.cpp
std::string logPath = "/var/log/ddogreen.log";  // Linux-specific
std::string installPath = "/usr/local/bin/ddogreen";  // Linux-specific
```

### ✅ ALWAYS Do These (Correct Patterns):

#### 1. Platform Abstraction for ALL Platform Operations
```cpp
// ✅ ALWAYS - Use platform abstraction
auto platformUtils = PlatformFactory::createPlatformUtils();
if (!platformUtils->hasRequiredPrivileges()) {
    std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
}
```

#### 2. Platform Delegation for Complex Operations
```cpp
// ✅ ALWAYS - Platform implementations handle complexity
auto serviceManager = PlatformFactory::createServiceManager();
serviceManager->installService(name, path, description);
// Platform implementation handles: file copying, directories, permissions, etc.
```

#### 3. Platform-Agnostic Application Logic
```cpp
// ✅ ALWAYS - main.cpp knows nothing about platforms
ParsedArgs args = platformUtils->parseCommandLine(argc, argv);
std::string logPath = platformUtils->getDefaultLogPath();
std::string pidPath = platformUtils->getDefaultPidPath();
```

### 🔒 Architectural Enforcement Rules:

#### 1. File Organization Rules
- **main.cpp**: ZERO platform-specific code, includes, or ifdefs
- **include/platform/**: Only interface headers, no implementation
- **src/platform/[platform]/**: ALL platform-specific implementations
- **Platform factory**: Only compile-time platform selection allowed

#### 2. Dependency Rules
- **Application layer**: Only depends on platform interfaces
- **Platform layer**: Can depend on platform-specific libraries
- **No circular dependencies**: Platform implementations cannot depend on application layer

#### 3. Interface Rules
- **Complete abstraction**: Platform interfaces must handle ALL platform operations
- **No leakage**: Platform-specific types or constants must not appear in interfaces
- **Self-contained**: Platform implementations must be complete and self-sufficient

### 🧪 Testing Rules:

#### 1. Platform Independence Testing
- **main.cpp functionality**: Must be testable without platform-specific code
- **Platform implementations**: Must be unit testable independently
- **Interface compliance**: All platform implementations must pass interface tests

#### 2. Build Verification
- **No cross-platform code**: Linux builds must contain zero Windows code
- **Clean compilation**: No platform-specific warnings or errors
- **Consistent behavior**: Same command-line interface across all platforms

### 📋 Code Review Checklist:

#### Before Adding ANY Platform-Specific Code:
1. ❓ Is this platform-specific logic in the application layer? → **Move to platform layer**
2. ❓ Are there ifdefs in main.cpp or application files? → **Remove and use platform abstraction**
3. ❓ Are platform-specific includes used in application layer? → **Move to platform implementations**
4. ❓ Are hard-coded paths or platform assumptions present? → **Use platform interfaces**
5. ❓ Can this operation be tested independently per platform? → **Ensure clean interfaces**

#### Key Questions:
- **"Could main.cpp compile and run on any platform?"** → Should be YES
- **"Does the application layer know what platform it's running on?"** → Should be NO
- **"Are platform implementations complete and self-contained?"** → Should be YES

This architectural discipline ensures maintainable, testable, and extensible cross-platform code.
- Clean shutdown coordination

## Security Patterns

### 1. Privilege Management
- Runs as root (required for TLP commands)
- No privilege escalation beyond initial requirements
- Minimal file system access beyond required paths

### 2. Input Validation
- Command-line argument validation
- Configuration file parsing with bounds checking
- Safe string handling throughout

### 3. Process Isolation
- Systemd security features enabled
- Private temporary directories
- Protected system directories

## Extension Points

### 1. Additional Activity Sources
- Network activity monitoring
- Disk I/O monitoring
- Process-specific monitoring
- User session detection

### 2. Power Management Backends
- Alternative to TLP
- Custom power profiles
- Hardware-specific optimizations

### 3. Notification Systems
- Desktop notifications
- Log aggregation systems
- Monitoring system integration

## Testing Strategy Patterns

### 1. Component Testing
- Mock TLP commands for testing
- Configurable CPU thresholds for testing
- Separate test builds with debug logging

### 2. Integration Testing
- Test full daemon lifecycle
- Verify systemd integration
- Test log rotation and cleanup

### 3. Performance Testing
- Long-running stability tests
- Resource usage monitoring
- Activity detection accuracy testing

## Memory Bank Content Guidelines

### What Belongs in Each Section:

#### **Component Responsibilities**
- **Purpose**: What each component is responsible for
- **Role**: How it fits in the overall architecture
- **Boundaries**: What it should and shouldn't do
- **NOT**: Implementation details, specific file operations, code snippets

#### **Key Design Patterns**
- **Actual Patterns**: Observer, Factory, Strategy, Abstraction patterns
- **Pattern Examples**: Code showing pattern structure
- **Pattern Benefits**: Why the pattern is used
- **NOT**: Implementation details, operational procedures, specific file operations

#### **Implementation Details**
- **Operational Details**: File operations, directory structures, path management
- **Technical Specifics**: How abstractions are implemented
- **Platform-Specific Operations**: What each platform implementation does
- **NOT**: Design patterns, architectural principles

#### **Critical Implementation Decisions**
- **Architectural Choices**: Why certain approaches were chosen
- **Trade-offs**: Benefits and costs of decisions
- **Strategic Direction**: High-level implementation strategy
- **NOT**: Detailed implementation steps, specific operations

#### **Architectural Rules**
- **Principles**: What should always/never be done
- **Guidelines**: How to maintain clean architecture
- **Anti-Patterns**: Common mistakes to avoid
- **NOT**: Implementation procedures, operational details

### Writing Guidelines:

#### **Use Correct Language**:
- ✅ "Component **responsibility**", "Component **purpose**", "Component **role**"
- ✅ "Pattern **structure**", "Pattern **benefits**", "Pattern **application**"
- ❌ "File operations moved", "Implementation handles", "Platform does X"

#### **Focus on Architecture, Not Operations**:
- ✅ "ServiceManager abstracts platform-specific service management"
- ❌ "ServiceManager handles file copying, directory creation, permission setting"

#### **Separate Concerns Properly**:
- **Patterns Section**: How components interact, architectural relationships
- **Implementation Section**: What specific operations are performed
- **Decisions Section**: Why architectural choices were made
