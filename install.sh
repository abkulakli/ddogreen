#!/bin/bash

# Installation script for ddotlp

set -e

echo "Installing ddotlp - Dynamic TLP Power Management Service"

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
echo "Building ddotlp..."
if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
make -j$(nproc)

# Install
echo "Installing ddotlp..."
make install

# Create log directory
mkdir -p /var/log
touch /var/log/ddotlp.log
chmod 644 /var/log/ddotlp.log

# Create config directory
mkdir -p /etc/ddotlp

# Reload systemd
systemctl daemon-reload

echo "Installation completed successfully!"
echo
echo "To start the service:"
echo "  sudo systemctl enable ddotlp"
echo "  sudo systemctl start ddotlp"
echo
echo "To check status:"
echo "  sudo systemctl status ddotlp"
echo
echo "To view logs:"
echo "  sudo journalctl -u ddotlp -f"
