#!/bin/bash

# Uninstallation script for ddops

set -e

echo "Uninstalling ddops - Dynamic Device Optimization Power Switcher"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)"
   exit 1
fi

# Uninstall service using the built-in service management
echo "Uninstalling system service..."
if [ -f /usr/local/bin/ddops ]; then
    /usr/local/bin/ddops --uninstall
else
    echo "ddops executable not found, manually cleaning up service..."
    systemctl stop ddops 2>/dev/null || true
    systemctl disable ddops 2>/dev/null || true
    rm -f /etc/systemd/system/ddops.service
    systemctl daemon-reload
fi

# Remove files
echo "Removing files..."
rm -f /usr/local/bin/ddops
rm -rf /etc/ddops
rm -f /var/log/ddops.log
rm -f /var/run/ddops.pid

echo "Uninstallation completed successfully!"
