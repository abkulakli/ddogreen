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
  --debug              Build in debug mode
  --clean              Clean build directory before building
  --help               Show this help message

Package options:
  --package            Create DEB and RPM packages after build

Examples:
  $0                   # Standard release build (default)
  $0 --debug           # Debug build
  $0 --clean           # Clean build directory only
  $0 --package         # Build release and create DEB and RPM packages
  $0 --debug --package # Build debug and create DEB and RPM packages
  $0 --clean --package # Clean, build and create DEB and RPM packages

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

# Detect platform
PLATFORM="unknown"
case "$(uname -s)" in
    Linux*)     PLATFORM="linux";;
    Darwin*)    PLATFORM="macos";;
    CYGWIN*|MINGW*|MSYS*) PLATFORM="windows";;
    *)          PLATFORM="unknown";;
esac

# Create platform and build type specific directory
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
BUILD_DIR="$PROJECT_ROOT/build/$PLATFORM/$BUILD_TYPE_LOWER"

print_status "ddogreen build script starting..."
print_status "Platform: $PLATFORM"
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"
print_status "Project root: $PROJECT_ROOT"

# Clean build if requested
if [[ "$CLEAN_BUILD" == "ON" ]]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    print_success "Build directory cleaned successfully!"

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

# Check if packaging tools are needed
if [[ "$PACKAGE_REQUESTED" == "ON" ]]; then
    check_tool cpack || exit 1

    # Only support Linux packaging for now
    if [[ "$PLATFORM" != "linux" ]]; then
        print_error "DEB packaging is only supported on Linux platforms"
        exit 1
    fi
fi

# Build if requested
if [[ "$BUILD_REQUESTED" == "ON" ]]; then
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure CMake
    print_status "Configuring CMake..."
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$PROJECT_ROOT" || exit 1

    # Build
    print_status "Building project..."
    make -j$(nproc) || exit 1

    print_success "Build completed successfully"

    # Final build status
    print_success "Build script completed successfully!"
    print_status "Executable: $BUILD_DIR/ddogreen"

    if [[ "$BUILD_TYPE" == "Release" ]]; then
        # Show binary size
        if [[ -f "$BUILD_DIR/ddogreen" ]]; then
            SIZE=$(du -h "$BUILD_DIR/ddogreen" | cut -f1)
            print_status "Binary size: $SIZE"
        fi
    fi

    if [[ "$PACKAGE_REQUESTED" == "OFF" ]]; then
        print_status "To install: sudo make install"
        print_status "To test: sudo $BUILD_DIR/ddogreen --help"
    fi
fi

# Package if requested
if [[ "$PACKAGE_REQUESTED" == "ON" ]]; then
    # Check if build exists
    if [[ ! -f "$BUILD_DIR/ddogreen" ]]; then
        print_error "No ddogreen executable found in $BUILD_DIR"
        print_error "Build failed or does not exist"
        exit 1
    fi

    # Check if CMakeCache exists
    if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        print_error "No CMake build found in $BUILD_DIR"
        print_error "Build failed or is incomplete"
        exit 1
    fi

    # Change to build directory for packaging
    cd "$BUILD_DIR"

    # Create DEB package
    print_package_status "Creating DEB package..."
    cpack -G DEB || exit 1

    # Create RPM package
    print_package_status "Creating RPM package..."
    cpack -G RPM || exit 1

    # Create TGZ package (generic installer)
    print_package_status "Creating TGZ package (generic installer)..."
    cpack -G TGZ || exit 1

    # Find the created packages
    DEB_FILE=$(find . -name "*.deb" -type f | head -n 1)
    RPM_FILE=$(find . -name "*.rpm" -type f | head -n 1)
    TGZ_FILE=$(find . -name "*.tar.gz" -type f | head -n 1)

    if [[ -z "$DEB_FILE" ]]; then
        print_error "DEB package creation failed - no .deb file found"
        exit 1
    fi

    if [[ -z "$RPM_FILE" ]]; then
        print_error "RPM package creation failed - no .rpm file found"
        exit 1
    fi

    if [[ -z "$TGZ_FILE" ]]; then
        print_error "TGZ package creation failed - no .tar.gz file found"
        exit 1
    fi

    # Get package info
    DEB_NAME=$(basename "$DEB_FILE")
    DEB_SIZE=$(du -h "$DEB_FILE" | cut -f1)
    RPM_NAME=$(basename "$RPM_FILE")
    RPM_SIZE=$(du -h "$RPM_FILE" | cut -f1)
    TGZ_NAME=$(basename "$TGZ_FILE")
    TGZ_SIZE=$(du -h "$TGZ_FILE" | cut -f1)

    print_success "DEB, RPM, and TGZ packages created successfully!"
    echo
    print_package_status "DEB Package: $DEB_NAME"
    print_package_status "DEB Size: $DEB_SIZE"
    print_package_status "DEB Location: $BUILD_DIR/$DEB_NAME"
    echo
    print_package_status "RPM Package: $RPM_NAME"
    print_package_status "RPM Size: $RPM_SIZE"
    print_package_status "RPM Location: $BUILD_DIR/$RPM_NAME"
    echo
    print_package_status "Generic TGZ Package: $TGZ_NAME"
    print_package_status "TGZ Size: $TGZ_SIZE"
    print_package_status "TGZ Location: $BUILD_DIR/$TGZ_NAME"

    # Show DEB package contents
    print_package_status "DEB package contents:"
    dpkg-deb -c "$DEB_FILE" | head -5

    # Show DEB package info
    print_package_status "DEB package information:"
    dpkg-deb -I "$DEB_FILE" | grep -E "(Package|Version|Architecture|Description|Depends)"

    # Installation instructions
    print_success "Package creation completed successfully!"
    echo
    print_package_status "To install DEB package (Debian/Ubuntu):"
    print_package_status "  sudo dpkg -i $BUILD_DIR/$DEB_NAME"
    print_package_status "  sudo apt install -f  # Fix dependencies if needed"
    print_package_status "  (Service will be automatically installed and started)"
    echo
    print_package_status "To install RPM package (RHEL/CentOS/Fedora):"
    print_package_status "  sudo rpm -i $BUILD_DIR/$RPM_NAME"
    print_package_status "  # or: sudo dnf install $BUILD_DIR/$RPM_NAME"
    print_package_status "  (Service will be automatically installed and started)"
    echo
    print_package_status "To install generic TGZ package (All Linux distributions):"
    print_package_status "  tar -xzf $BUILD_DIR/$TGZ_NAME"
    print_package_status "  cd ddogreen-*/"
    print_package_status "  sudo ./install.sh --install"
    print_package_status "  (Works on any Linux distribution with systemd)"
    echo
    print_package_status "Service management after installation:"
    print_package_status "  sudo systemctl status ddogreen   # Check status"
    print_package_status "  sudo systemctl stop ddogreen     # Stop service"
    print_package_status "  sudo systemctl start ddogreen    # Start service"
    print_package_status "  sudo journalctl -u ddogreen -f   # View logs"
    echo
    print_package_status "To remove packages:"
    print_package_status "  sudo apt remove ddogreen     # Debian/Ubuntu"
    print_package_status "  sudo rpm -e ddogreen             # RHEL/CentOS/Fedora"
    print_package_status "  sudo ./install.sh --uninstall   # Generic TGZ"
fi
