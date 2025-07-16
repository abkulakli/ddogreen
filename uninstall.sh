#!/bin/bash

# # Stop and disable service
systemctl stop ddops 2>/dev/null || true
systemctl disable ddops 2>/dev/null || true

# Remove files
echo "Removing files..."
rm -f /usr/local/bin/ddops
rm -f /etc/systemd/system/ddops.service
rm -rf /etc/ddops
rm -f /var/log/ddops.log
rm -f /var/run/ddops.pidion script for ddops

set -e

echo "Uninstalling ddops - Dynamic Device Optimization Power Switcher"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)"
   exit 1
fi

# Stop and disable service
echo "Stopping and disabling service..."
systemctl stop ddops 2>/dev/null || true
systemctl disable ddops 2>/dev/null || true

# Remove files
echo "Removing files..."
rm -f /usr/local/bin/ddops
rm -f /etc/systemd/system/ddops.service
rm -rf /etc/ddops
rm -f /var/log/ddops.log
rm -f /var/run/ddops.pid

# Reload systemd
systemctl daemon-reload

echo "Uninstallation completed successfully!"
