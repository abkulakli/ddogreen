# Development Guide - ddogreen

> **File Purpose**: Development workflows, contribution guidelines, and project processes
> **Content Focus**: How to contribute, development patterns, and project collaboration
> **Cross-Reference**: Build setup in `techContext.md`, coding rules in `codingStandards.md`

## Development Workflow

### Code Standards Compliance
- Follow guidelines in `codingStandards.md`
- Follow testing practices in `testingStandards.md`
- Use C++17 features and modern practices
- Maintain platform abstraction compliance
- Write comprehensive unit tests for new features

### Git Workflow

#### Commit Message Convention
```bash
# Format: <type>(<scope>): <description>
git commit -m "feat(linux): add TLP integration for power management"
git commit -m "fix(config): handle missing configuration file gracefully" 
git commit -m "test(config): add comprehensive validation tests"
git commit -m "docs(readme): update installation instructions"
git commit -m "refactor(platform): improve interface abstraction"
git commit -m "perf(monitor): optimize load average calculation"
```

#### Commit Types
- **feat**: New features
- **fix**: Bug fixes
- **test**: Adding or updating tests
- **docs**: Documentation changes
- **refactor**: Code refactoring without feature changes
- **perf**: Performance improvements
- **style**: Code formatting (no functional changes)
- **build**: Build system or dependency changes

### Pull Request Process
1. **Create feature branch** from `main`
2. **Implement changes** following coding standards
3. **Add/update unit tests** with good coverage
4. **Update documentation** if needed (memory bank files)
5. **Test on target platforms** (Linux minimum, Windows if possible)
6. **Submit PR** with clear description
7. **Address review feedback** promptly
8. **Squash commits** if requested before merge

### Branch Naming Convention
```bash
feature/tlp-integration
fix/config-parsing-error
test/platform-abstraction-coverage
docs/update-memory-bank
refactor/logger-simplification
```

## Configuration System Development

### Configuration File Format
```ini
# /etc/ddogreen/ddogreen.conf (Linux)
# CPU load thresholds (as decimal percentages per core)
high_performance_threshold=0.70
power_save_threshold=0.30

# Monitoring frequency in seconds
monitoring_frequency=10
```

### Configuration Parameters
- **high_performance_threshold**: Range 0.1-1.0 (CPU load per core)
- **power_save_threshold**: Range 0.05-0.9 (CPU load per core)  
- **monitoring_frequency**: Range 1-300 seconds

### Hysteresis Logic Implementation
```cpp
// Load calculation logic
double currentLoad = systemMonitor->getCurrentLoad();
double threshold = isPerformanceMode ? 
    (cores * config.getPowerSaveThreshold()) :
    (cores * config.getHighPerformanceThreshold());

// Mode switching logic
if (!isPerformanceMode && currentLoad > (cores * config.getHighPerformanceThreshold())) {
    // Switch to high performance
} else if (isPerformanceMode && currentLoad < (cores * config.getPowerSaveThreshold())) {
    // Switch to power save
}
```

Example with 8 cores:
- High performance trigger: 8 × 0.70 = 5.6 load average
- Power save trigger: 8 × 0.30 = 2.4 load average

## Contributing Areas

### High-Priority Contributions
- **macOS support**: Complete platform implementation
- **Enhanced testing**: Platform-specific test coverage
- **Performance optimization**: Memory and CPU usage improvements
- **Error handling**: Robust error recovery and logging

### Medium-Priority Contributions
- **Additional power managers**: Support for other Linux power tools
- **Enhanced monitoring**: More sophisticated load detection algorithms
- **Documentation improvements**: User guides and API documentation
- **Build system enhancements**: Packaging and distribution improvements

### Low-Priority Contributions
- **Code quality**: Static analysis and linting improvements
- **Logging enhancements**: Structured logging and metrics
- **Configuration validation**: More comprehensive config checking
- **Platform compatibility**: Support for older distributions

## Development Environment Setup

### Quick Start for Contributors
```bash
# 1. Fork the repository on GitHub
# 2. Clone your fork
git clone https://github.com/yourusername/ddogreen.git
cd ddogreen

# 3. Set up development dependencies (see techContext.md)
sudo apt install build-essential cmake tlp libgtest-dev

# 4. Build and test
cmake --preset debug
cmake --build --preset debug
ctest --preset debug-tests

# 5. Read memory bank documentation
ls memory-bank/
```

### Verification Checklist
- [ ] Code compiles without warnings
- [ ] All existing tests pass
- [ ] New tests added for new functionality
- [ ] Code follows style guidelines
- [ ] Platform abstraction maintained
- [ ] Memory bank documentation updated if needed
- [ ] No hardcoded paths or platform-specific code in application layer

## Logging Standards for Development

### Structured Logging Usage
```cpp
// Use throughout development
Logger::info("System load: {:.2f}, switching to performance mode", load);
Logger::error("Failed to set power mode: {}", error);
Logger::debug("Configuration loaded: {}Hz, {:.2f}/{:.2f} thresholds", 
              freq, lowThresh, highThresh);
Logger::warning("TLP not available, using mock power manager");
```

### Log Levels
- **ERROR**: System failures, unrecoverable errors
- **WARNING**: Recoverable issues, fallback behavior
- **INFO**: Normal operation, mode changes, startup/shutdown
- **DEBUG**: Detailed operation info, configuration details

## Platform Development Guidelines

### Adding New Platform Support
1. **Create platform directory**: `src/platform/newplatform/`
2. **Implement interfaces**:
   - `NewPlatformPowerManager.cpp`
   - `NewPlatformDaemon.cpp`
   - `NewPlatformSystemMonitor.cpp`
   - `NewPlatformUtils.cpp`
3. **Update factory**: Add platform detection in `platform_factory.cpp`
4. **Add CMake support**: Platform-specific source inclusion
5. **Add platform tests**: Create comprehensive test coverage
6. **Update documentation**: Platform-specific setup and debugging

### Platform Abstraction Rules
- **No platform ifdefs** in application layer (`main.cpp`, `daemon.cpp`)
- **All platform operations** through interfaces
- **Factory pattern** for platform-specific object creation
- **Compile-time selection** of platform implementations
- **Common error handling** patterns across platforms

## Testing Development

### Test Development Guidelines
- See `testingStandards.md` for comprehensive testing standards
- **Unit tests required** for all new functionality
- **Integration tests** for cross-component functionality
- **Platform-specific tests** for platform implementations
- **Mock objects** for external dependencies

### Test Coverage Requirements
- **Unit tests**: Minimum 80% line coverage
- **Platform implementations**: 90% coverage for each platform
- **Critical paths**: 100% coverage (privilege checking, power management)
- **Error handling**: All error paths must be tested

## Getting Help

### Resources for Contributors
1. **Read memory bank files**: Complete project context in `memory-bank/`
2. **Check existing issues**: Look for similar problems or feature requests
3. **Review coding standards**: Follow established patterns and conventions
4. **Ask questions**: Use GitHub discussions for clarification
5. **Join development**: Reference `systemPatterns.md` for architecture understanding

### Common Development Questions
- **"How does platform abstraction work?"** → See `systemPatterns.md`
- **"What are the coding standards?"** → See `codingStandards.md`
- **"How do I write tests?"** → See `testingStandards.md`
- **"What's the build process?"** → See `techContext.md`
- **"What's the project goal?"** → See `productContext.md` and `projectbrief.md`

### Code Review Focus Areas
- **Architecture compliance**: Platform abstraction maintained
- **Test coverage**: Adequate testing for new features
- **Performance impact**: Memory and CPU usage considerations
- **Error handling**: Graceful failure and recovery
- **Documentation**: Memory bank updates for significant changes
- **Cross-platform compatibility**: Works on all target platforms

This guide ensures consistent development practices and high-quality contributions to the ddogreen project.
