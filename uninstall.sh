#!/bin/bash

# Uninstallation script for ddotlp

set -e

echo "Uninstalling ddotlp - Dynamic TLP Power Management Service"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)"
   exit 1
fi

# Stop and disable service
echo "Stopping and disabling service..."
systemctl stop ddotlp 2>/dev/null || true
systemctl disable ddotlp 2>/dev/null || true

# Remove files
echo "Removing files..."
rm -f /usr/local/bin/ddotlp
rm -f /etc/systemd/system/ddotlp.service
rm -rf /etc/ddotlp
rm -f /var/log/ddotlp.log
rm -f /var/run/ddotlp.pid

# Reload systemd
systemctl daemon-reload

echo "Uninstallation completed successfully!"
