#!/bin/bash

# ddogreen build and package script
# Enhanced build system and DEB packaging for the Intelligent Green Power Management daemon

set -e  # Exit on any error

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=OFF
BUILD_REQUESTED=OFF
PACKAGE_REQUESTED=OFF

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

print_package_status() {
    echo -e "${BLUE}[PACKAGE]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Usage function
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build options:
  --debug              Build in debug mode (both platforms)
  --clean              Clean build directories before building
  --help               Show this help message

Package options:
  --package            Create packages after build (all platforms)

Examples:
  $0                   # Build Linux and Windows release binaries
  $0 --debug           # Build Linux and Windows debug binaries
  $0 --clean           # Clean all build directories only
  $0 --package         # Build release binaries and create all packages
  $0 --debug --package # Build debug binaries and create all packages
  $0 --clean --package # Clean, build and create all packages

Output:
  Linux: DEB, RPM, TGZ packages + native binary
  Windows: NSIS installer, ZIP package + .exe binary

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            BUILD_REQUESTED=ON
            shift
            ;;
        --clean)
            CLEAN_BUILD=ON
            shift
            ;;
        --package)
            PACKAGE_REQUESTED=ON
            BUILD_REQUESTED=ON
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# If no explicit build or package options, default to build
if [[ "$BUILD_REQUESTED" == "OFF" && "$PACKAGE_REQUESTED" == "OFF" && "$CLEAN_BUILD" == "OFF" ]]; then
    BUILD_REQUESTED=ON
fi

# Project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Create platform and build type specific directories
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
LINUX_BUILD_DIR="$PROJECT_ROOT/build/linux/$BUILD_TYPE_LOWER"
WINDOWS_BUILD_DIR="$PROJECT_ROOT/build/windows/$BUILD_TYPE_LOWER"

print_status "ddogreen build script starting..."
print_status "Build type: $BUILD_TYPE"
print_status "Linux build directory: $LINUX_BUILD_DIR"
print_status "Windows build directory: $WINDOWS_BUILD_DIR"
print_status "Project root: $PROJECT_ROOT"

# Clean build if requested
if [[ "$CLEAN_BUILD" == "ON" ]]; then
    print_status "Cleaning build directories..."
    rm -rf "$LINUX_BUILD_DIR"
    rm -rf "$WINDOWS_BUILD_DIR"
    print_success "Build directories cleaned successfully!"

    # If no build was explicitly requested, exit after cleaning
    if [[ "$BUILD_REQUESTED" == "OFF" && "$PACKAGE_REQUESTED" == "OFF" ]]; then
        exit 0
    fi
fi

# Check for required tools
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        print_error "$1 is required but not installed"
        return 1
    fi
    return 0
}

print_status "Checking required tools..."
check_tool cmake || exit 1
check_tool make || exit 1

# Check cross-compilation requirements for Windows
print_status "Checking Windows cross-compilation requirements..."
check_tool x86_64-w64-mingw32-gcc || {
    print_error "MinGW-w64 cross-compiler is required for Windows builds"
    print_error "Install with: sudo apt install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64"
    exit 1
}

# Check if packaging tools are needed
if [[ "$PACKAGE_REQUESTED" == "ON" ]]; then
    check_tool cpack || exit 1
    
    check_tool makensis || {
        print_error "NSIS is required for Windows installer creation"
        print_error "Install with: sudo apt install nsis"
        exit 1
    }
fi

# Build function for a specific platform
build_platform() {
    local platform=$1
    local build_dir=$2
    local toolchain_args=$3
    
    print_status "Building for $platform..."
    
    # Create build directory
    mkdir -p "$build_dir"
    cd "$build_dir"

    # Configure CMake
    print_status "Configuring CMake for $platform..."
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE $toolchain_args"
    
    cmake $CMAKE_ARGS "$PROJECT_ROOT" || {
        print_error "CMake configuration failed for $platform"
        return 1
    }

    # Build
    print_status "Building $platform project..."
    make -j$(nproc) || {
        print_error "Build failed for $platform"
        return 1
    }

    print_success "$platform build completed successfully"
    
    # Show build results
    if [[ "$platform" == "Windows" ]]; then
        if [[ -f "$build_dir/ddogreen.exe" ]]; then
            SIZE=$(du -h "$build_dir/ddogreen.exe" | cut -f1)
            print_status "Windows executable: $build_dir/ddogreen.exe ($SIZE)"
        fi
    else
        if [[ -f "$build_dir/ddogreen" ]]; then
            SIZE=$(du -h "$build_dir/ddogreen" | cut -f1)
            print_status "Linux executable: $build_dir/ddogreen ($SIZE)"
        fi
    fi
    
    return 0
}

# Build if requested
if [[ "$BUILD_REQUESTED" == "ON" ]]; then
    # Build Linux version
    build_platform "Linux" "$LINUX_BUILD_DIR" "" || exit 1
    
    # Build Windows version (cross-compiled)
    build_platform "Windows" "$WINDOWS_BUILD_DIR" "-DCMAKE_TOOLCHAIN_FILE=$PROJECT_ROOT/packaging/windows/mingw-toolchain.cmake -DCMAKE_SYSTEM_NAME=Windows" || exit 1

    print_success "All builds completed successfully!"

    if [[ "$PACKAGE_REQUESTED" == "OFF" ]]; then
        print_status "Build summary:"
        print_status "  Linux binary ready for installation: sudo make install (from $LINUX_BUILD_DIR)"
        print_status "  Windows binary ready for deployment: $WINDOWS_BUILD_DIR/ddogreen.exe"
        print_status "  Note: Windows binary should be run on Windows system for full functionality"
    fi
fi

# Package function for a specific platform
package_platform() {
    local platform=$1
    local build_dir=$2
    local executable=$3
    
    print_package_status "Creating $platform packages..."
    
    # Check if build exists
    if [[ ! -f "$build_dir/$executable" ]]; then
        print_error "No $executable found in $build_dir"
        print_error "$platform build failed or does not exist"
        return 1
    fi

    # Check if CMakeCache exists
    if [[ ! -f "$build_dir/CMakeCache.txt" ]]; then
        print_error "No CMake build found in $build_dir"
        print_error "$platform build failed or is incomplete"
        return 1
    fi

    # Change to build directory for packaging
    cd "$build_dir"

    if [[ "$platform" == "Windows" ]]; then
        # Windows packaging
        print_package_status "Creating Windows installer (NSIS)..."
        cpack -G NSIS || return 1

        print_package_status "Creating Windows ZIP package..."
        cpack -G ZIP || return 1

        # Find the created packages
        INSTALLER_FILE=$(find . -name "*-windows.exe" -type f | head -n 1)
        ZIP_FILE=$(find . -name "*.zip" -type f | head -n 1)

        if [[ -z "$INSTALLER_FILE" ]]; then
            print_error "Windows installer creation failed - no .exe installer found"
            return 1
        fi

        if [[ -z "$ZIP_FILE" ]]; then
            print_error "ZIP package creation failed - no .zip file found"
            return 1
        fi

        # Get package info
        INSTALLER_NAME=$(basename "$INSTALLER_FILE")
        INSTALLER_SIZE=$(du -h "$INSTALLER_FILE" | cut -f1)
        ZIP_NAME=$(basename "$ZIP_FILE")
        ZIP_SIZE=$(du -h "$ZIP_FILE" | cut -f1)

        print_success "Windows packages created successfully!"
        echo
        print_package_status "Windows Installer: $INSTALLER_NAME ($INSTALLER_SIZE)"
        print_package_status "  Location: $build_dir/$INSTALLER_NAME"
        print_package_status "ZIP Package: $ZIP_NAME ($ZIP_SIZE)"
        print_package_status "  Location: $build_dir/$ZIP_NAME"

    else
        # Linux packaging
        print_package_status "Creating Linux DEB package..."
        cpack -G DEB || return 1

        print_package_status "Creating Linux RPM package..."
        cpack -G RPM || return 1

        print_package_status "Creating Linux TGZ package..."
        cpack -G TGZ || return 1

        # Find the created packages
        DEB_FILE=$(find . -name "*.deb" -type f | head -n 1)
        RPM_FILE=$(find . -name "*.rpm" -type f | head -n 1)
        TGZ_FILE=$(find . -name "*.tar.gz" -type f | head -n 1)

        if [[ -z "$DEB_FILE" || -z "$RPM_FILE" || -z "$TGZ_FILE" ]]; then
            print_error "Linux package creation failed - missing packages"
            return 1
        fi

        # Get package info
        DEB_NAME=$(basename "$DEB_FILE")
        DEB_SIZE=$(du -h "$DEB_FILE" | cut -f1)
        RPM_NAME=$(basename "$RPM_FILE")
        RPM_SIZE=$(du -h "$RPM_FILE" | cut -f1)
        TGZ_NAME=$(basename "$TGZ_FILE")
        TGZ_SIZE=$(du -h "$TGZ_FILE" | cut -f1)

        print_success "Linux packages created successfully!"
        echo
        print_package_status "DEB Package: $DEB_NAME ($DEB_SIZE)"
        print_package_status "  Location: $build_dir/$DEB_NAME"
        print_package_status "RPM Package: $RPM_NAME ($RPM_SIZE)"
        print_package_status "  Location: $build_dir/$RPM_NAME"
        print_package_status "TGZ Package: $TGZ_NAME ($TGZ_SIZE)"
        print_package_status "  Location: $build_dir/$TGZ_NAME"
    fi
    
    return 0
}

# Package if requested
if [[ "$PACKAGE_REQUESTED" == "ON" ]]; then
    # Package Linux version
    package_platform "Linux" "$LINUX_BUILD_DIR" "ddogreen" || exit 1
    
    # Package Windows version
    package_platform "Windows" "$WINDOWS_BUILD_DIR" "ddogreen.exe" || exit 1

    # Installation instructions
    print_success "All packages created successfully!"
    echo
    print_package_status "=== INSTALLATION INSTRUCTIONS ==="
    echo
    print_package_status "Linux Installation (Debian/Ubuntu):"
    print_package_status "  sudo dpkg -i $LINUX_BUILD_DIR/ddogreen-linux.deb"
    print_package_status "  sudo apt install -f  # Fix dependencies if needed"
    echo
    print_package_status "Linux Installation (RHEL/CentOS/Fedora):"
    print_package_status "  sudo rpm -i $LINUX_BUILD_DIR/ddogreen-linux.rpm"
    echo
    print_package_status "Linux Installation (Generic):"
    print_package_status "  tar -xzf $LINUX_BUILD_DIR/ddogreen-linux.tar.gz"
    print_package_status "  cd ddogreen-*/ && sudo ./install.sh --install"
    echo
    print_package_status "Windows Installation:"
    print_package_status "  1. Transfer installer to Windows system"
    print_package_status "  2. Run installer as Administrator"
    print_package_status "  3. Service will be automatically configured"
    echo
    print_package_status "Windows Alternative (ZIP):"
    print_package_status "  1. Extract ZIP on Windows system"
    print_package_status "  2. Run as Administrator: ddogreen.exe --install-service"
    print_package_status "  3. Start service: sc start ddogreen"
fi
