#!/bin/bash

# Build script for ddogreen using CMake

set -e

echo "Building ddogreen - Intelligent Green Power Management"

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Please install CMake:"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Fedora/RHEL:   sudo dnf install cmake"
    exit 1
fi

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "g++ not found. Please install build tools:"
    echo "  Ubuntu/Debian: sudo apt install build-essential"
    echo "  Fedora/RHEL:   sudo dnf install gcc-c++"
    exit 1
fi

# Create and enter build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "Build completed successfully!"
echo "Executable: build/ddogreen"
echo ""
echo "To install system-wide:"
echo "  sudo make install"
echo ""
echo "To test the binary:"
echo "  sudo ./ddogreen --help"
