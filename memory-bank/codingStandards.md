# DDOGreen Coding Standards

## Core Principles

This document establishes the coding standards for the ddogreen power management service, incorporating ISO C++ Core Guidelines for modern, safe, and efficient C++ development. These standards ensure code quality, maintainability, and consistency across the codebase.

### Architecture Compliance

- **NO #ifdef**: Platform-specific code must use the platform abstraction layer
- **Single Point of Truth**: Platform differences resolved through factory pattern
- **Interface Segregation**: Clean separation between platform-agnostic core and platform implementations
- **Dependency Injection**: Platform implementations injected through factory pattern

### C++ Core Guidelines Integration

This project follows the **ISO C++ Core Guidelines** for modern C++ development. Key principles include:

#### Resource Management and Safety
- **RAII (Resource Acquisition Is Initialization)**: All resources managed through constructor/destructor pairs
- **Prefer `std::span<T>` over pointer-length pairs**: Bounds-aware views for contiguous sequences
- **Use `span<const T>` by default**: Read-only access unless modification explicitly required
- **Avoid raw pointers for ownership**: Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- **Exception safety**: All operations provide basic or strong exception safety guarantee

#### Interface Design
- **Pass `span<T>` by value**: Treat spans like lightweight pointer ranges
- **Const-correctness**: Use `span<const T>` for read-only data, `const span<T>` for immutable spans
- **Prefer range-based for loops**: Zero-overhead iteration with automatic bounds checking
- **Use STL algorithms with spans**: Direct integration with `std::find_if`, `std::ranges`, etc.

#### Modern C++ Practices
- **Use `auto` for type deduction**: Improve maintainability and readability
- **Prefer `constexpr` for compile-time constants**: Enable compile-time optimization
- **Use uniform initialization**: `{}` syntax for consistent initialization
- **Range-based operations**: Leverage spans with STL algorithms for safe, efficient code

### Minimal Documentation Philosophy

- Code should be **self-documenting** through clear naming and structure
- Comments only when **business logic is non-obvious**
- **No redundant documentation** (avoid stating the obvious)
- Focus on **why** not **what** in comments
- **Update documentation with code changes** or remove outdated docs

## File Organization Standards

### Header Files (.h)

```cpp
// File: include/component_name.h
#pragma once

#include <span>
#include <memory>
#include <standard_library_headers>
#include "project_headers.h"
#include "platform/interfaces.h"

namespace ddogreen {

class ComponentName {
public:
    // Use span<const T> for read-only parameters
    virtual void process_data(std::span<const uint8_t> data) = 0;
    
    // Use span<T> for read-write parameters  
    virtual void fill_buffer(std::span<uint8_t> buffer) = 0;
    
    virtual ~ComponentName() = default;
    
private:
    // Implementation details hidden
};

} // namespace ddogreen
```

### Implementation Files (.cpp)

```cpp
// File: src/component_name.cpp
#include "component_name.h"

#include <algorithm>
#include <span>
#include <standard_library_headers>
#include "platform/platform_factory.h"

namespace ddogreen {

void ComponentImpl::process_data(std::span<const uint8_t> data) {
    // Range-based iteration (zero-overhead, bounds-safe)
    for (const auto& byte : data) {
        // Process each byte safely
    }
    
    // STL algorithm integration
    auto found = std::find_if(data.begin(), data.end(), 
        [](uint8_t b) { return b > 128; });
    
    if (found != data.end()) {
        // Handle found element
    }
}

void ComponentImpl::fill_buffer(std::span<uint8_t> buffer) {
    // Safe bounds checking
    if (!buffer.empty()) {
        // Fill buffer safely
        std::fill(buffer.begin(), buffer.end(), 0);
    }
}

} // namespace ddogreen
```

### Platform Implementation Files

```cpp
// File: src/platform/linux/linux_component.cpp
#include "linux_component.h"

#include <sys/linux_specific.h>
#include <algorithm>
#include <span>

namespace ddogreen::platform {

void LinuxComponent::platform_specific_operation(std::span<const char> config_data) {
    // Use spans for safe buffer operations
    if (config_data.empty()) {
        return;
    }
    
    // Safe iteration without bounds checking
    for (const auto& byte : config_data) {
        // Process platform-specific data
    }
    
    // Create sub-spans safely
    if (config_data.size() > 10) {
        auto header = config_data.first(10);
        auto payload = config_data.subspan(10);
        // Process header and payload separately
    }
}

} // namespace ddogreen::platform
```

## Code Style and Formatting

### Indentation and Spacing
- **Indentation**: 4 spaces (no tabs) - following C++ Core Guidelines document style
- **Line length**: Maximum 100 characters
- **Blank lines**: Single blank line between functions, two between classes
- **Trailing whitespace**: None allowed

### Naming Conventions
- **Classes**: PascalCase (e.g., `ActivityMonitor`, `IPowerManager`)
- **Functions/Methods**: camelCase (e.g., `hasRequiredPrivileges()`, `setPerformanceMode()`)
- **Variables**: camelCase (e.g., `isActive`, `currentMode`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `DEFAULT_THRESHOLD`, `MAX_RETRY_COUNT`)
- **Private members**: Prefix with `m_` (e.g., `m_logFile`, `m_isRunning`)

### Braces and Formatting
```cpp
// CORRECT - Allman style braces (C++ Core Guidelines compatible)
class ActivityMonitor
{
public:
    void startMonitoring()
    {
        if (m_isRunning)
        {
            return;
        }
        // Implementation using modern C++ practices
    }

private:
    bool m_isRunning{false}; // Uniform initialization
};

// CORRECT - Function formatting with spans
void process_buffer(std::span<const uint8_t> data)
{
    // Range-based for with auto deduction
    for (const auto& byte : data) {
        // Safe processing
    }
}
```

## Memory Safety and Resource Management

### RAII and Smart Pointers
```cpp
// CORRECT - RAII for file handling
class ConfigFile
{
public:
    explicit ConfigFile(const std::string& path)
        : m_file{std::make_unique<std::ifstream>(path)}
    {
        if (!m_file->is_open()) {
            throw std::runtime_error("Failed to open config file");
        }
    }
    
    // Destructor automatically closes file through unique_ptr
    
    std::span<const char> read_section(std::size_t offset, std::size_t length);
    
private:
    std::unique_ptr<std::ifstream> m_file;
};
```

### Span Usage Patterns
```cpp
// CORRECT - Span parameter design
class DataProcessor
{
public:
    // Read-only data with span<const T>
    bool validate_config(std::span<const uint8_t> config_data) const
    {
        if (config_data.empty()) {
            return false;
        }
        
        // Safe iteration
        return std::all_of(config_data.begin(), config_data.end(),
            [](uint8_t byte) { return byte < 128; });
    }
    
    // Read-write data with span<T>
    void encrypt_buffer(std::span<uint8_t> buffer) const
    {
        for (auto& byte : buffer) {
            byte ^= 0xAA; // Simple XOR encryption
        }
    }
    
    // Creating sub-spans safely
    void process_packet(std::span<const uint8_t> packet)
    {
        if (packet.size() < 4) {
            return; // Invalid packet
        }
        
        auto header = packet.first(4);
        auto payload = packet.subspan(4);
        
        // Process header and payload separately
    }
};
```

## Error Handling Standards

### Exception Safety
```cpp
// CORRECT - Strong exception safety guarantee
class ActivityMonitor
{
public:
    void update_config(std::span<const char> new_config)
    {
        // Validate before modifying state
        if (!validate_config(new_config)) {
            throw std::invalid_argument("Invalid configuration format");
        }
        
        // Copy-and-swap idiom for strong exception safety
        auto temp_config = parse_config(new_config);
        std::swap(m_config, temp_config);
        // Previous config automatically cleaned up
    }
    
private:
    ConfigData m_config;
    
    bool validate_config(std::span<const char> config) const;
    ConfigData parse_config(std::span<const char> config) const;
};
```

## Platform Abstraction with Modern C++

### Interface Design
```cpp
// File: include/platform/ipower_manager.h
#pragma once

#include <span>
#include <memory>
#include <cstdint>

namespace ddogreen::platform {

class IPowerManager
{
public:
    virtual ~IPowerManager() = default;
    
    // Use spans for configuration data
    virtual bool set_power_profile(std::span<const uint8_t> profile_data) = 0;
    virtual void get_power_status(std::span<uint8_t> status_buffer) const = 0;
    
    // Modern C++ with auto return type deduction
    virtual auto get_supported_modes() const -> std::vector<PowerMode> = 0;
    
protected:
    // Protected constructor prevents direct instantiation
    IPowerManager() = default;
};

} // namespace ddogreen::platform
```

### Factory Implementation
```cpp
// File: src/platform/platform_factory.cpp
#include "platform_factory.h"

#include <memory>

namespace ddogreen::platform {

auto PlatformFactory::create_power_manager() -> std::unique_ptr<IPowerManager>
{
    // Platform-specific creation using smart pointers
#ifdef __linux__
    return std::make_unique<LinuxPowerManager>();
#elif defined(_WIN32)
    return std::make_unique<WindowsPowerManager>();
#elif defined(__APPLE__)
    return std::make_unique<MacOSPowerManager>();
#else
    return nullptr; // Unsupported platform
#endif
}

} // namespace ddogreen::platform
```

## Testing Standards Integration

All code must follow these standards and be validated through:
- **Unit tests**: Test individual components with mock platform implementations
- **Integration tests**: Test platform factory and real platform interactions
- **Static analysis**: Code must pass static analysis tools checking C++ Core Guidelines compliance
- **Code coverage**: Maintain >90% code coverage as documented in `testingStandards.md`

## Guidelines Compliance Checklist

Before submitting code, ensure:
- [ ] Uses `std::span<T>` instead of pointer-length pairs
- [ ] Follows const-correctness with `span<const T>` by default
- [ ] Employs RAII for all resource management
- [ ] Uses range-based for loops where applicable
- [ ] Integrates with STL algorithms appropriately
- [ ] Provides exception safety guarantees
- [ ] Follows platform abstraction patterns
- [ ] Maintains uniform initialization syntax
- [ ] Uses `auto` for type deduction where beneficial
- [ ] Documents business logic, not obvious code
