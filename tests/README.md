# Testing Documentation - ddogreen

## Overview

This document describes the unit testing setup for ddogreen, particularly focusing on the `config.cpp` component tests.

## Test Framework

- **Framework**: GoogleTest (gtest)
- **Build System**: CMake with CTest integration
- **Test Runner**: CTest

## Test Structure

### Config Tests (`tests/test_config.cpp`)

Comprehensive unit tests for the `Config` class covering:

#### Core Functionality
- Constructor initialization
- Configuration file loading
- Default configuration path retrieval
- Getter methods validation

#### File Parsing
- Valid configuration parsing
- Comment and empty line handling
- Whitespace trimming
- Malformed line handling

#### Validation Testing
- **Monitoring Frequency**: Range validation (1-300 seconds)
- **High Performance Threshold**: Range validation (0.1-1.0)
- **Power Save Threshold**: Range validation (0.05-0.9)
- Required parameter validation
- Data type validation

#### Error Handling
- File not found scenarios
- Missing required configuration
- Invalid value ranges
- Non-numeric values
- Unknown configuration keys

#### Edge Cases
- Empty files
- Comment-only files
- Boundary value testing
- Whitespace-only values

## Building and Running Tests

### Prerequisites

Install GoogleTest development package:

```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev

# Fedora/RHEL
sudo dnf install gtest-devel

# macOS (with Homebrew)
brew install googletest
```

### Automatic Test Discovery

The tests use GoogleTest with CTest integration for automatic discovery:

- **CTest Integration**: `include(CTest)` and `gtest_discover_tests()` automatically register all test methods
- **IDE Compatible**: Any IDE with CTest support will discover tests automatically
- **Individual Execution**: Each test method runs as a separate CTest case
- **No Configuration Required**: Tests are discovered without additional setup
- **Clean Output**: Proper CTest initialization eliminates configuration warnings

### Quick Test Run

Use the provided test script:

```bash
./test.sh
```

### Manual Build and Test

```bash
# Create build directory with tests enabled
mkdir -p build
cd build
cmake .. -DBUILD_TESTS=ON

# Build tests
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Running Specific Tests

```bash
# Run only config tests
./tests/test_config

# Run with GoogleTest filter
./tests/test_config --gtest_filter="TestConfig.test_load_from_file_*"

# Run individual test via CTest
ctest -R "TestConfig.test_constructor_initializes_values_to_zero"
```

## Test Coverage

The config tests provide comprehensive coverage of:

✅ **Constructor behavior** - Zero initialization verification
✅ **File I/O operations** - Success and failure scenarios  
✅ **Configuration parsing** - Valid and invalid format handling
✅ **Value validation** - Range checking for all parameters
✅ **Error handling** - Graceful failure with invalid inputs
✅ **Edge cases** - Boundary conditions and malformed data
✅ **Getter methods** - Correct value retrieval after loading

## Test Design Principles

### Following Project Standards
- Uses GoogleTest framework as specified in coding standards
- Follows `TestConfig` class naming convention
- Implements proper setup/teardown lifecycle
- Uses descriptive test method names with underscores

### Test Isolation
- Each test uses temporary files in unique directories
- Logger level suppression to avoid test output pollution
- Proper cleanup in teardown methods
- Independent test execution

### Comprehensive Coverage
- Tests both positive and negative scenarios
- Validates boundary conditions for all numeric ranges
- Covers error paths and edge cases
- Tests integration between parsing and validation

### Maintainable Design
- Helper methods for common operations (file creation, path generation)
- Clear arrange-act-assert structure
- Descriptive test names that document expected behavior
- Consistent error scenario testing

## Adding New Tests

When adding new configuration parameters or modifying existing ones:

1. **Add validation tests** for new parameter ranges
2. **Update boundary tests** if ranges change
3. **Add missing parameter tests** for new required fields
4. **Include malformed value tests** for new data types
5. **Update integration tests** if parameter interactions change

### Test Naming Convention

```cpp
TEST_F(TestConfig, test_[method]_[scenario]_[expected_outcome])
```

Examples:
- `test_load_from_file_returns_true_with_valid_config`
- `test_load_from_file_rejects_monitoring_frequency_below_minimum`
- `test_constructor_initializes_values_to_zero`

## Integration with CI/CD

The test suite is designed for universal integration:

- **Fast execution** - All tests complete in under 30 seconds
- **Clear output** - CTest provides structured test results
- **Exit codes** - Proper success/failure indication for CI systems
- **No external dependencies** - Tests use temporary files and standard components
- **IDE Agnostic** - Works with any IDE that supports CTest/GoogleTest

## Performance Considerations

- Tests use in-memory file operations where possible
- Temporary directories are cleaned up automatically
- Logger output is suppressed during test execution
- No network or external service dependencies

This testing framework ensures the reliability and maintainability of the ddogreen configuration system while following established project patterns and standards.
