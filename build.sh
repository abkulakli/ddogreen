#!/bin/bash

# Build script for ddogreen
# Usage: ./build.sh [BUILD_TYPE]
# BUILD_TYPE: debug or release (default: release)

BUILD_TYPE=${1:-release}

# Validate build type
if [[ "$BUILD_TYPE" != "debug" && "$BUILD_TYPE" != "release" ]]; then
    echo "Error: Invalid build type '$BUILD_TYPE'. Must be 'debug' or 'release'."
    echo "Usage: $0 [debug|release]"
    exit 1
fi

echo "Building ddogreen with $BUILD_TYPE configuration..."

git clean -fdx
rm -rf build/
cmake --preset "$BUILD_TYPE"
cmake --build --preset "$BUILD_TYPE"

echo "Build completed successfully with $BUILD_TYPE configuration."