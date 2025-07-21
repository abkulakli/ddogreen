#!/bin/bash
# Generic ddogreen installer for all Linux distributions
# Works without package managers - uses direct systemd integration

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

# Service configuration
SERVICE_NAME="ddogreen"
SERVICE_DESCRIPTION="DDOSoft ddogreen - Intelligent Green Power Management for Sustainable Computing"
TARGET_EXECUTABLE_PATH="/usr/local/bin/ddogreen"
SERVICE_FILE="/etc/systemd/system/ddogreen.service"
LOG_FILE="/var/log/ddogreen.log"
PID_FILE="/run/ddogreen.pid"

# Function to check if running as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

# Function to check if systemd is available
check_systemd() {
    if ! command -v systemctl &> /dev/null; then
        print_error "systemctl not found. This installer requires systemd."
        exit 1
    fi
}

# Function to check if TLP is installed
check_tlp() {
    if ! command -v tlp &> /dev/null; then
        print_warning "TLP not found. ddogreen requires TLP for power management."
        print_info "Install TLP with:"
        print_info "  Ubuntu/Debian: sudo apt install tlp"
        print_info "  Fedora/RHEL:   sudo dnf install tlp"
        print_info "  Arch Linux:    sudo pacman -S tlp"
        print_info ""
        read -p "Continue anyway? [y/N]: " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Function to install ddogreen
install_ddogreen() {
    print_info "Installing ddogreen..."
    
    # Check if binary exists in current directory or bin/ subdirectory
    DDOGREEN_BINARY=""
    if [[ -f "ddogreen" ]]; then
        DDOGREEN_BINARY="ddogreen"
    elif [[ -f "bin/ddogreen" ]]; then
        DDOGREEN_BINARY="bin/ddogreen"
    else
        print_error "ddogreen binary not found in current directory or bin/ subdirectory"
        print_error "Please run this script from the extracted tar.gz directory"
        exit 1
    fi
    
    # Check if service already exists and stop it
    if systemctl is-active --quiet "$SERVICE_NAME" 2>/dev/null; then
        print_warning "ddogreen service is already running"
        print_info "Stopping existing service..."
        systemctl stop "$SERVICE_NAME" || true
    fi
    
    if [[ -f "$SERVICE_FILE" ]]; then
        print_warning "ddogreen service already exists - updating..."
        systemctl disable "$SERVICE_NAME" || true
    fi
    
    # Install configuration file first (before service installation)
    print_info "Setting up configuration..."
    mkdir -p /etc/ddogreen
    if [[ ! -f "/etc/ddogreen/ddogreen.conf" ]]; then
        if [[ -f "share/ddogreen/ddogreen.conf.default" ]]; then
            cp "share/ddogreen/ddogreen.conf.default" "/etc/ddogreen/ddogreen.conf"
            chmod 644 "/etc/ddogreen/ddogreen.conf"
            print_success "Default configuration installed at /etc/ddogreen/ddogreen.conf"
        elif [[ -f "ddogreen.conf.default" ]]; then
            cp "ddogreen.conf.default" "/etc/ddogreen/ddogreen.conf"
            chmod 644 "/etc/ddogreen/ddogreen.conf"
            print_success "Default configuration installed at /etc/ddogreen/ddogreen.conf"
        elif [[ -f "ddogreen.conf" ]]; then
            cp "ddogreen.conf" "/etc/ddogreen/ddogreen.conf"
            chmod 644 "/etc/ddogreen/ddogreen.conf"
            print_success "Default configuration installed at /etc/ddogreen/ddogreen.conf"
        else
            print_warning "Configuration template not found in package - you may need to create /etc/ddogreen/ddogreen.conf manually"
        fi
    else
        print_info "Existing configuration preserved at /etc/ddogreen/ddogreen.conf"
    fi
    
    # Use the application's built-in service installation (which will also copy the executable)
    print_info "Installing executable and starting system service..."
    if "$DDOGREEN_BINARY" --install; then
        print_success "ddogreen installed and started successfully!"
        print_info ""
        print_info "Service Status:"
        systemctl status "$SERVICE_NAME" --no-pager --lines=3
        print_info ""
        print_info "To view logs: sudo journalctl -u ddogreen -f"
        print_info "To stop:      sudo systemctl stop ddogreen"
        print_info "To restart:   sudo systemctl restart ddogreen"
        print_info ""
        print_success "ddogreen is now managing your system's power automatically!"
    else
        print_error "Service installation failed"
        print_info "Check the logs: sudo journalctl -u ddogreen"
        exit 1
    fi
}

# Function to uninstall ddogreen
uninstall_ddogreen() {
    print_info "Uninstalling ddogreen..."
    
    # Use the application's built-in service uninstallation if executable exists
    if [[ -f "$TARGET_EXECUTABLE_PATH" ]]; then
        print_info "Using built-in service uninstall..."
        if "$TARGET_EXECUTABLE_PATH" --uninstall; then
            print_info "Service uninstalled successfully."
        else
            print_warning "Built-in uninstall failed, attempting manual cleanup..."
            # Fallback to manual cleanup
            systemctl stop "$SERVICE_NAME" || true
            systemctl disable "$SERVICE_NAME" || true
            rm -f "$SERVICE_FILE" || true
            systemctl daemon-reload || true
        fi
        
        # Remove executable
        print_info "Removing executable..."
        rm -f "$TARGET_EXECUTABLE_PATH"
    else
        print_warning "Executable not found, performing manual cleanup..."
        # Manual cleanup if executable is missing
        systemctl stop "$SERVICE_NAME" || true
        systemctl disable "$SERVICE_NAME" || true
        rm -f "$SERVICE_FILE" || true
        systemctl daemon-reload || true
    fi
    
    # Clean up remaining files
    print_info "Cleaning up files..."
    rm -f "$LOG_FILE"
    rm -f "$PID_FILE"
    
    # Remove configuration directory (ask user first)
    if [[ -d "/etc/ddogreen" ]]; then
        echo -n "Remove configuration directory /etc/ddogreen? [y/N]: "
        read -r response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            rm -rf "/etc/ddogreen"
            print_info "Configuration directory removed."
        else
            print_info "Configuration directory preserved."
        fi
    fi
    
    print_success "ddogreen has been completely uninstalled."
}

# Function to show status
show_status() {
    print_info "ddogreen Status:"
    
    if [[ -f "$TARGET_EXECUTABLE_PATH" ]]; then
        print_success "Executable: $TARGET_EXECUTABLE_PATH (installed)"
    else
        print_warning "Executable: Not installed"
    fi
    
    if [[ -f "$SERVICE_FILE" ]]; then
        print_success "Service: $SERVICE_FILE (installed)"
        
        if systemctl is-enabled --quiet "$SERVICE_NAME" 2>/dev/null; then
            print_success "Auto-start: Enabled"
        else
            print_warning "Auto-start: Disabled"
        fi
        
        if systemctl is-active --quiet "$SERVICE_NAME" 2>/dev/null; then
            print_success "Status: Running"
        else
            print_warning "Status: Stopped"
        fi
    else
        print_warning "Service: Not installed"
    fi
}

# Function to show usage
show_usage() {
    cat << EOF
ddogreen Generic Linux Installer

Usage: sudo ./install.sh [OPTION]

Options:
  --install     Install ddogreen service
  --uninstall   Uninstall ddogreen service  
  --status      Show installation status
  --help        Show this help message

Examples:
  sudo ./install.sh --install      # Install and start service
  sudo ./install.sh --uninstall    # Stop and remove service
  sudo ./install.sh --status       # Check current status

Requirements:
  - Linux with systemd
  - TLP installed (sudo apt install tlp / sudo dnf install tlp)
  - Root privileges (use sudo)

After installation:
  - Service starts automatically on boot
  - View logs: sudo journalctl -u ddogreen -f
  - Control service: sudo systemctl {start|stop|restart|status} ddogreen

EOF
}

# Main script logic
case "${1:-}" in
    --install)
        check_root
        check_systemd
        check_tlp
        install_ddogreen
        ;;
    --uninstall)
        check_root
        check_systemd
        uninstall_ddogreen
        ;;
    --status)
        show_status
        ;;
    --help)
        show_usage
        ;;
    "")
        print_error "No option specified"
        echo
        show_usage
        exit 1
        ;;
    *)
        print_error "Unknown option: $1"
        echo
        show_usage
        exit 1
        ;;
esac
