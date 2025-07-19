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
  --package            Create DEB package after build

Examples:
  $0                   # Standard release build (default)
  $0 --debug           # Debug build
  $0 --clean           # Clean build directory only
  $0 --package         # Build release and create DEB package
  $0 --debug --package # Build debug and create DEB package
  $0 --clean --package # Clean, build and create DEB package

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

    # Create the package
    print_package_status "Creating DEB package..."
    cpack -G DEB || exit 1

    # Find the created package
    PACKAGE_FILE=$(find . -name "*.deb" -type f | head -n 1)

    if [[ -z "$PACKAGE_FILE" ]]; then
        print_error "Package creation failed - no .deb file found"
        exit 1
    fi

    # Get package info
    PACKAGE_NAME=$(basename "$PACKAGE_FILE")
    PACKAGE_SIZE=$(du -h "$PACKAGE_FILE" | cut -f1)

    print_success "DEB package created successfully!"
    print_package_status "Package: $PACKAGE_NAME"
    print_package_status "Size: $PACKAGE_SIZE"
    print_package_status "Location: $BUILD_DIR/$PACKAGE_NAME"

    # Show package contents
    print_package_status "Package contents:"
    dpkg-deb -c "$PACKAGE_FILE" | head -10

    # Show package info
    print_package_status "Package information:"
    dpkg-deb -I "$PACKAGE_FILE" | grep -E "(Package|Version|Architecture|Description|Depends)"

    # Installation instructions
    print_success "Package creation completed successfully!"
    echo
    print_package_status "To install the package:"
    print_package_status "  sudo dpkg -i $BUILD_DIR/$PACKAGE_NAME"
    print_package_status "  sudo apt-get install -f  # Fix dependencies if needed"
    echo
    print_package_status "To remove the package:"
    print_package_status "  sudo apt-get remove ddogreen"
    print_package_status "  sudo apt-get purge ddogreen  # Remove all config files"
    echo
    print_package_status "To inspect the package:"
    print_package_status "  dpkg-deb -c $BUILD_DIR/$PACKAGE_NAME  # List contents"
    print_package_status "  dpkg-deb -I $BUILD_DIR/$PACKAGE_NAME  # Show package info"
fi
