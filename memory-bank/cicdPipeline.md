# CI/CD Pipeline Documentation - ddogreen

## Overview

The ddogreen project uses a comprehensive GitHub Actions-based CI/CD pipeline (`unified-ci-cd.yml`) that provides automated building, testing, packaging, and releasing across multiple platforms. This pipeline ensures high quality and consistency for all releases.

## Pipeline Architecture

### Trigger Conditions
- **Push Events**: `main` and `develop` branches, version tags (`v*`)
- **Pull Requests**: Targeting `main` branch
- **Release Creation**: Automated on version tags

### Platform Support
- **Linux**: Full build, test, and packaging (DEB, RPM, TGZ)
- **Windows**: Full build, test, and packaging (ZIP)

## Pipeline Stages

### Stage 1: Build Applications

#### Linux Build (`build-linux`)
- **Runner**: `ubuntu-latest`
- **Dependencies**: `build-essential`, `cmake`
- **Configuration**: Uses CMake presets (`--preset release`)
- **Version Handling**: 
  - Tags: Extracts version from Git tag (removes `v` prefix)
  - Non-tags: Uses development version `0.0.0`
- **Artifacts**: 
  - `ddogreen` binary
  - `test_config` test executable

#### Windows Build (`build-windows`)  
- **Runner**: `windows-latest`
- **Dependencies**: MSVC
- **Configuration**: Standard CMake with Release build type
- **Version Handling**: Same logic as Linux (PowerShell implementation)
- **Artifacts**: `ddogreen.exe` binary

### Stage 2: Testing

#### Linux Testing (`test-linux`)
- **Compatibility Testing**: Both standard CMake and preset-based builds
- **Test Execution**: 
  - Standard CMake: `ctest --output-on-failure --verbose`
  - Presets: Both debug and release preset testing
- **Functional Testing**: Binary help and version commands
- **Dependencies**: Same as build stage

#### Windows Testing (`test-windows`)
- **Artifact Integration**: Downloads Windows build artifacts
- **Functional Testing**: Binary help and version commands
- **Platform**: Windows-specific PowerShell commands

### Stage 3: Package Creation

#### Linux Packaging (`package-linux`)
- **Package Types**: DEB, RPM, TGZ
- **Tool**: CPack with multiple generators
- **Version Integration**: Supports both tagged and development versions
- **Artifacts**: 
  - `ddogreen-deb`: Debian package
  - `ddogreen-rpm`: Red Hat package  
  - `ddogreen-tgz`: Generic tarball

#### Windows Packaging (`package-windows`)
- **Package Types**: ZIP
- **Custom ZIP Creation**: Includes executable, config template, installer script
- **Structure**: Standardized `share/ddogreen/` layout across platforms
- **Artifacts**:
  - `ddogreen-windows-zip`: Custom ZIP with installer

### Stage 4: Package Validation

#### Linux Package Validation (Sequential)

**DEB Validation (`validate-deb`)**
- **Installation Testing**: Full package installation via `dpkg`
- **Service Verification**: Systemd service file creation and syntax validation
- **Functional Testing**: Binary execution and version verification
- **File Layout**: Verification of `/usr/share/ddogreen/` structure
- **Removal Testing**: Clean package removal verification

**RPM Validation (`validate-rpm`)**
- **Structure Testing**: Package extraction and content verification
- **Binary Testing**: Executable functionality verification
- **Note**: Structure validation only (no installation testing due to Ubuntu runner)

**TGZ Validation (`validate-tgz`)**
- **Installation Testing**: Full installer script execution
- **Service Creation**: Systemd service installation verification
- **File Layout**: Verification of standardized structure
- **Removal Testing**: Clean uninstall via installer script

#### Windows Package Validation

**ZIP Package Validation (`validate-windows-zip`)**
- **Content Verification**: Binary, config template, installer script presence
- **Functional Testing**: Executable help/version commands
- **Installer Testing**: Script help and status functionality (non-admin)

### Stage 5: Release Creation (`release`)

#### Trigger Conditions
- **Only on version tags**: `refs/tags/v*` pattern
- **All validation passed**: Requires all build, test, and validation stages

#### Release Artifacts
- **Versioned Naming**: All packages renamed with version number
- **Package Types**:
  - `ddogreen-{VERSION}-linux.deb`
  - `ddogreen-{VERSION}-linux.rpm` 
  - `ddogreen-{VERSION}-linux.tar.gz`
  - `ddogreen-{VERSION}-windows.zip`
- **Verification**: SHA256 checksums for all packages

#### Release Documentation
- **Auto-generated Notes**: GitHub release notes from commits
- **Installation Instructions**: Platform-specific installation commands
- **Requirements**: System prerequisites for each platform
- **Service Management**: Commands for Linux (systemd) and Windows (SCM)

## Quality Assurance Features

### Comprehensive Testing
- **Build Verification**: Multiple build configurations (standard + presets)
- **Cross-Platform**: Both Linux and Windows testing
- **Package Integration**: Real installation and removal testing
- **Functional Verification**: Binary execution and basic functionality

### Package Quality
- **Structure Standardization**: Consistent `share/ddogreen/` layout
- **Service Integration**: Proper systemd/Windows service setup
- **Clean Installation**: Verified file placement and permissions
- **Clean Removal**: Verified complete uninstallation

### Version Management
- **Semantic Versioning**: Automatic version extraction from Git tags
- **Development Builds**: Clear separation with `0.0.0` version
- **Artifact Naming**: Consistent versioned package naming

## Pipeline Performance

### Parallel Execution
- **Build Stage**: Linux and Windows builds run in parallel
- **Test Stage**: Platform-specific testing in parallel
- **Package Stage**: Platform-specific packaging in parallel
- **Validation Stage**: Sequential execution for reliability

### Artifact Management
- **Upload Strategy**: Stage-specific artifact uploads
- **Download Strategy**: Dependent stages download required artifacts
- **Retention**: GitHub default artifact retention (90 days)

## Security Considerations

### Permissions
- **Contents**: Write access for release creation
- **Packages**: Read access for artifact management
- **Secrets**: Uses `RELEASE_TOKEN` for GitHub releases

### Validation Scope
- **Linux**: Full installation testing (DEB, TGZ) with service verification
- **Windows**: Structure validation only due to admin restrictions
- **RPM**: Structure validation only (package manager compatibility)

## Maintenance and Monitoring

### Update Requirements
- **Runner Images**: GitHub-managed, automatically updated
- **Dependencies**: Fixed versions where critical (CMake)
- **Actions**: Uses stable action versions (`@v4`, `@v2`)

### Failure Analysis
- **Build Failures**: CMake configuration or compilation errors
- **Test Failures**: Unit test failures or binary functionality issues
- **Package Failures**: CPack configuration or dependency issues
- **Validation Failures**: Installation, service, or functional verification issues

### Performance Metrics
- **Full Pipeline**: ~15-20 minutes for complete run
- **Build Stage**: ~3-5 minutes per platform
- **Test Stage**: ~2-3 minutes per platform
- **Package Stage**: ~2-3 minutes per platform
- **Validation Stage**: ~5-10 minutes total

## Integration with Project Architecture

### CMake Integration
- **Preset Support**: Full compatibility with CMake presets
- **Version Override**: Support for `PROJECT_VERSION_OVERRIDE`
- **Cross-Platform**: Unified build system across platforms

### Package Structure Compliance
- **FHS Standards**: Follows Filesystem Hierarchy Standard
- **Cross-Platform**: Consistent structure across all package types
- **Service Integration**: Proper systemd and Windows SCM support

### Memory Bank Integration
- **Documentation**: This file documents the complete CI/CD system
- **Quality Standards**: Aligns with coding and testing standards
- **Architecture**: Supports platform abstraction layer validation

This CI/CD pipeline ensures that every release of ddogreen is thoroughly tested, properly packaged, and ready for production deployment across all supported platforms.
