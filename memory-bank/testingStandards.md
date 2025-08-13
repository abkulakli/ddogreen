# Testing Standards - ddogreen

> **File Purpose**: Comprehensive testing standards, practices, and guidelines for ddogreen project
> **Content Focus**: Testing frameworks, conventions, coverage requirements, and quality assurance
> **Cross-Reference**: Works with coding standards in `codingStandards.md`

## Overview
This document establishes comprehensive testing standards for the ddogreen project, ensuring code quality, maintainability, and consistency across the cross-platform codebase.

## Test Framework

### GoogleTest Integration
- **Framework**: GoogleTest (gtest) with Google Mock for mocking
- **Build System**: CMake with CTest integration for automatic test discovery
- **Test Runner**: CTest for IDE compatibility and CI/CD integration
- **Discovery**: `gtest_discover_tests()` automatically registers all test methods

### Build Configuration
```cmake
# Enable testing in CMakeLists.txt
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)
    add_subdirectory(tests)
endif()
```

## Test Structure and Organization

### Directory Structure
```
tests/
├── unit/
│   ├── test_activity_monitor.cpp
│   ├── test_config.cpp
│   ├── test_logger.cpp
│   └── platform/
│       ├── test_linux_power_manager.cpp
│       ├── test_windows_power_manager.cpp
│       └── test_platform_factory.cpp
├── integration/
│   ├── test_full_daemon.cpp
│   └── test_service_lifecycle.cpp
└── mocks/
    ├── mock_power_manager.h
    ├── mock_system_monitor.h
    └── mock_platform_utils.h
```

### Test File Naming
- **Test files**: `test_[component].cpp`
- **Mock files**: `mock_[interface].h`
- **Test classes**: `Test[Component]`
- **Test methods**: `test_[functionality]_[scenario]_[expected_result]()`

### Test Method Naming Convention
```cpp
TEST_F(TestConfig, test_load_from_file_returns_true_with_valid_config)
TEST_F(TestConfig, test_load_from_file_rejects_monitoring_frequency_below_minimum)
TEST_F(TestConfig, test_constructor_initializes_values_to_zero)
```

## Unit Testing Standards

### Test Class Structure
```cpp
#include <gtest/gtest.h>
#include "config.h"

class TestConfig : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup test environment
        config = std::make_unique<Config>();
        
        // Create temporary directory for test files
        tempDir = createTempDirectory();
    }

    void TearDown() override
    {
        // Cleanup test environment
        config.reset();
        
        // Remove temporary files
        cleanupTempDirectory(tempDir);
    }

    std::unique_ptr<Config> config;
    std::string tempDir;
};
```

### Test Method Structure (Arrange-Act-Assert)
```cpp
TEST_F(TestConfig, test_load_from_file_returns_true_with_valid_config)
{
    // Arrange
    std::string configPath = tempDir + "/test.conf";
    createValidConfigFile(configPath);

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(0.70, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.30, config->getPowerSaveThreshold());
    EXPECT_EQ(10, config->getMonitoringFrequency());
}
```

### Test Categories and Coverage

#### Core Functionality Tests
- **Constructor behavior**: Zero initialization verification
- **File I/O operations**: Success and failure scenarios
- **Configuration parsing**: Valid and invalid format handling
- **Value validation**: Range checking for all parameters
- **Getter methods**: Correct value retrieval after loading

#### Error Handling Tests
```cpp
TEST_F(TestConfig, test_load_from_file_returns_false_when_file_not_found)
{
    // Arrange
    std::string nonExistentPath = "/path/that/does/not/exist";

    // Act
    bool result = config->loadFromFile(nonExistentPath);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_rejects_invalid_monitoring_frequency)
{
    // Arrange
    std::string configPath = tempDir + "/invalid.conf";
    createConfigWithInvalidFrequency(configPath);

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_FALSE(result);
}
```

#### Edge Cases and Boundary Testing
```cpp
TEST_F(TestConfig, test_monitoring_frequency_accepts_minimum_value)
{
    // Test minimum valid value (1 second)
    EXPECT_TRUE(testConfigValue("monitoring_frequency=1"));
}

TEST_F(TestConfig, test_monitoring_frequency_accepts_maximum_value)
{
    // Test maximum valid value (300 seconds)
    EXPECT_TRUE(testConfigValue("monitoring_frequency=300"));
}

TEST_F(TestConfig, test_monitoring_frequency_rejects_below_minimum)
{
    // Test below minimum (0 seconds)
    EXPECT_FALSE(testConfigValue("monitoring_frequency=0"));
}
```

## Mock Objects and Test Doubles

### Mock Interface Implementation
```cpp
class MockPowerManager : public IPowerManager
{
public:
    MOCK_METHOD(bool, setPerformanceMode, (), (override));
    MOCK_METHOD(bool, setPowerSavingMode, (), (override));
    MOCK_METHOD(PowerMode, getCurrentMode, (), (const, override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
};

class MockPlatformUtils : public IPlatformUtils
{
public:
    MOCK_METHOD(bool, hasRequiredPrivileges, (), (const, override));
    MOCK_METHOD(std::string, getPrivilegeEscalationMessage, (), (const, override));
    MOCK_METHOD(std::string, getExecutablePath, (), (const, override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
};
```

### Using Mocks in Tests
```cpp
TEST_F(TestDaemon, test_start_switches_to_performance_mode_when_load_high)
{
    // Arrange
    auto mockPowerManager = std::make_unique<MockPowerManager>();
    auto mockSystemMonitor = std::make_unique<MockSystemMonitor>();
    
    EXPECT_CALL(*mockSystemMonitor, getCurrentLoad())
        .WillOnce(Return(0.8));  // High load
    EXPECT_CALL(*mockPowerManager, setPerformanceMode())
        .WillOnce(Return(true));

    Daemon daemon(std::move(mockPowerManager), std::move(mockSystemMonitor));

    // Act
    daemon.checkAndSwitchMode();

    // Assert - Expectations verified by mock framework
}
```

## Integration Testing

### Service Lifecycle Testing
```cpp
TEST(IntegrationTest, test_full_daemon_lifecycle)
{
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

### Cross-Platform Testing
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

## Test Coverage Standards

### Coverage Requirements
- **Line Coverage Target**: Minimum 70% overall, 90% for core business logic
- **Function Coverage Target**: Minimum 85% overall
- **Branch Coverage**: All critical decision paths must be tested
- **Platform Coverage**: All platform implementations must have equivalent test coverage

### Coverage Tools and Configuration
- **Tool**: gcov/lcov for coverage analysis
- **CMake Integration**: BUILD_WITH_COVERAGE option for coverage builds
- **Automation**: `scripts/coverage.sh` for automated coverage analysis
- **Reporting**: HTML reports generated at `build/coverage/coverage/html/index.html`

### Coverage Build Configuration
```cmake
# Coverage option
option(BUILD_WITH_COVERAGE "Enable coverage reporting" OFF)

if(BUILD_WITH_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
```

### Coverage Analysis Workflow
1. **Configure Coverage Build**: `cmake -B build/coverage -DBUILD_WITH_COVERAGE=ON -DBUILD_TESTS=ON`
2. **Build Project**: `cmake --build build/coverage`
3. **Generate Coverage Report**: `cmake --build build/coverage --target coverage`
4. **Quick Summary**: `cmake --build build/coverage --target coverage-summary`
5. **View HTML Report**: Open `build/coverage/coverage/html/index.html`

### Coverage CMake Targets
- **`coverage`**: Complete coverage analysis with HTML report generation
- **`coverage-summary`**: Quick console summary of coverage metrics
- **Dependencies**: Automatically builds project and runs all tests before analysis

### Coverage Exclusions
- Test files (`tests/**`)
- System headers (`/usr/include/**`)
- External dependencies (`build/_deps/**`)
- Platform-specific code not relevant to current build target

### Current Coverage Status
- **Line Coverage**: 64.0% (419 of 655 lines)
- **Function Coverage**: 84.9% (73 of 86 functions)
- **Files Analyzed**: 14 source files
- **Test Success Rate**: 100% (70/70 tests)

### Coverage Verification
```bash
# Build with coverage
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON

# Run tests
make test

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info '*/tests/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Performance and Quality Standards

### Test Performance Requirements
- **Fast execution**: All tests complete in under 30 seconds
- **Test isolation**: Each test uses independent temporary files/directories
- **No external dependencies**: Tests use mocks instead of real system resources
- **Clean output**: Logger output suppressed during test execution

### Test Isolation Techniques
```cpp
class TestConfig : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create unique temporary directory
        tempDir = std::filesystem::temp_directory_path() / 
                  ("ddogreen_test_" + std::to_string(std::time(nullptr)) + 
                   "_" + std::to_string(std::rand()));
        std::filesystem::create_directories(tempDir);
        
        // Suppress logger output during tests
        Logger::setLevel(Logger::Level::CRITICAL);
    }

    void TearDown() override
    {
        // Clean up temporary directory
        if (std::filesystem::exists(tempDir))
        {
            std::filesystem::remove_all(tempDir);
        }
        
        // Restore logger level
        Logger::setLevel(Logger::Level::INFO);
    }
};
```

## Build Integration

### CMake Test Configuration
```cmake
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)

    # Create test executable
    add_executable(test_config
        test_config.cpp
        ${CMAKE_SOURCE_DIR}/src/config.cpp
    )

    target_link_libraries(test_config
        GTest::gtest_main
        ${CMAKE_THREAD_LIBS_INIT}
    )

    # Automatic test discovery
    gtest_discover_tests(test_config)
endif()
```

### Running Tests
```bash
# Using CMake presets (recommended)
cmake --preset debug
cmake --build --preset debug
ctest --preset debug-tests

# Manual build and test
mkdir -p build
cd build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
ctest --output-on-failure

# Run specific tests
./tests/test_config
./tests/test_config --gtest_filter="TestConfig.test_load_from_file_*"
ctest -R "TestConfig.test_constructor_initializes_values_to_zero"
```

## Continuous Integration Integration

### CI Requirements
- **Universal compatibility**: Works with any CI system supporting CTest
- **Clear exit codes**: Proper success/failure indication
- **Structured output**: CTest provides standardized test results
- **Artifact support**: Test results can be uploaded for debugging

### GitHub Actions Integration
```yaml
- name: Run Tests
  run: |
    cd build
    ctest --output-on-failure --verbose
    
- name: Upload Test Results
  if: failure()
  uses: actions/upload-artifact@v3
  with:
    name: test-results
    path: build/Testing/
```

## Adding New Tests

### When Adding New Features
1. **Add validation tests** for new parameter ranges
2. **Update boundary tests** if ranges change
3. **Add missing parameter tests** for new required fields
4. **Include malformed value tests** for new data types
5. **Update integration tests** if parameter interactions change

### Test Documentation Requirements
- **Clear test names** that describe expected behavior
- **Helper methods** for common operations (file creation, path generation)
- **Consistent structure** following arrange-act-assert pattern
- **Error scenario coverage** for all failure modes

## Quality Assurance Integration

### Pre-commit Testing
- Code formatting verification
- Basic compilation test
- Unit test execution
- Static analysis scan

### Code Review Requirements
- Test coverage review for new features
- Mock usage verification for external dependencies
- Platform abstraction compliance in tests
- Performance impact assessment for test changes

This comprehensive testing standard ensures the reliability and maintainability of the ddogreen project while following established patterns and cross-platform best practices.
