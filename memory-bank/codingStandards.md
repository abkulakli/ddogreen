# Coding Standards - ddogreen

> **File Purpose**: Comprehensive coding standards, practices, and conventions for ddogreen project
> **Content Focus**: Code formatting, architecture compliance, documentation, and quality standards
> **Cross-Reference**: For testing standards, see `testingStandards.md`

## Overview
This document establishes comprehensive coding standards for the ddogreen project. These standards ensure code quality, maintainability, and consistency across the cross-platform codebase.

**For Testing Standards**: See [`testingStandards.md`](testingStandards.md) for comprehensive testing guidelines, frameworks, and practices.

## **DOCUMENTATION PHILOSOPHY** ⚠️

### Minimal Documentation Principle
**CRITICAL GUIDELINE**: This project follows an ultra-minimal documentation approach:

- **Do NOT**: Create additional .md files without explicit user request
- **Do NOT**: Add comprehensive documentation for simple functionality
- **Do NOT**: Create README files for individual directories/components
- **Do**: Keep scripts self-documenting with built-in help text
- **Do**: Use clear, descriptive code that documents itself
- **Do**: Align all documentation with the project's simplicity principle

### Documentation Standards
- **Self-Documenting Code**: Write clear, readable code that explains its purpose
- **Built-in Help**: Scripts should include `--help` options with usage examples
- **Minimal External Docs**: Only create documentation files when absolutely necessary
- **Avoid Redundancy**: Never duplicate information across multiple files

## Coding Standards

### 1. File Organization and Naming

#### File Naming Conventions
- **Header files**: Use `.h` extension, no `.hpp`
- **Source files**: Use `.cpp` extension
- **File names**: Use snake_case (e.g., `activity_monitor.h`, `linux_power_manager.cpp`)
- **Interface headers**: Prefix with `i` (e.g., `ipower_manager.h`, `idaemon.h`)
- **Platform implementations**: Include platform name (e.g., `linux_power_manager.cpp`)

#### Directory Structure
```
include/
├── [component].h              # Public interfaces and main components
└── platform/
    └── i[interface].h         # Platform abstraction interfaces

src/
├── [component].cpp            # Component implementations
└── platform/
    ├── platform_factory.cpp  # Platform selection logic
    ├── linux/
    │   └── linux_[component].cpp
    ├── windows/
    │   └── windows_[component].cpp
    └── macos/
        └── macos_[component].cpp
```

### 2. Code Formatting and Style

#### Indentation and Spacing
- **Indentation**: 4 spaces (no tabs)
- **Line length**: Maximum 100 characters
- **Blank lines**: Single blank line between functions, two between classes
- **Trailing whitespace**: None allowed

#### Naming Conventions
- **Classes**: PascalCase (e.g., `ActivityMonitor`, `IPowerManager`)
- **Functions/Methods**: camelCase (e.g., `hasRequiredPrivileges()`, `setPerformanceMode()`)
- **Variables**: camelCase (e.g., `isActive`, `currentMode`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `DEFAULT_THRESHOLD`, `MAX_RETRY_COUNT`)
- **Private members**: Prefix with `m_` (e.g., `m_logFile`, `m_isRunning`)

#### Braces and Formatting
```cpp
// CORRECT - Allman style braces
class ActivityMonitor
{
public:
    void startMonitoring()
    {
        if (m_isRunning)
        {
            return;
        }
        // Implementation
    }

private:
    bool m_isRunning = false;
};

// CORRECT - Function formatting
bool hasRequiredPrivileges() const
{
    return geteuid() == 0;
}
```

### 3. Documentation Standards

#### Header Documentation
```cpp
/**
 * Brief description of the class/function purpose
 *
 * Detailed description if needed. Explain the role in the system,
 * any important constraints, or usage patterns.
 */
class ActivityMonitor
{
public:
    /**
     * Start monitoring system activity
     * @param threshold CPU usage threshold (0.0-1.0)
     * @param callback Function to call when activity state changes
     * @return true if monitoring started successfully
     */
    bool startMonitoring(double threshold, std::function<void(bool)> callback);
};
```

#### Inline Comments
```cpp
// Single line for brief explanations
int calculateCpuUsage()
{
    // Read current CPU stats from /proc/stat
    std::ifstream file("/proc/stat");

    /* Multi-line comment for complex algorithms:
     * CPU usage calculation based on kernel documentation
     * Formula: (total_time - idle_time) / total_time
     */
    return usage;
}
```

### 4. Architecture Compliance

#### Platform Abstraction Rules
```cpp
// CORRECT - Platform-agnostic application layer
auto platformUtils = PlatformFactory::createPlatformUtils();
if (!platformUtils->hasRequiredPrivileges()) {
    std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
    return 1;
}

// WRONG - Platform-specific code in application layer
#ifdef _WIN32
    if (!CheckTokenMembership(...)) { /* Windows code */ }
#else
    if (geteuid() != 0) { /* Linux code */ }
#endif
```

#### Interface Design
```cpp
// CORRECT - Complete platform abstraction
class IPlatformUtils
{
public:
    virtual ~IPlatformUtils() = default;
    virtual bool hasRequiredPrivileges() const = 0;
    virtual std::string getPrivilegeEscalationMessage() const = 0;
    virtual bool isAvailable() const = 0;
};

// WRONG - Leaking platform-specific types
class IPlatformUtils
{
public:
    virtual DWORD getWindowsError() const = 0;  // Windows-specific type
    virtual uid_t getUserId() const = 0;        // Linux-specific type
};
```

#### Monitoring Frequency Configuration Pattern
```cpp
// CORRECT - Dynamic load calculation adapting to monitoring frequency
void setMonitoringFrequency(int frequencySeconds) override {
    m_monitoringFrequency = frequencySeconds;
    
    // Calculate dynamic sample window: 60 seconds / frequency
    const size_t samplesFor1Min = (m_monitoringFrequency <= 0) ? 1 : 
                                  (60 / m_monitoringFrequency < 1) ? 1 : 
                                  (60 / m_monitoringFrequency);
    
    // Update sample window for load averaging
    updateSampleWindow(samplesFor1Min);
    Logger::debug("Monitoring frequency set to " + std::to_string(frequencySeconds) + " seconds");
}

// WRONG - Fixed calculations ignoring configuration
void setMonitoringFrequency(int frequencySeconds) override {
    // Ignore frequency, always use 60 samples - NOT CORRECT
    const size_t fixedSamples = 60;
    updateSampleWindow(fixedSamples);
}
```

### 5. Error Handling Standards

#### Exception Policy
- **No exceptions**: Use return codes and error checking
- **Error propagation**: Use boolean returns or error codes
- **Logging**: Log all errors with appropriate severity

```cpp
// CORRECT - Error handling pattern
bool performOperation()
{
    if (!prerequisiteCheck())
    {
        Logger::error("Prerequisite check failed for operation");
        return false;
    }

    // Perform operation
    if (operationResult != SUCCESS)
    {
        Logger::warning("Operation completed with warnings");
        return false;
    }

    return true;
}
```

#### Resource Management
```cpp
// CORRECT - RAII pattern
class FileManager
{
public:
    FileManager(const std::string& filename) : m_file(filename)
    {
        if (!m_file.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }
    }

    ~FileManager()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
    }

private:
    std::fstream m_file;
};
```

### 6. Memory Management

#### Smart Pointers
```cpp
// CORRECT - Use smart pointers
std::unique_ptr<IPowerManager> createPowerManager()
{
    return std::make_unique<LinuxPowerManager>();
}

// WRONG - Raw pointers for ownership
IPowerManager* createPowerManager()
{
    return new LinuxPowerManager();  // Memory leak risk
}
```

#### Container Usage
```cpp
// CORRECT - Standard containers
std::vector<std::string> getLogEntries() const;
std::unordered_map<std::string, int> getCounters() const;

// Prefer const references for parameters
void processEntries(const std::vector<std::string>& entries);
```

## Code Quality Tools

### Static Analysis

#### Compiler Warnings
```cmake
# CMakeLists.txt configuration
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")
endif()
```

#### Additional Tools
- **cppcheck**: Static analysis for bugs and style issues
- **clang-tidy**: Modernization and best practices
- **valgrind**: Memory leak detection (Linux)

### Formatting Tools

#### clang-format Configuration
```yaml
# .clang-format
BasedOnStyle: LLVM
IndentWidth: 4
AllowShortFunctionsOnASingleLine: None
AllowShortIfStatementsOnASingleLine: false
BreakBeforeBraces: Allman
ColumnLimit: 100
```

### Build Standards

#### Build Requirements
```bash
# All builds must pass without warnings
cmake --preset debug -DCMAKE_CXX_FLAGS="-Werror"    # Enable all warnings as errors
cmake --preset debug -DENABLE_SANITIZERS=ON         # Enable address/thread sanitizers
cmake --preset debug -DENABLE_STATIC_ANALYSIS=ON    # Run static analysis tools
```

#### Continuous Integration
- All tests must pass on target platforms
- Code coverage must meet minimum thresholds
- Static analysis must report no issues
- Build time must remain under 30 seconds

## Enforcement

### Pre-commit Checks
- Code formatting verification
- Basic compilation test
- Unit test execution
- Static analysis scan

### Code Review Requirements
- Architecture compliance verification
- Test coverage review
- Documentation completeness
- Performance impact assessment

### Quality Gates
- No code merged without tests
- No warnings in production builds
- All public APIs must be documented
- Platform abstraction compliance verified

This comprehensive standard ensures maintainable, reliable, and consistent code across the entire ddogreen project.

**For Testing Guidelines**: See [`testingStandards.md`](testingStandards.md) for detailed testing practices, frameworks, and quality assurance standards.
