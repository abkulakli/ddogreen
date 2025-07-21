#!/bin/bash

# Build and run tests for ddogreen
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building ddogreen with tests...${NC}"

# Create build directory
mkdir -p build
cd build

# Configure with tests enabled
cmake .. -DBUILD_TESTS=ON

# Build
make -j$(nproc)

echo -e "${YELLOW}Running tests...${NC}"

# Run tests with CTest
if ctest --output-on-failure; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
