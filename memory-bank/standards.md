# DDOGreen - Working Standards & Methodology

## MCP Usage Requirements
**CRITICAL**: Always use Model Context Protocol (MCP) tools when working with external libraries or documentation. This ensures accurate, up-to-date information and proper implementation patterns.

### MCP Workflow
1. **Library Resolution**: Use `mcp_context7_resolve-library-id` to find exact library identifiers
2. **Documentation Retrieval**: Use `mcp_context7_get-library-docs` for current documentation
3. **Implementation**: Apply documented patterns and best practices from MCP sources
4. **Validation**: Cross-reference implementation against MCP-provided examples

## Static Analysis Integration
**IMPLEMENTED**: Comprehensive code quality monitoring using cppcheck static analysis tool with exhaustive analysis

### Static Analysis Workflow
**Tools**: cppcheck 2.17.1 with **EXHAUSTIVE ANALYSIS ENABLED** (clang-tidy disabled per project requirements)

#### Enhanced Configuration (August 20, 2025)
**MAXIMUM COVERAGE**: All available C++ checks enabled with deepest analysis level
- **Check Level**: `--check-level=exhaustive` (deepest valueflow analysis)
- **Library Support**: `--library=posix` (POSIX API specific checks)
- **Standards**: C++20 compliance with comprehensive rule set
- **Coverage**: 179/856 available checkers active (20.9% coverage optimized for C++ projects)
- **Analysis Depth**: Exhaustive valueflow analysis for maximum issue detection

#### Build Integration Commands
```bash
# Enable enhanced static analysis during build
cmake --preset static-analysis           # Configure with exhaustive static analysis
cmake --build build/static-analysis      # Build with real-time comprehensive code analysis

# Standard development builds (analysis disabled by default)
cmake --preset debug                     # Regular debug build
cmake --build build/debug               # Build without static analysis
```

#### Advanced Configuration Details
- **Scope**: All source files in `src/` with `include/` headers
- **Standards**: C++20 compliance with all available checks
- **Integration**: Automatic execution during compilation when enabled
- **Thread Safety**: Portability checks for reentrant function usage
- **Performance**: Enhanced detection of energy-inefficient code patterns
- **Security**: Comprehensive validation including POSIX-specific security checks

#### Key Benefits
- **Maximum Detection**: Exhaustive analysis catches subtle issues missed by normal checking
- **Thread Safety**: Identifies non-reentrant function usage (e.g., localtime vs localtime_r)
- **Performance Optimization**: Identifies energy-inefficient code patterns with deeper analysis
- **Portability**: Cross-platform compatibility checks with POSIX library awareness
- **Security**: Enhanced security pattern detection for cross-platform applications
- **Sustainability Alignment**: Supports energy-efficient coding practices through automated detection

#### Example Static Analysis Output
```
/path/to/file.cpp:23:5: style: Class 'RateLimiter' has a constructor with 1 argument that is not explicit. [noExplicitConstructor]
/path/to/file.cpp:108:17: performance: Inefficient usage of string::find() in condition; string::starts_with() could be faster. [stlIfStrFind]  
/path/to/file.cpp:74:17: style: Consider using std::accumulate algorithm instead of a raw loop. [useStlAlgorithm]
```

#### Warning Resolution Achievements
**Status**: COMPREHENSIVE FIXES COMPLETED (2024-12-19)
- **Critical Syntax Error**: Fixed unmatched brace in `src/config_backup.cpp`
- **Constructor Safety**: Added `explicit` to `RateLimiter` constructor preventing implicit conversions
- **Inheritance Safety**: Added `override` specifiers to 8 virtual destructors across all platform implementations
- **Performance Optimization**: Replaced raw loop with `std::accumulate` in system monitoring algorithms
- **Code Quality**: Fixed variable scope, redeclaration, and string operation inefficiencies
- **Test Validation**: Maintained 122/122 tests passing throughout all fixes
- **Warning Reduction**: Systematically addressed 20+ critical warnings while preserving functionality

### Code Quality Standards

### Documentation Management
**CRITICAL RULE**: All documentation must follow the established memory bank structure

#### Documentation Placement Rules
- **User Information**: Add to `README.md` (installation, usage, configuration guides)
- **Developer Information**: Add to memory bank files (architecture, standards, development processes)
- **FORBIDDEN**: Creating additional `.md` files (CODING_STANDARDS.md, CONTRIBUTING.md, etc.)
- **Memory Bank Structure**: Use only `context.md`, `standards.md`, and `active.md`

**RATIONALE**: Maintains centralized, consistent documentation without file proliferation

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
- **Performance tests**: Validate system responsiveness and resource usage

### Quality Assurance Workflow
1. **Code Review**: All changes require review for energy efficiency and architecture compliance
2. **Static Analysis**: Use compiler warnings and static analysis tools
3. **Dynamic Testing**: Run full test suite on all supported platforms
4. **Performance Validation**: Monitor system resource usage and responsiveness
5. **Documentation Updates**: Keep memory bank synchronized with changes

### Naming Standards and Conventions
**ESTABLISHED**: Comprehensive naming standards for consistent branding and technical compatibility

#### Brand Names (Display/Marketing)
- **Company**: `DDOSoft` or `DDOSoft Solutions`
- **Product**: `DDOGreen`
- **Full Title**: `DDOGreen - Intelligent Green Power Management`
- **Copyright**: `Copyright (c) 2025 DDOSoft Solutions (www.ddosoft.com)`

#### Technical Names (Files/Folders/Executables)
- **Company Folders**: `ddosoft` (lowercase)
- **Product Folders**: `ddogreen` (lowercase)
- **Executables**: `ddogreen` (lowercase)
- **Services**: `ddogreen` (lowercase)
- **Configuration**: `ddogreen.conf` (lowercase)
- **URLs/Email**: `www.ddosoft.com`, `support@ddosoft.com` (lowercase)

#### Folder Structure Standards
```
Program Files/ddosoft/ddogreen/ddogreen.exe       # Windows
ProgramData/ddosoft/ddogreen/ddogreen.conf        # Windows
/usr/local/bin/ddogreen                           # Linux/macOS
/etc/ddogreen/ddogreen.conf                       # Linux/macOS
```

#### Code Naming (C++)
- **Classes**: `PascalCase` (ActivityMonitor, Config, Logger)
- **Interfaces**: `IPascalCase` (IPowerManager, ISystemMonitor)
- **Functions**: `camelCase` (getLoadAverage, setLogLevel)
- **Variables**: `camelCase` (monitoringInterval, configPath)
- **Constants**: `SCREAMING_SNAKE_CASE` (DEFAULT_INTERVAL, MAX_LOG_SIZE)
- **Files**: `snake_case` (activity_monitor.cpp, platform_utils.h)

#### Platform-Specific Rules
- **Windows Paths**: Use lowercase folders, display names for registry/services
- **Linux/Unix**: Use lowercase throughout
- **Package Names**: `ddogreen-platform` (ddogreen-linux.deb, ddogreen-windows.msi)
- **Service Display Name**: `DDOGreen - Intelligent Green Power Management`

**RATIONALE**: Display names use proper branding, technical names use lowercase for cross-platform compatibility

### Documentation and File Management Rules
**CRITICAL**: Do not create additional documentation files outside the memory bank system

#### File Creation Rules
- **User Information**: Add to `README.md` (installation, usage, configuration)
- **Developer Information**: Add to memory bank files (architecture, standards, processes)
- **No Additional MD Files**: All documentation must go into existing structure
- **Memory Bank Only**: Technical standards, workflows, and development guidelines stay in memory bank

#### Approved Documentation Structure
```
README.md                    # User-facing information only
memory-bank/
├── context.md              # Project essentials and architecture
├── standards.md            # Working methodology and standards (THIS FILE)
└── active.md              # Current focus and priorities
```

**RULE**: Never create separate documentation files like CODING_STANDARDS.md, CONTRIBUTING.md, etc.

### Response Quality Guidelines
- **Always read memory bank first**: Understand current project state before making changes
- **Use tools effectively**: Prefer appropriate tools over manual work
- **Maintain consistency**: Follow established patterns and naming conventions
- **Document decisions**: Update memory bank with significant architectural changes
- **Energy consciousness**: Consider energy impact of all code changes

### Communication Standards
**CRITICAL**: Direct, honest technical communication without artificial pleasantries or excessive accommodation

#### Communication Philosophy
- **Complete Technical Assessment**: Always present BOTH advantages AND disadvantages of any solution
- **No Sugar-Coating**: Present technical trade-offs, limitations, and potential issues directly
- **User Decision Authority**: Acknowledge that the user makes the final decisions on implementation approach
- **Factual Analysis**: Focus on objective technical analysis rather than persuasive language
- **Problem Disclosure**: Always identify potential issues, edge cases, and maintenance implications upfront

#### Response Structure
```
✅ **Advantages:**
- Clear technical benefits
- Performance improvements
- Maintainability gains

⚠️ **Disadvantages:**
- Implementation complexity
- Resource requirements
- Potential risks or limitations

🔧 **Implementation Considerations:**
- Required changes and effort
- Dependencies and prerequisites
- Alternative approaches
```

**RATIONALE**: Technical decisions require complete information. The user needs honest assessment of all aspects to make informed choices about implementation direction.
