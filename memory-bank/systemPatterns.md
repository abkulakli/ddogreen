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

### Cross-Platform Architecture
```
Application Layer (main.cpp, daemon.cpp)
├── Platform Abstraction Layer
│   ├── IPowerManager
│   ├── ISystemMonitor  
│   ├── IServiceManager
│   └── IPlatformUtils
└── Platform Implementations
    ├── Linux (TLP integration)
    ├── Windows (Power Plans)
    └── macOS (future)
```

### Architectural Principles
- **No platform-specific code** in application layer
- **Interface-based design** for all platform operations
- **Factory pattern** for platform selection
- **RAII** for resource management
- **Compile-time platform detection** for optimal binaries

### Platform-Specific Implementation Details

#### Linux Implementation
- **Power Management**: TLP integration (`tlp ac`, `tlp bat`)
- **System Monitoring**: `/proc/loadavg` reading
- **Service Management**: systemd integration
- **Privilege Checking**: `geteuid()` for root access

#### Windows Implementation
- **Power Management**: Power Plans via `powercfg`
- **System Monitoring**: Performance Counters
- **Service Management**: Windows Service Control Manager
- **Privilege Checking**: Windows Token API

#### macOS Implementation (Future)
- **Power Management**: `pmset` integration planned
- **System Monitoring**: System framework APIs
- **Service Management**: launchd integration
- **Privilege Checking**: Authorization Services

## Memory Bank Organization Guidelines

### Content Placement Rules

#### projectbrief.md - Foundation Document
**Purpose**: Project scope, requirements, and core objectives
**Content Should Include**:
- Project overview and final status
- Core objectives and what was achieved
- Project type (language, build system, architecture)
- Key functional, technical, and performance requirements
- Success criteria and completion status

**Content Should NOT Include**:
- Implementation details or code specifications
- Progress tracking or milestone dates
- Current work focus or active context
- Detailed technical patterns or architecture

#### productContext.md - Business Purpose
**Purpose**: Why project exists and user value proposition
**Content Should Include**:
- Problem statement and solution overview
- Real-world impact and business value
- User experience descriptions
- Market need and target audience
- Benefits and value delivered

**Content Should NOT Include**:
- Technical implementation details
- Code architecture or patterns
- Development progress or milestones
- Current work status or active tasks

#### activeContext.md - Current State Focus
**Purpose**: Current work focus and immediate context
**Content Should Include**:
- Current work status and priorities
- Recent achievements and immediate context
- Active decisions and working patterns
- Next steps and immediate tasks
- Current blockers or challenges
- Recent technical decisions and insights

**Content Should NOT Include**:
- Completed historical achievements (belongs in progress.md)
- Detailed technical specifications (belongs in systemPatterns.md)
- Architecture patterns (belongs in systemPatterns.md)
- Technology stack details (belongs in techContext.md)

#### systemPatterns.md - Architecture & Design
**Purpose**: System architecture, patterns, and technical decisions
**Content Should Include**:
- Architecture overview and component relationships
- Design patterns and implementation strategies
- Configuration system architecture
- Code organization principles
- Technical specifications and system design
- Interface definitions and component responsibilities

**Content Should NOT Include**:
- Progress tracking or completion status
- Technology stack or build setup (belongs in techContext.md)
- Current work focus (belongs in activeContext.md)
- Historical milestones (belongs in progress.md)

#### techContext.md - Technology Stack
**Purpose**: Technologies, tools, and development environment
**Content Should Include**:
- Programming languages and frameworks
- Build system configuration and requirements
- Dependencies and external integrations
- Development tools and environment setup
- Platform requirements and constraints
- Tool usage patterns and configurations

**Content Should NOT Include**:
- Architecture patterns (belongs in systemPatterns.md)
- Progress tracking (belongs in progress.md)
- Current work focus (belongs in activeContext.md)
- Implementation specifications (belongs in systemPatterns.md)

#### progress.md - Achievements & Milestones
**Purpose**: What has been completed and project evolution
**Content Should Include**:
- Major milestones and their completion dates
- Completed features and achievements
- Project health metrics and status
- Evolution of major decisions
- Final status and delivery confirmation
- High-level "what works" summaries

**Content Should NOT Include**:
- Detailed implementation specifications (belongs in systemPatterns.md)
- Current work focus (belongs in activeContext.md)
- Architecture patterns (belongs in systemPatterns.md)
- Technology setup details (belongs in techContext.md)

### Content Flow Guidelines

#### When Content Should Move Between Files:
1. **Completed Work**: Move from activeContext.md → progress.md when work is finished
2. **Architecture Decisions**: Move from activeContext.md → systemPatterns.md when patterns are established
3. **Tech Stack Changes**: Update techContext.md when tools or technologies change
4. **Requirements Evolution**: Update projectbrief.md only for scope changes

#### Red Flags for Misplaced Content:
- **In progress.md**: Detailed "how it works" technical specifications
- **In activeContext.md**: Completed work that's no longer active
- **In systemPatterns.md**: Progress tracking or milestone information
- **In techContext.md**: Architecture patterns or design decisions
- **In projectbrief.md**: Current work status or detailed implementations

#### File Size Guidelines:
- **activeContext.md**: Should be concise, focus on immediate context
- **progress.md**: Should focus on achievements, not implementation details
- **systemPatterns.md**: Can be detailed for architecture, but avoid progress tracking
- **techContext.md**: Focus on setup and tools, not patterns
- **projectbrief.md**: High-level only, avoid technical deep dives
- **productContext.md**: User-focused, avoid technical implementation details

## Documentation Principles

### File Documentation Strategy
- **Component README Files**: Detailed documentation for each major component
- **Memory Bank**: Context, implementation decisions, and progress tracking
- **No Redundant Files**: Avoid creating additional markdown files that duplicate information
- **Consolidated Maintenance**: Keep documentation in minimal, well-defined locations

### Documentation Standards
- Use existing README files in component directories (like `tests/README.md`)
- Document architectural decisions and patterns in `systemPatterns.md`
- Track progress and context in memory bank files
- Avoid creating summary files that duplicate information from other sources

## Code Organization Principles

### Simplified Build Structure (Updated July 2025)
```
ddogreen/
├── build.sh                    # Simplified build script
├── CMakeLists.txt             # Build configuration
└── build/                     # Platform-specific builds
    ├── linux/
    │   ├── debug/             # Linux debug builds
    │   └── release/           # Linux release builds
    ├── windows/
    │   ├── debug/             # Windows debug builds  
    │   └── release/           # Windows release builds
    └── macos/
        ├── debug/             # macOS debug builds
        └── release/           # macOS release builds
```

### Build Script Features
- **Smart Clean**: `--clean` standalone or combined with build options
- **Platform Detection**: Automatic environment detection
- **Minimal Dependencies**: Only cmake and make required
- **Essential Options**: --debug, --release, --clean, --help
- **No Quality Enforcement**: Manual coding standards application

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

## Configuration System Architecture

### Read-Only Configuration Pattern
- **Design Principle**: Configuration file must exist, no auto-creation or defaults
- **Failure Mode**: Application fails cleanly when configuration missing (user requirement)
- **Validation**: Complete parameter validation with clear error messages
- **Parsing**: Robust key-value parsing with whitespace handling
- **Custom Paths**: `--config` parameter support for non-standard configuration locations

### Dual Threshold Hysteresis System
**Implementation Details**:
- **High Performance Threshold**: 0.7 (70% per core) - prevents oscillation
- **Power Save Threshold**: 0.3 (30% per core) - creates stable switching zones
- **Hysteresis Benefits**: Eliminates rapid mode switching between performance levels
- **Load Calculation**: Per-core basis (e.g., 20 cores = 14.00 load for high performance, 6.00 load for power save)

### Configuration File Format
```ini
# /etc/ddogreen/ddogreen.conf
high_performance_threshold=0.70
power_save_threshold=0.30  
monitoring_frequency=10
```

### User Experience Design
- **Number Formatting**: All outputs show exactly 2 decimal places consistently
- **Monitoring Frequency**: Configurable (default 10 seconds for responsiveness)
- **Clear Logging**: Threshold information displayed in both percentage and absolute values
- **Error Handling**: Single clear error messages, no redundant logging

## Component Responsibilities

#### main.cpp (Refactored - Clean Architecture)
- **Purpose**: Pure platform-agnostic orchestration and coordination
- **Zero ifdefs**: No platform-specific code whatsoever
- **No platform includes**: Only standard C++ and platform abstraction headers
- **Platform delegation**: All platform-specific logic delegated to platform layer
- **Clean separation**: Business logic separated from platform concerns

#### IPlatformUtils (New Interface)
- **Purpose**: Platform-specific utility functions abstraction
- **Responsibility**: Abstract platform-dependent operations like privilege checking and path management
- **Interface**: Provides uniform utility operations across platforms
- **Encapsulation**: Hides platform-specific implementation details from application code

#### ActivityMonitor
- **Purpose**: Monitor system activity and notify of state changes
- **Responsibility**: Detect system activity levels and trigger power mode changes
- **Pattern**: Uses Observer pattern to notify subscribers of activity state changes
- **Threading**: Operates asynchronously to avoid blocking main application flow

#### PowerManager (Platform Abstraction)
- **Purpose**: Cross-platform power management interface
- **Responsibility**: Abstract power mode switching and power state management
- **Interface**: Provides uniform power control operations across platforms
- **Implementation**: Platform-specific power control (Linux: TLP, Windows: powercfg)
- **Encapsulation**: Hides platform-specific power management details from application code

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

### Power Management Implementation
- **PowerManager Methods**: `setPerformanceMode()`, `setPowerSavingMode()`, `getCurrentMode()`
- **Linux Implementation**: Executes `tlp start` and `tlp bat` commands
- **Windows Implementation**: Uses `powercfg` command for power plan switching
- **Output Capture**: Captures and processes command output for logging
- **State Tracking**: Maintains current power mode state
- **Error Handling**: Robust command failure handling

### Activity Monitoring Implementation
- **Data Source**: Uses `/proc/stat` filesystem on Linux
- **Decision Logic**: CPU threshold-based activity detection (10% threshold)
- **Monitoring Interval**: 5-second polling for CPU usage calculation
- **Callback Mechanism**: Notifies main daemon of state changes via callback function

### Platform Utilities Implementation
- **Privilege Checking**: `hasRequiredPrivileges()` - Linux uses `geteuid()`, Windows uses token membership
- **Executable Path**: `getExecutablePath()` - Linux uses `/proc/self/exe`, Windows uses `GetModuleFileName`
- **Default Paths**: Platform-specific installation, log, and PID file paths
- **Error Messages**: Platform-appropriate privilege escalation messages

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
// Platform implementation handles all required platform-specific operations
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
