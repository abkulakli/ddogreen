# Coding and Testing Standards - ddogreen

## Overview
This document establishes comprehensive coding and testing standards for the ddogreen project. These standards ensure code quality, maintainability, and consistency across the cross-platform codebase.

## Coding Standards

### 1. File Organization and Naming

#### File Naming Conventions
- **Header files**: Use `.h` extension, no `.hpp`
- **Source files**: Use `.cpp` extension
- **File names**: Use snake_case (e.g., `activity_monitor.h`, `linux_power_manager.cpp`)
- **Interface headers**: Prefix with `i` (e.g., `ipower_manager.h`, `iservice_manager.h`)
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
// ✅ CORRECT - Allman style braces
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

// ✅ CORRECT - Function formatting
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
// ✅ CORRECT - Platform-agnostic application layer
auto platformUtils = PlatformFactory::createPlatformUtils();
if (!platformUtils->hasRequiredPrivileges()) {
    std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
    return 1;
}

// ❌ WRONG - Platform-specific code in application layer
#ifdef _WIN32
    if (!CheckTokenMembership(...)) { /* Windows code */ }
#else
    if (geteuid() != 0) { /* Linux code */ }
#endif
```

#### Interface Design
```cpp
// ✅ CORRECT - Complete platform abstraction
class IPlatformUtils
{
public:
    virtual ~IPlatformUtils() = default;
    virtual bool hasRequiredPrivileges() const = 0;
    virtual std::string getPrivilegeEscalationMessage() const = 0;
    virtual bool isAvailable() const = 0;
};

// ❌ WRONG - Leaking platform-specific types
class IPlatformUtils
{
public:
    virtual DWORD getWindowsError() const = 0;  // Windows-specific type
    virtual uid_t getUserId() const = 0;        // Linux-specific type
};
```

### 5. Error Handling Standards

#### Exception Policy
- **No exceptions**: Use return codes and error checking
- **Error propagation**: Use boolean returns or error codes
- **Logging**: Log all errors with appropriate severity

```cpp
// ✅ CORRECT - Error handling pattern
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
// ✅ CORRECT - RAII pattern
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
// ✅ CORRECT - Use smart pointers
std::unique_ptr<IPowerManager> createPowerManager()
{
    return std::make_unique<LinuxPowerManager>();
}

// ❌ WRONG - Raw pointers for ownership
IPowerManager* createPowerManager()
{
    return new LinuxPowerManager();  // Memory leak risk
}
```

#### Container Usage
```cpp
// ✅ CORRECT - Standard containers
std::vector<std::string> getLogEntries() const;
std::unordered_map<std::string, int> getCounters() const;

// Prefer const references for parameters
void processEntries(const std::vector<std::string>& entries);
```

## Code Quality Tools

### 1. Static Analysis

#### Test Structure
```
tests/
├── unit/
│   ├── test_activity_monitor.cpp
│   ├── test_logger.cpp
│   └── platform/
│       ├── test_linux_power_manager.cpp
│       └── test_platform_factory.cpp
├── integration/
│   ├── test_full_daemon.cpp
│   └── test_service_lifecycle.cpp
└── mocks/
    ├── mock_power_manager.h
    └── mock_system_monitor.h
```

#### Test Naming
- **Test files**: `test_[component].cpp`
- **Test functions**: `test_[functionality]_[expected_result]()`
- **Test classes**: `Test[Component]`

### 2. Unit Testing Standards

#### Test Function Structure
```cpp
#include <gtest/gtest.h>
#include "activity_monitor.h"

class TestActivityMonitor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test environment
        monitor = std::make_unique<ActivityMonitor>();
    }

    void TearDown() override
    {
        // Cleanup
        monitor.reset();
    }

    std::unique_ptr<ActivityMonitor> monitor;
};

TEST_F(TestActivityMonitor, test_start_monitoring_returns_true_when_not_running)
{
    // Arrange
    bool callbackCalled = false;
    auto callback = [&callbackCalled](bool active) { callbackCalled = true; };

    // Act
    bool result = monitor->startMonitoring(0.1, callback);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(monitor->isRunning());
}

TEST_F(TestActivityMonitor, test_start_monitoring_returns_false_when_already_running)
{
    // Arrange
    auto callback = [](bool active) {};
    monitor->startMonitoring(0.1, callback);

    // Act
    bool result = monitor->startMonitoring(0.1, callback);

    // Assert
    EXPECT_FALSE(result);
}
```

#### Mock Standards
```cpp
// Mock interface implementations for testing
class MockPowerManager : public IPowerManager
{
public:
    MOCK_METHOD(bool, setPerformanceMode, (), (override));
    MOCK_METHOD(bool, setPowerSavingMode, (), (override));
    MOCK_METHOD(PowerMode, getCurrentMode, (), (const, override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
};
```

### 3. Integration Testing Standards

#### Test Scenarios
```cpp
TEST(IntegrationTest, test_full_daemon_lifecycle)
{
    // Test complete daemon startup, operation, and shutdown

    // Arrange
    auto daemon = std::make_unique<Daemon>();

    // Act
    bool startResult = daemon->start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bool stopResult = daemon->stop();

    // Assert
    EXPECT_TRUE(startResult);
    EXPECT_TRUE(stopResult);
}
```

### 4. Platform Testing Standards

#### Platform-Specific Tests
```cpp
#ifdef __linux__
TEST(LinuxPlatformTest, test_tlp_availability_check)
{
    LinuxPowerManager manager;

    // This test only runs on Linux
    EXPECT_TRUE(manager.isAvailable() || !system("which tlp > /dev/null 2>&1"));
}
#endif

#ifdef _WIN32
TEST(WindowsPlatformTest, test_powercfg_availability)
{
    WindowsPowerManager manager;

    // This test only runs on Windows
    EXPECT_TRUE(manager.isAvailable());
}
#endif
```

#### Cross-Platform Interface Tests
```cpp
// Test that runs on all platforms
TEST(CrossPlatformTest, test_platform_utils_interface_compliance)
{
    auto utils = PlatformFactory::createPlatformUtils();

    // All platforms must implement these methods
    EXPECT_NO_THROW(utils->hasRequiredPrivileges());
    EXPECT_NO_THROW(utils->getExecutablePath());
    EXPECT_FALSE(utils->getPrivilegeEscalationMessage().empty());
}
```

### 5. Test Coverage Standards

#### Coverage Requirements
- **Unit tests**: Minimum 80% line coverage
- **Platform implementations**: 90% coverage for each platform
- **Critical paths**: 100% coverage (privilege checking, power management)
- **Error handling**: All error paths must be tested

#### Coverage Verification
```bash
# Generate coverage report
./build.sh --coverage
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Code Quality Tools

### 1. Static Analysis

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

### 2. Formatting Tools

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

### 3. Build Standards

#### Build Requirements
```bash
# All builds must pass without warnings
./build.sh --strict                    # Enable all warnings as errors
./build.sh --sanitize                  # Enable address/thread sanitizers
./build.sh --static-analysis           # Run static analysis tools
```

#### Continuous Integration
- All tests must pass on target platforms
- Code coverage must meet minimum thresholds
- Static analysis must report no issues
- Build time must remain under 30 seconds

## Enforcement

### 1. Pre-commit Checks
- Code formatting verification
- Basic compilation test
- Unit test execution
- Static analysis scan

### 2. Code Review Requirements
- Architecture compliance verification
- Test coverage review
- Documentation completeness
- Performance impact assessment

### 3. Quality Gates
- No code merged without tests
- No warnings in production builds
- All public APIs must be documented
- Platform abstraction compliance verified

This comprehensive standard ensures maintainable, reliable, and consistent code across the entire ddogreen project.
