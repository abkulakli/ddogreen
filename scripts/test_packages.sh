#!/bin/bash
# Package testing script for ddogreen
# Tests DEB, RPM, and TGZ package installation/uninstallation

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
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

# Check if running as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to clean up any existing installation
cleanup_installation() {
    print_info "Cleaning up any existing ddogreen installation..."
    
    # Stop service if running
    systemctl stop ddogreen 2>/dev/null || true
    systemctl disable ddogreen 2>/dev/null || true
    
    # Remove package if installed via package manager
    apt-get remove -y ddogreen 2>/dev/null || true
    
    # Clean up files
    rm -rf /etc/ddogreen
    rm -f /usr/local/bin/ddogreen
    rm -f /etc/systemd/system/ddogreen.service
    rm -f /var/log/ddogreen.log
    rm -f /run/ddogreen.pid
    
    # Remove share directory if exists
    rm -rf /usr/share/ddogreen
    
    systemctl daemon-reload || true
    
    print_success "Cleanup completed"
}

# Function to verify package contents
verify_package_contents() {
    local package_type="$1"
    local package_file="$2"
    
    print_info "Verifying $package_type package contents..."
    
    case "$package_type" in
        "DEB")
            # Check DEB package contents
            if ! dpkg-deb -c "$package_file" | grep -q "ddogreen$"; then
                print_error "DEB package does not contain ddogreen binary"
                return 1
            fi
            if ! dpkg-deb -c "$package_file" | grep -q "ddogreen.conf.default"; then
                print_error "DEB package does not contain configuration template"
                return 1
            fi
            if ! dpkg-deb -c "$package_file" | grep -q "share/ddogreen"; then
                print_error "DEB package does not use standard FHS path for config"
                return 1
            fi
            ;;
        "TGZ")
            # Check TGZ package contents
            if ! tar -tzf "$package_file" | grep -q "ddogreen$"; then
                print_error "TGZ package does not contain ddogreen binary"
                return 1
            fi
            if ! tar -tzf "$package_file" | grep -q "ddogreen.conf.default"; then
                print_error "TGZ package does not contain configuration template"
                return 1
            fi
            if ! tar -tzf "$package_file" | grep -q "installer.sh"; then
                print_error "TGZ package does not contain installer script"
                return 1
            fi
            ;;
        "RPM")
            # Check RPM package contents (if rpm2cpio is available)
            if command -v rpm2cpio >/dev/null && command -v cpio >/dev/null; then
                local temp_dir=$(mktemp -d)
                cd "$temp_dir"
                rpm2cpio "$package_file" | cpio -idmv >/dev/null 2>&1
                if ! find . -name "ddogreen" -type f | grep -q ddogreen; then
                    print_error "RPM package does not contain ddogreen binary"
                    cd - && rm -rf "$temp_dir"
                    return 1
                fi
                if ! find . -name "ddogreen.conf.default" -type f | grep -q ddogreen.conf.default; then
                    print_error "RPM package does not contain configuration template"
                    cd - && rm -rf "$temp_dir"
                    return 1
                fi
                cd - && rm -rf "$temp_dir"
            else
                print_warning "rpm2cpio/cpio not available, skipping RPM content verification"
            fi
            ;;
    esac
    
    print_success "$package_type package contents verified"
}

# Function to test service functionality
test_service_functionality() {
    print_info "Testing service functionality..."
    
    # Check if binary exists and is executable
    if [[ ! -x "/usr/local/bin/ddogreen" ]]; then
        print_error "ddogreen binary not found or not executable"
        return 1
    fi
    
    # Test binary help functionality
    if ! /usr/local/bin/ddogreen --help >/dev/null; then
        print_error "ddogreen --help failed"
        return 1
    fi
    
    # Test binary version functionality
    if ! /usr/local/bin/ddogreen --version >/dev/null; then
        print_error "ddogreen --version failed"
        return 1
    fi
    
    # Check if configuration file exists
    if [[ ! -f "/etc/ddogreen/ddogreen.conf" ]]; then
        print_error "Configuration file not found at /etc/ddogreen/ddogreen.conf"
        return 1
    fi
    
    # Check if service file exists
    if [[ ! -f "/etc/systemd/system/ddogreen.service" ]]; then
        print_error "Service file not found"
        return 1
    fi
    
    # Check if service is enabled
    if ! systemctl is-enabled ddogreen >/dev/null 2>&1; then
        print_error "Service is not enabled"
        return 1
    fi
    
    # Check if service is running
    if ! systemctl is-active ddogreen >/dev/null 2>&1; then
        print_error "Service is not running"
        return 1
    fi
    
    # Test service restart
    systemctl restart ddogreen
    sleep 2
    
    if ! systemctl is-active ddogreen >/dev/null 2>&1; then
        print_error "Service failed to restart"
        return 1
    fi
    
    print_success "Service functionality verified"
}

# Function to test DEB package
test_deb_package() {
    local deb_file="$1"
    
    print_info "Testing DEB package: $deb_file"
    
    # Verify package contents
    verify_package_contents "DEB" "$deb_file"
    
    # Clean up any existing installation
    cleanup_installation
    
    # Install package
    print_info "Installing DEB package..."
    dpkg -i "$deb_file"
    
    # Wait for service to start
    sleep 3
    
    # Test functionality
    test_service_functionality
    
    # Test uninstallation
    print_info "Testing DEB package removal..."
    apt-get remove -y ddogreen
    
    # Verify removal
    if which ddogreen 2>/dev/null || command -v ddogreen 2>/dev/null; then
        print_warning "ddogreen binary still found after DEB removal"
    fi
    
    if [[ -f "/usr/local/bin/ddogreen" ]]; then
        print_warning "ddogreen binary still exists after DEB removal"
    fi
    
    print_success "DEB package test completed"
}

# Function to test TGZ package
test_tgz_package() {
    local tgz_file="$1"
    
    print_info "Testing TGZ package: $tgz_file"
    
    # Verify package contents
    verify_package_contents "TGZ" "$tgz_file"
    
    # Clean up any existing installation
    cleanup_installation
    
    # Extract and install package
    print_info "Installing TGZ package..."
    local temp_dir=$(mktemp -d)
    cd "$temp_dir"
    tar -xzf "$tgz_file"
    cd ddogreen-*/
    ./installer.sh --install
    cd /
    rm -rf "$temp_dir"
    
    # Wait for service to start
    sleep 3
    
    # Test functionality
    test_service_functionality
    
    # Test uninstallation
    print_info "Testing TGZ package removal..."
    local temp_dir2=$(mktemp -d)
    cd "$temp_dir2"
    tar -xzf "$tgz_file"
    cd ddogreen-*/
    ./installer.sh --uninstall
    cd /
    rm -rf "$temp_dir2"
    
    # Verify removal
    if which ddogreen 2>/dev/null || command -v ddogreen 2>/dev/null; then
        print_warning "ddogreen binary still found after TGZ removal"
    fi
    
    if [[ -f "/usr/local/bin/ddogreen" ]]; then
        print_warning "ddogreen binary still exists after TGZ removal"
    fi
    
    print_success "TGZ package test completed"
}

# Function to test RPM package structure
test_rpm_package() {
    local rpm_file="$1"
    
    print_info "Testing RPM package: $rpm_file"
    
    # Verify package contents
    verify_package_contents "RPM" "$rpm_file"
    
    print_success "RPM package structure test completed"
    print_warning "Note: Full RPM installation test requires RPM-based system"
}

# Main function
main() {
    check_root
    
    local build_dir="${1:-/home/runner/work/ddogreen/ddogreen/build/release}"
    
    print_info "Starting package tests..."
    print_info "Build directory: $build_dir"
    
    # Check if build directory exists
    if [[ ! -d "$build_dir" ]]; then
        print_error "Build directory not found: $build_dir"
        exit 1
    fi
    
    # Test DEB package
    local deb_file="$build_dir/ddogreen-linux.deb"
    if [[ -f "$deb_file" ]]; then
        test_deb_package "$deb_file"
    else
        print_warning "DEB package not found: $deb_file"
    fi
    
    # Test TGZ package
    local tgz_file="$build_dir/ddogreen-linux.tar.gz"
    if [[ -f "$tgz_file" ]]; then
        test_tgz_package "$tgz_file"
    else
        print_warning "TGZ package not found: $tgz_file"
    fi
    
    # Test RPM package (structure only)
    local rpm_file="$build_dir/ddogreen-linux.rpm"
    if [[ -f "$rpm_file" ]]; then
        test_rpm_package "$rpm_file"
    else
        print_warning "RPM package not found: $rpm_file"
    fi
    
    # Final cleanup
    cleanup_installation
    
    print_success "All package tests completed successfully!"
}

# Show usage if help requested
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    cat << EOF
ddogreen Package Testing Script

Usage: sudo ./test_packages.sh [BUILD_DIR]

Arguments:
  BUILD_DIR    Path to build directory containing packages (default: /home/runner/work/ddogreen/ddogreen/build/release)

This script tests:
- DEB package installation, functionality, and removal
- TGZ package installation, functionality, and removal  
- RPM package structure validation

Requirements:
- Must be run as root (use sudo)
- Packages must be built in BUILD_DIR
- TLP should be installed

Examples:
  sudo ./test_packages.sh
  sudo ./test_packages.sh /path/to/build/dir
EOF
    exit 0
fi

# Run main function
main "$@"
