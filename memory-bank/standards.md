# DDOGreen - Working Standards & Methodology

## MCP Usage Requirements
**CRITICAL**: Always use Model Context Protocol (MCP) tools when working with external libraries or documentation. This ensures accurate, up-to-date information and proper implementation patterns.

### MCP Workflow
1. **Library Resolution**: Use `mcp_context7_resolve-library-id` to find exact library identifiers
2. **Documentation Retrieval**: Use `mcp_context7_get-library-docs` for current documentation
3. **Implementation**: Apply documented patterns and best practices from MCP sources
4. **Validation**: Cross-reference implementation against MCP-provided examples

## Core Development Principles

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
```

### Architecture Compliance
- **NO #ifdef**: Platform-specific code must use the platform abstraction layer
- **Single Point of Truth**: Platform differences resolved through factory pattern
- **Interface Segregation**: Clean separation between platform-agnostic core and platform implementations
- **Dependency Injection**: Platform implementations injected through factory pattern

### C++ Core Guidelines Integration
This project follows the **ISO C++ Core Guidelines** for modern C++ development with emphasis on energy-efficient patterns:

#### Resource Management
- **RAII everywhere**: All resources managed through constructors/destructors
- **Smart pointers**: Use `std::unique_ptr` and `std::shared_ptr` appropriately
- **Move semantics**: Implement move constructors/assignment for efficiency
- **Span usage**: Use `std::span` for array-like parameter passing

#### Modern C++ Features (C++20)
- **Concepts**: Use concepts for template constraints
- **Ranges**: Prefer range-based algorithms over traditional iterators
- **Modules**: Use modules when available instead of headers
- **Coroutines**: Use for asynchronous operations when beneficial

### Testing Standards
- **100% test coverage**: All production code must have corresponding tests
- **Test-driven development**: Write tests before implementation when possible
- **Mock objects**: Use proper mocking for platform abstractions
- **Integration tests**: Verify cross-platform behavior
- **Performance tests**: Validate energy efficiency claims

### Quality Assurance Workflow
1. **Code Review**: All changes require review for energy efficiency and architecture compliance
2. **Static Analysis**: Use compiler warnings and static analysis tools
3. **Dynamic Testing**: Run full test suite on all supported platforms
4. **Performance Validation**: Measure energy consumption impact
5. **Documentation Updates**: Keep memory bank synchronized with changes

### Response Quality Guidelines
- **Always read memory bank first**: Understand current project state before making changes
- **Use tools effectively**: Prefer appropriate tools over manual work
- **Maintain consistency**: Follow established patterns and naming conventions
- **Document decisions**: Update memory bank with significant architectural changes
- **Energy consciousness**: Consider energy impact of all code changes
