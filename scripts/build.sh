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
RED='\033[0;31m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="debug"
CLEAN_BUILD=false
RUN_TESTS=false

# Function to show usage
show_usage() {
    echo "Usage: $0 [BUILD_TYPE] [OPTIONS]"
    echo ""
    echo "BUILD_TYPE:"
    echo "  debug     - Debug build with optimization disabled (default)"
    echo "  release   - Release build with optimizations"
    echo "  coverage  - Coverage build for testing"
    echo "  no-cache  - Debug build without ccache"
    echo ""
    echo "OPTIONS:"
    echo "  --clean        Clean build directory before building"
    echo "  --with-tests   Run tests after successful build"
    echo "  --help, -h     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                         # Debug build"
    echo "  $0 release                 # Release build"
    echo "  $0 debug --clean           # Clean debug build"
    echo "  $0 release --with-tests    # Release build with tests"
    echo "  $0 debug --clean --with-tests  # Clean debug build with tests"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        debug|release|coverage|no-cache)
            BUILD_TYPE="$1"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --with-tests)
            RUN_TESTS=true
            shift
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Error: Unknown argument '$1'${NC}"
            echo ""
            show_usage
            exit 1
            ;;
    esac
done

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

echo -e "${BLUE}Build configuration:${NC}"
echo "  Build type: $BUILD_TYPE"
echo "  Clean build: $CLEAN_BUILD"
echo "  Run tests: $RUN_TESTS"
echo ""

# Clean build if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Performing clean build...${NC}"
    rm -rf build/$BUILD_TYPE
fi

# Configure and build
echo -e "${BLUE}Configuring build...${NC}"
time cmake --preset $BUILD_TYPE

echo -e "${BLUE}Building project...${NC}"
time cmake --build --preset $BUILD_TYPE

echo -e "${GREEN}✓ Build completed successfully!${NC}"

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo ""
    echo -e "${BLUE}Running tests...${NC}"
    ctest --preset ${BUILD_TYPE}-tests --output-on-failure
    echo -e "${GREEN}✓ Tests completed successfully!${NC}"
fi

# Show updated ccache stats if available
if command -v ccache >/dev/null 2>&1; then
    echo ""
    echo -e "${BLUE}Updated ccache statistics:${NC}"
    ccache --show-stats | head -10
fi

echo ""
echo -e "${GREEN}Build artifact location: build/$BUILD_TYPE/ddogreen${NC}"
if [ "$RUN_TESTS" = false ]; then
    echo -e "${BLUE}Run tests with: ctest --preset ${BUILD_TYPE}-tests${NC}"
fi
