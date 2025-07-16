# ddops – Dynamic Device Optimization Power Switcher

A smart cross-platform service that automatically optimizes your device's power settings by switching between high-performance and power-saving modes based on system load and usage patterns.

**Developed by ddosoft (www.ddosoft.com)**

## Why Use ddops?

### Longer Battery Life
- **20-30% more battery time** with no effort from you
- Automatically switches to power-saving when you're not actively using the system
- Full performance when you need it for work

### Environmental Benefits  
- **Reduces energy consumption** when your laptop is idle
- Lower carbon footprint from reduced electricity usage
- Contributes to sustainable computing practices

### Save Money
- Lower electricity bills from reduced power consumption
- Your laptop runs cooler, lasting longer

## How It Works

ddops watches how busy your computer is and automatically switches power modes:

- **High Performance Mode**: When you're actively working (system load > 10%)
- **Power Saving Mode**: When your laptop is mostly idle (system load ≤ 10%)

## Installation

### 1. Install Requirements
Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake tlp
```

Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake tlp
```

### 2. Build and Install
```bash
git clone <repository-url>
cd ddops
./build.sh
sudo ./install.sh
```

**Or install manually:**
```bash
./build.sh
sudo make install -C build
sudo ddops --install    # Install system service
sudo systemctl enable ddops
sudo systemctl start ddops
```

### 3. Start the Service
```bash
sudo systemctl enable ddops
sudo systemctl start ddops
```

That's it! ddops is now running and will automatically manage your power settings.

## Command Line Options

ddops supports several command-line options for different use cases:

```bash
Usage: ddops [OPTIONS]
Options:
  -d, --daemon           Run as daemon
  -h, --help             Show this help message
  -v, --version          Show version information
  -i, --install          Install system service
  -u, --uninstall        Uninstall system service
```

### Service Management
```bash
# Install the service
sudo ddops --install

# Uninstall the service  
sudo ddops --uninstall

# Run interactively (see live activity)
sudo ddops

# Run as daemon (silent operation)
sudo ddops --daemon
```

### Interactive vs Daemon Mode

**Interactive Mode** (for testing and monitoring):
```bash
sudo ddops
```
- Shows real-time activity on console
- Perfect for seeing how ddops responds to system load
- All log messages displayed with timestamps

**Daemon Mode** (for production):
```bash
sudo ddops -d
```
- Silent operation, logs to file only
- Suitable for systemd service deployment

## Check if It's Working

### Service Status
```bash
sudo systemctl status ddops
```

### See What It's Doing
```bash
sudo tail -f /var/log/ddops.log
```

You'll see messages like:
```
[2025-07-02 15:30:45] System became idle - switching to power saving mode
[2025-07-02 15:45:12] System became active - switching to high performance mode
```

### Check Your Current Power Mode
```bash
sudo tlp-stat -s
```

## Your Energy Impact

### Individual Benefits
- **Daily savings**: Reduced energy consumption per laptop
- **Annual impact**: Significant reduction in electricity usage
- **Environmental benefit**: Lower carbon footprint from reduced power consumption

### Cost Savings
- **Lower electricity bills** from reduced power consumption
- Extended laptop lifespan from better thermal management

## Troubleshooting

**Service won't start?**
- Make sure TLP is installed: `which tlp`
- Check service status: `sudo systemctl status ddops`

**Not switching modes?**
- Check the logs: `sudo tail /var/log/ddops.log`
- Verify TLP is working: `sudo tlp-stat`

**Need help?**
Check the detailed logs in `/var/log/ddops.log` for error messages.

## Uninstall

**Using the script:**
```bash
sudo ./uninstall.sh
```

**Or manually:**
```bash
sudo ddops --uninstall
sudo rm -f /usr/local/bin/ddops
```

## Technical Details

- **No configuration needed** - works automatically with smart defaults
- **Minimal resource usage** - checks system load once per minute
- **Works everywhere** - any Linux system with TLP
- **Safe** - only changes TLP power modes, nothing else

---

**Make your Linux laptop more sustainable with zero effort!**

*Every laptop running ddops helps reduce global energy consumption.*
