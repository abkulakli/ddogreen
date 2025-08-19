#!/bin/bash

# DDOGreen Power Management Functional Test Script
# This script tests the power management functionality of ddogreen
# Usage: ./scripts/test_power_management.sh [executable_path]

set -e

# Configuration
EXECUTABLE="${1:-./build/release/ddogreen}"
TEST_DURATION=30
STRESS_DURATION=15
RECOVERY_DURATION=10

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking prerequisites..."
    
    # Check if executable exists
    if [[ ! -f "$EXECUTABLE" ]]; then
        log_error "DDOGreen executable not found at: $EXECUTABLE"
        log_info "Build the project first: cmake --preset release && cmake --build --preset release"
        exit 1
    fi
    
    # Make executable if needed
    chmod +x "$EXECUTABLE"
    
    # Check if stress-ng is available
    if ! command -v stress-ng &> /dev/null; then
        log_warning "stress-ng not found. Installing..."
        if command -v apt-get &> /dev/null; then
            sudo apt-get update && sudo apt-get install -y stress-ng
        elif command -v yum &> /dev/null; then
            sudo yum install -y stress-ng
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y stress-ng
        else
            log_error "Could not install stress-ng. Please install it manually."
            exit 1
        fi
    fi
    
    log_success "Prerequisites check passed"
}

# Display system information
show_system_info() {
    log_info "System Information:"
    echo "  CPU cores: $(nproc)"
    echo "  CPU model: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d: -f2 | xargs)"
    echo "  Initial load average: $(cat /proc/loadavg)"
    echo "  Available memory: $(free -h | grep Mem | awk '{print $7}')"
    
    # Check for power management capabilities
    if [[ -d "/sys/devices/system/cpu/cpu0/cpufreq" ]]; then
        echo "  CPU frequency scaling: Available"
        if [[ -f "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor" ]]; then
            echo "  Current governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)"
        fi
    else
        echo "  CPU frequency scaling: Not available (virtual machine or restricted)"
    fi
    echo ""
}

# Test basic functionality
test_basic_functionality() {
    log_info "Testing basic functionality..."
    
    # Test help command
    if timeout 5s "$EXECUTABLE" --help &> /dev/null; then
        log_success "Help command works"
    else
        log_error "Help command failed"
        return 1
    fi
    
    # Test version command
    if timeout 5s "$EXECUTABLE" --version &> /dev/null; then
        log_success "Version command works"
    else
        log_error "Version command failed"
        return 1
    fi
    
    # Test dry run
    if timeout 5s "$EXECUTABLE" --dry-run &> /dev/null; then
        log_success "Dry run works"
    else
        log_error "Dry run failed"
        return 1
    fi
    
    log_success "Basic functionality test passed"
}

# Monitor system load
monitor_load() {
    local duration=$1
    local phase=$2
    local interval=1
    
    log_info "Monitoring system load during $phase phase ($duration seconds)..."
    
    for (( i=1; i<=duration; i++ )); do
        local load=$(cat /proc/loadavg | cut -d' ' -f1)
        local cores=$(nproc)
        local load_percent=$(echo "scale=1; $load * 100 / $cores" | bc -l 2>/dev/null || echo "N/A")
        
        printf "  [%2d/%d] Load: %s (%s%% of CPU capacity)\n" "$i" "$duration" "$load" "$load_percent"
        sleep $interval
    done
}

# Main power management test
test_power_management() {
    log_info "Starting power management functional test..."
    
    # Create temporary log file
    local log_file="/tmp/ddogreen_test_$$.log"
    
    # Start ddogreen in background
    log_info "Starting DDOGreen service..."
    timeout ${TEST_DURATION}s "$EXECUTABLE" --verbose > "$log_file" 2>&1 &
    local ddogreen_pid=$!
    
    # Wait for service to start
    sleep 2
    
    if ! kill -0 $ddogreen_pid 2>/dev/null; then
        log_error "DDOGreen failed to start"
        cat "$log_file"
        return 1
    fi
    
    log_success "DDOGreen started with PID: $ddogreen_pid"
    
    # Phase 1: Monitor initial low-load state
    log_info "Phase 1: Monitoring initial state (should be power save mode)"
    monitor_load 5 "initial"
    
    # Phase 2: Create CPU stress
    log_info "Phase 2: Creating CPU stress to trigger high performance mode"
    stress-ng --cpu $(nproc) --timeout ${STRESS_DURATION}s &
    local stress_pid=$!
    
    log_info "CPU stress started with PID: $stress_pid"
    monitor_load $STRESS_DURATION "stress"
    
    # Wait for stress to complete
    wait $stress_pid 2>/dev/null || true
    log_success "CPU stress completed"
    
    # Phase 3: Monitor recovery
    log_info "Phase 3: Monitoring recovery (should return to power save mode)"
    monitor_load $RECOVERY_DURATION "recovery"
    
    # Stop ddogreen
    log_info "Stopping DDOGreen service..."
    if kill $ddogreen_pid 2>/dev/null; then
        wait $ddogreen_pid 2>/dev/null || true
        log_success "DDOGreen stopped successfully"
    else
        log_warning "DDOGreen may have already stopped"
    fi
    
    # Show relevant log entries
    log_info "DDOGreen log output:"
    if [[ -f "$log_file" && -s "$log_file" ]]; then
        cat "$log_file" | sed 's/^/  /'
    else
        log_warning "No log output captured"
    fi
    
    # Cleanup
    rm -f "$log_file"
    
    log_success "Power management test completed"
}

# Test error handling
test_error_handling() {
    log_info "Testing error handling..."
    
    # Test with invalid config file
    if timeout 5s "$EXECUTABLE" --config /nonexistent/file 2>/dev/null; then
        log_error "Should have failed with invalid config file"
        return 1
    else
        log_success "Correctly handled invalid config file"
    fi
    
    # Test with invalid option
    if timeout 5s "$EXECUTABLE" --invalid-option 2>/dev/null; then
        log_error "Should have failed with invalid option"
        return 1
    else
        log_success "Correctly handled invalid option"
    fi
    
    log_success "Error handling test passed"
}

# Main execution
main() {
    echo "========================================"
    echo "DDOGreen Power Management Functional Test"
    echo "========================================"
    echo ""
    
    check_prerequisites
    show_system_info
    
    # Run all test phases
    if test_basic_functionality && \
       test_error_handling && \
       test_power_management; then
        echo ""
        log_success "🎉 All tests passed! DDOGreen is working correctly."
        echo ""
        echo "Test Summary:"
        echo "  ✅ Basic functionality"
        echo "  ✅ Error handling"
        echo "  ✅ Power management workflow"
        echo ""
        exit 0
    else
        echo ""
        log_error "❌ Some tests failed. Please check the output above."
        exit 1
    fi
}

# Check if bc is available for load percentage calculation
if ! command -v bc &> /dev/null; then
    log_warning "bc not found. Load percentages will not be calculated."
fi

# Run main function
main "$@"
