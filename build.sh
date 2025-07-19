#!/bin/bash

# ddogreen build script
# Enhanced build system for the Intelligent Green Power Management daemon

set -e  # Exit on any error

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=OFF
BUILD_REQUESTED=OFF

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
  --release            Build in release mode (default)
  --clean              Clean build directory before building
  --help               Show this help message

Examples:
  $0                   # Standard release build
  $0 --debug           # Debug build
  $0 --clean           # Clean build directory only
  $0 --clean --release # Clean then build release
  $0 --clean --debug   # Clean then build debug

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
        --release)
            BUILD_TYPE="Release"
            BUILD_REQUESTED=ON
            shift
            ;;
        --clean)
            CLEAN_BUILD=ON
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
    if [[ "$BUILD_REQUESTED" == "OFF" ]]; then
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

# Final status
print_success "Build script completed successfully!"
print_status "Executable: $BUILD_DIR/ddogreen"

if [[ "$BUILD_TYPE" == "Release" ]]; then
    # Show binary size
    if [[ -f "$BUILD_DIR/ddogreen" ]]; then
        SIZE=$(du -h "$BUILD_DIR/ddogreen" | cut -f1)
        print_status "Binary size: $SIZE"
    fi
fi

print_status "To install: sudo make install"
print_status "To test: sudo $BUILD_DIR/ddogreen --help"
