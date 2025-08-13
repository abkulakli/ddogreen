# DDOGreen Coding Standards

## Core Principles

This document establishes the coding standards for the ddogreen power management service, incorporating ISO C++ Core Guidelines for modern, safe, and **energy-efficient** C++ development. These standards ensure code quality, maintainability, consistency, and environmental sustainability through efficient resource usage.

### Energy Efficiency First

**FUNDAMENTAL PRINCIPLE**: Write code that minimizes computational and energy overhead while maintaining functionality and readability.

#### Energy-Efficient Coding Philosophy
- **Algorithmic efficiency over micro-optimizations**: Choose proper algorithms and data structures first
- **Minimize memory allocations**: Prefer stack allocation, reuse containers, avoid unnecessary copying
- **Efficient I/O operations**: Batch operations, minimize system calls, use appropriate buffer sizes
- **Sleep appropriately**: Allow CPU to enter lower power states during idle periods
- **Avoid busy waiting**: Use event-driven patterns and blocking operations instead of polling
- **Right-sized data types**: Use appropriate data types (don't use `int64_t` when `int32_t` suffices)
- **Compile-time computation**: Use `constexpr` to move computation from runtime to compile-time

#### Energy-Efficient Code Examples
```cpp
// ENERGY EFFICIENT: Move computation to compile-time
constexpr auto MONITORING_INTERVALS = std::array{1, 5, 10, 30, 60, 300};
constexpr auto DEFAULT_INTERVAL = 5;

// ENERGY EFFICIENT: Prefer stack allocation and reuse
void process_load_history(std::vector<double>& history_buffer) {
    history_buffer.clear(); // Reuse existing capacity
    // Process using existing buffer...
}

// ENERGY EFFICIENT: Use appropriate sleep instead of busy waiting
void monitoring_loop() {
    while (running) {
        collect_metrics();
        std::this_thread::sleep_for(std::chrono::seconds(monitoring_interval));
    }
}

// ENERGY EFFICIENT: Batch I/O operations
void flush_logs_batch(std::span<const LogEntry> entries) {
    std::string batch_output;
    batch_output.reserve(entries.size() * 100); // Pre-allocate
    for (const auto& entry : entries) {
        batch_output += format_log_entry(entry);
    }
    write_to_file(batch_output); // Single I/O operation
}
```

### Zero Warning Policy - Fix Code, Never Disable Warnings

**FUNDAMENTAL RULE**: All compiler warnings must be addressed through proper code fixes, never through warning suppression.

#### Warning Management Philosophy
- **Enable ALL warnings**: Use comprehensive warning flags (-Wall, -Wextra, -Wpedantic, etc.)
- **Fix underlying issues**: Address the root cause that triggers warnings
- **Never suppress project warnings**: No `-Wno-*` flags for project code
- **Proper code patterns**: Use `[[maybe_unused]]`, epsilon-based float comparisons, explicit casts
- **External library exceptions**: Only suppress warnings from third-party code we cannot modify
- **Zero tolerance**: Build must produce zero warnings from project source files

#### Warning Resolution Examples
```cpp
// WRONG: Suppressing warnings
#pragma GCC diagnostic ignored "-Wunused-parameter"
void func(int unused_param) { /* ... */ }

// RIGHT: Proper annotation
void func([[maybe_unused]] int param) { /* ... */ }

// WRONG: Direct float comparison
if (value == 0.0) { /* ... */ }

// RIGHT: Epsilon-based comparison
if (std::fabs(value - 0.0) < std::numeric_limits<double>::epsilon()) { /* ... */ }

// WRONG: Shadowing variables
void func() {
    auto time_t = get_time(); // Shadows global time_t
}

// RIGHT: Clear naming
void func() {
    auto time_val = get_time(); // No shadowing
}
```

### Architecture Compliance

- **NO #ifdef**: Platform-specific code must use the platform abstraction layer
- **Single Point of Truth**: Platform differences resolved through factory pattern
- **Interface Segregation**: Clean separation between platform-agnostic core and platform implementations
- **Dependency Injection**: Platform implementations injected through factory pattern

### C++ Core Guidelines Integration with Energy Efficiency

This project follows the **ISO C++ Core Guidelines** for modern C++ development with emphasis on energy-efficient patterns. Key principles include:

#### Resource Management and Energy Efficiency
- **RAII (Resource Acquisition Is Initialization)**: All resources managed through constructor/destructor pairs
- **Prefer `std::span<T>` over pointer-length pairs**: Bounds-aware views for contiguous sequences with zero overhead
- **Use `span<const T>` by default**: Read-only access unless modification explicitly required
- **Avoid raw pointers for ownership**: Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- **Exception safety**: All operations provide basic or strong exception safety guarantee
- **Memory pool patterns**: Reuse containers and pre-allocate when size is predictable
- **Move semantics**: Prefer moving over copying for expensive objects

#### Energy-Conscious Interface Design
- **Pass `span<T>` by value**: Treat spans like lightweight pointer ranges (zero-copy views)
- **Const-correctness**: Use `span<const T>` for read-only data, `const span<T>` for immutable spans
- **Prefer range-based for loops**: Zero-overhead iteration with automatic bounds checking
- **Use STL algorithms with spans**: Direct integration with `std::find_if`, `std::ranges`, etc.
- **Avoid unnecessary string creation**: Use `string_view` for read-only string operations
- **Minimize function call overhead**: Use `inline` for small, frequently called functions

#### Modern C++ Practices for Efficiency
- **Use `auto` for type deduction**: Improve maintainability and readability
- **Prefer `constexpr` for compile-time constants**: Enable compile-time optimization and reduce runtime work
- **Use uniform initialization**: `{}` syntax for consistent initialization
- **Range-based operations**: Leverage spans with STL algorithms for safe, efficient code
- **Template metaprogramming**: Move complexity to compile-time when beneficial
- **Reserve container capacity**: Pre-allocate when final size is known or estimable

#### Energy-Efficient Data Structures and Algorithms
```cpp
// EFFICIENT: Use appropriate containers for access patterns
std::array<double, 60> recent_loads; // Fixed-size, stack-allocated for small, known collections
std::vector<SystemEvent> events;     // Reserve capacity for dynamic collections
events.reserve(expected_size);

// EFFICIENT: Use algorithms that minimize comparisons and data movement
std::ranges::sort(data);  // Optimized sort implementation
auto result = std::ranges::binary_search(sorted_data, target); // O(log n) search

// EFFICIENT: Minimize string operations
std::string build_status_message(std::string_view prefix, int value) {
    std::string result;
    result.reserve(prefix.size() + 20); // Pre-allocate reasonable size
    result += prefix;
    result += std::to_string(value);
    return result; // Move semantics applied automatically
}
```

### Minimal Documentation Philosophy with Efficiency Focus

- Code should be **self-documenting** through clear naming and structure
- Comments only when **business logic or energy trade-offs are non-obvious**
- **No redundant documentation** (avoid stating the obvious)
- Focus on **why** not **what** in comments, especially for performance/energy decisions
- **Document energy-conscious choices**: Explain algorithmic decisions that favor efficiency
- **Update documentation with code changes** or remove outdated docs

#### Energy-Efficiency Documentation Examples
```cpp
// Document energy trade-offs when not obvious
class LoadMonitor {
private:
    // Using circular buffer to avoid memory allocations during monitoring
    // Trade-off: Small fixed memory usage vs. dynamic allocation overhead
    std::array<double, 300> load_history{};
    size_t current_index = 0;
    
    // Cache last calculated average to avoid repeated computation
    // Updated only when new data arrives, not on every query
    mutable std::optional<double> cached_average;
};

// Document algorithmic choices for energy efficiency
auto calculate_load_average(std::span<const double> samples) -> double {
    // Using Kahan summation for numerical stability without extra iterations
    // Single-pass algorithm to minimize CPU cycles and cache misses
    double sum = 0.0;
    double compensation = 0.0;
    
    for (const auto sample : samples) {
        const auto adjusted = sample - compensation;
        const auto new_sum = sum + adjusted;
        compensation = (new_sum - sum) - adjusted;
        sum = new_sum;
    }
    
    return sum / static_cast<double>(samples.size());
}
```

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
- [ ] Documents business logic and energy trade-offs, not obvious code
- [ ] **Energy Efficiency**: Minimizes unnecessary allocations and computations
- [ ] **Energy Efficiency**: Uses appropriate sleep/blocking instead of busy waiting
- [ ] **Energy Efficiency**: Chooses algorithms for efficiency, not micro-optimizations
- [ ] **Energy Efficiency**: Pre-allocates containers when size is predictable
- [ ] **Energy Efficiency**: Uses `constexpr` for compile-time computation when possible

## Energy-Efficient Coding Patterns

### Memory Management for Energy Efficiency

```cpp
// GOOD: Reuse containers and pre-allocate capacity
class SystemMonitor {
private:
    std::vector<MetricSample> sample_buffer;
    
public:
    void collect_samples(size_t expected_count) {
        sample_buffer.clear(); // Keeps capacity
        sample_buffer.reserve(expected_count); // Pre-allocate
        
        // Collect samples without reallocations...
    }
};

// GOOD: Use appropriate data types
struct LoadAverage {
    float one_minute;   // float sufficient for load averages
    float five_minute;
    float fifteen_minute;
    std::uint32_t timestamp; // uint32_t sufficient for timestamps
};

// AVOID: Unnecessary large types
struct LoadAverageInefficient {
    double one_minute;      // double when float suffices
    double five_minute;
    double fifteen_minute;
    std::uint64_t timestamp; // uint64_t when uint32_t suffices
};
```

### I/O and System Call Efficiency

```cpp
// GOOD: Batch operations to reduce system calls
class Logger {
private:
    std::vector<std::string> pending_logs;
    static constexpr size_t BATCH_SIZE = 10;
    
public:
    void log(std::string_view message) {
        pending_logs.emplace_back(message);
        if (pending_logs.size() >= BATCH_SIZE) {
            flush_batch();
        }
    }
    
private:
    void flush_batch() {
        std::string batch_content;
        for (const auto& log : pending_logs) {
            batch_content += log + '\n';
        }
        write_to_file(batch_content); // Single I/O operation
        pending_logs.clear();
    }
};

// GOOD: Use appropriate buffer sizes for file operations
auto read_configuration_file(const std::filesystem::path& path) -> std::string {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    
    // Get file size and pre-allocate string
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string content(size, '\0');
    file.read(content.data(), size); // Single read operation
    return content;
}
```

### CPU-Friendly Patterns

```cpp
// GOOD: Use blocking operations instead of polling
class EventWatcher {
public:
    void wait_for_events() {
        while (running) {
            if (auto event = event_queue.pop_blocking()) { // Blocks until event
                process_event(*event);
            }
        }
        // CPU can enter low-power state during blocking
    }
};

// AVOID: Busy waiting patterns
class EventWatcherInefficient {
public:
    void wait_for_events() {
        while (running) {
            if (auto event = event_queue.try_pop()) { // Non-blocking
                process_event(*event);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Wasteful
        }
        // CPU constantly waking up every millisecond
    }
};

// GOOD: Compile-time computation reduces runtime work
template<size_t Seconds>
constexpr auto seconds_to_microseconds() -> std::uint64_t {
    return Seconds * 1'000'000ULL;
}

// Usage: computed at compile-time
constexpr auto FIVE_SECONDS_US = seconds_to_microseconds<5>();
```

### Cache-Friendly Data Layout

```cpp
// GOOD: Structure of Arrays for better cache utilization
class MetricsCollection {
private:
    std::vector<float> cpu_loads;        // All CPU data together
    std::vector<float> memory_usage;     // All memory data together
    std::vector<std::uint32_t> timestamps; // All timestamps together
    
public:
    void process_cpu_data() {
        // Processes contiguous memory, better cache utilization
        for (const auto load : cpu_loads) {
            // Process CPU loads...
        }
    }
};

// AVOID: Array of Structures can cause cache misses
struct MetricSample {
    float cpu_load;
    float memory_usage;
    std::uint32_t timestamp;
    char padding[100]; // Imagine additional data that's not always needed
};

class MetricsCollectionInefficient {
private:
    std::vector<MetricSample> samples; // Memory scattered with unused data
    
public:
    void process_cpu_data() {
        // Loads entire structures just to access cpu_load
        for (const auto& sample : samples) {
            // Process only sample.cpu_load, but loads all fields
        }
    }
};
```

### Energy-Conscious Error Handling

```cpp
// GOOD: Early returns reduce unnecessary computation
auto validate_configuration(const Config& config) -> std::optional<std::string> {
    if (config.monitoring_interval < 1) {
        return "Monitoring interval too small"; // Early return
    }
    
    if (config.monitoring_interval > 300) {
        return "Monitoring interval too large"; // Early return
    }
    
    // Only perform expensive validation if basic checks pass
    if (!perform_detailed_validation(config)) {
        return "Detailed validation failed";
    }
    
    return std::nullopt; // All good
}

// GOOD: Use RAII for automatic cleanup, reducing manual resource management
class SystemResourceMonitor {
private:
    std::unique_ptr<FILE, decltype(&fclose)> proc_file{nullptr, &fclose};
    
public:
    SystemResourceMonitor() {
        proc_file.reset(fopen("/proc/loadavg", "r"));
        // File automatically closed when object destroyed
    }
    
    auto read_load() -> std::optional<float> {
        if (!proc_file) return std::nullopt;
        
        float load;
        if (fscanf(proc_file.get(), "%f", &load) == 1) {
            rewind(proc_file.get()); // Reset for next read
            return load;
        }
        return std::nullopt;
    }
    // Automatic cleanup via RAII - no manual resource management
};
```
