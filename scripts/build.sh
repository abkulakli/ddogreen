#!/bin/bash
# DDOGreen Build Script with ccache Integration
# 
# This script demonstrates optimal build practices using ccache for energy-efficient development.
# ccache reduces compilation time and CPU energy consumption through intelligent caching.

set -e  # Exit on any error

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}DDOGreen - Energy-Efficient Build System${NC}"
echo "========================================"

# Check if ccache is available
if command -v ccache >/dev/null 2>&1; then
    echo -e "${GREEN}✓ ccache found: $(ccache --version | head -1)${NC}"
    
    # Show ccache status
    echo -e "${BLUE}ccache statistics:${NC}"
    ccache --show-stats | head -10
    echo ""
else
    echo -e "${YELLOW}⚠ ccache not found - builds will run without caching${NC}"
    echo "Install ccache for faster builds: sudo apt install ccache"
    echo ""
fi

# Build options
BUILD_TYPE=${1:-debug}
CLEAN_BUILD=${2:-false}

echo -e "${BLUE}Build configuration:${NC}"
echo "  Build type: $BUILD_TYPE"
echo "  Clean build: $CLEAN_BUILD"
echo ""

# Clean build if requested
if [ "$CLEAN_BUILD" = "clean" ] || [ "$CLEAN_BUILD" = "true" ]; then
    echo -e "${YELLOW}Performing clean build...${NC}"
    rm -rf build/$BUILD_TYPE
fi

# Configure and build
echo -e "${BLUE}Configuring build...${NC}"
time cmake --preset $BUILD_TYPE

echo -e "${BLUE}Building project...${NC}"
time cmake --build --preset $BUILD_TYPE

echo -e "${GREEN}✓ Build completed successfully!${NC}"

# Show updated ccache stats if available
if command -v ccache >/dev/null 2>&1; then
    echo ""
    echo -e "${BLUE}Updated ccache statistics:${NC}"
    ccache --show-stats | head -10
fi

echo ""
echo -e "${GREEN}Build artifact location: build/$BUILD_TYPE/ddogreen${NC}"
echo -e "${BLUE}Run tests with: ctest --preset ${BUILD_TYPE}-tests${NC}"
