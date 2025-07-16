#!/bin/bash

# Installation script for ddops

set -e

echo "Installing ddops - Dynamic Device Optimization Power Switcher"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)"
   exit 1
fi

# Check if TLP is installed
if ! command -v tlp &> /dev/null; then
    echo "TLP is not installed. Please install TLP first:"
    echo "  Ubuntu/Debian: sudo apt install tlp"
    echo "  Fedora/RHEL:   sudo dnf install tlp"
    exit 1
fi

# Check for required development packages
echo "Checking for required packages..."

# Ubuntu/Debian
if command -v apt &> /dev/null; then
    apt update
    apt install -y build-essential cmake
fi

# Fedora/RHEL
if command -v dnf &> /dev/null; then
    dnf install -y gcc-c++ cmake
fi

# Build the project
echo "Building ddops..."
if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
make -j$(nproc)

# Install
echo "Installing ddops..."
make install

# Install the service using the built-in service management
echo "Installing system service..."
/usr/local/bin/ddops --install

# Create log directory
mkdir -p /var/log
touch /var/log/ddops.log
chmod 644 /var/log/ddops.log

# Create config directory
mkdir -p /etc/ddotlp

echo "Installation completed successfully!"
echo
echo "To start the service:"
echo "  sudo systemctl enable ddops"
echo "  sudo systemctl start ddops"
echo
echo "To check status:"
echo "  sudo systemctl status ddops"
echo
echo "To view logs:"
echo "  sudo journalctl -u ddops -f"
