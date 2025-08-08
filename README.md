# ddogreen – Intelligent Green Power Management

A smart sustainability-focused service that automatically optimizes your PC and laptop power consumption by intelligently switching between high-performance and power-saving modes based on actual system usage patterns.

**Cross-Platform Support**: Linux (TLP) and Windows (Power Plans)

ddogreen - Power Management Service for Linux & Windows
======================================================

An intelligent power management daemon that automatically optimizes your system's power settings based on CPU load monitoring.

Developed by DDOSoft Software Solutions (www.ddosoft.com)

## Why Use ddogreen?

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

ddogreen watches how busy your computer is and automatically switches power modes:

- **High Performance Mode**: When you're actively working (high system load)
  - **Linux**: TLP AC mode (`tlp ac`)
  - **Windows**: High Performance power plan
- **Power Saving Mode**: When your laptop is mostly idle (low system load)
  - **Linux**: TLP battery mode (`tlp bat`)
  - **Windows**: Power Saver power plan

## Installation

### Linux Installation

Recommended (DEB/RPM packages)
```bash
# Debian/Ubuntu
wget https://github.com/abkulakli/ddogreen/releases/latest/download/ddogreen-linux.deb
sudo apt install ./ddogreen-linux.deb

# Fedora/RHEL
wget https://github.com/abkulakli/ddogreen/releases/latest/download/ddogreen-linux.rpm
sudo rpm -i ddogreen-linux.rpm
```

Packages install the systemd service and wire it to start automatically after configuration.

Alternative (TGZ)
```bash
wget https://github.com/abkulakli/ddogreen/releases/latest/download/ddogreen-linux.tgz
tar xzf ddogreen-linux.tgz
cd ddogreen-linux
sudo ./installer.sh   # sets up files and service
```

Prerequisite: TLP must be installed on Linux
```bash
# Ubuntu/Debian
sudo apt install tlp

# Fedora/RHEL
sudo dnf install tlp
```

### Windows Installation

Recommended (MSI)
1. Download ddogreen-windows.msi from Releases
2. Run the MSI (Administrator). It installs the service and data paths.

Alternative (ZIP)
1. Download ddogreen-windows.zip from Releases
2. Extract
3. Run installer.bat as Administrator to install and start the service

## Check if It's Working

### Service Status

- Linux: `sudo systemctl status ddogreen`
- Windows: `sc query ddogreen`

### See What It's Doing

#### Linux
```bash
sudo tail -f /var/log/ddogreen.log
```

#### Windows
- Check Windows Event Viewer (Applications and Services Logs), and/or service status

You'll see messages like:
```
[2025-07-21 15:30:45] System became idle - switching to power saving mode
[2025-07-21 15:45:12] System became active - switching to high performance mode
```

### Check Your Current Power Mode

#### Linux
```bash
sudo tlp-stat -s
```

#### Windows
```cmd
powercfg /getactivescheme
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

### Linux

**Service won't start?**
- Make sure TLP is installed: `which tlp`
- Check service status: `sudo systemctl status ddogreen`
- Verify logs: `sudo tail /var/log/ddogreen.log`

**Not switching modes?**
- Check the logs: `sudo tail /var/log/ddogreen.log`
- Verify TLP is working: `sudo tlp-stat`
- Ensure you have admin privileges

### Windows

**Service won't start?**
- Make sure you ran the install command as Administrator
- Check service status: `sc query ddogreen`
- Verify power plans are available: `powercfg /list`

**Not switching modes?**
- Check the logs: `type C:\Windows\System32\ddogreen.log`
- Verify power plans: `powercfg /getactivescheme`
- Ensure you have admin privileges

### Uninstall

- Linux (DEB): `sudo apt remove ddogreen`
- Linux (RPM): `sudo rpm -e ddogreen`
- Windows (MSI): Uninstall from “Apps & features”
- Windows (ZIP): Run `installer.bat` uninstall option (if provided), or remove service manually

## Advanced Usage

### Manual Control
Run interactively (foreground) for testing, or as a daemon/service.

- Linux: `sudo ddogreen` (interactive), `sudo ddogreen --daemon` (service-style)
- Windows: `ddogreen.exe` (interactive), `ddogreen.exe --daemon` (service-style)
```
Usage: ddogreen [OPTIONS]
Options:
  -d, --daemon           Run as daemon/service
  -c, --config PATH      Use custom configuration file
  -h, --help             Show this help message
  -v, --version          Show version information
```
  -d, --daemon           Run as daemon/service
```

---

**Make your laptop more sustainable with zero effort - on Linux and Windows!**

*Every laptop running ddogreen helps reduce global energy consumption.*

## Platform Support

- ✅ **Linux** - Full support with TLP integration
- ✅ **Windows** - Full support with Power Plans integration
- 🚧 **macOS** - Architecture ready, implementation pending

## Technical Details

### Simple and Safe
- **No configuration needed** - works automatically with smart defaults
- **Minimal resource usage** - checks system load once per minute
- **Safe** - only changes power modes, nothing else
- **Cross-platform** - same intelligent logic on all platforms

### How It Monitors
- Watches your system's load average to detect when you're busy vs idle
- Uses smart thresholds with hysteresis to prevent rapid switching
- Automatically adapts to your work patterns

For developers and contributors, see the project's memory bank documentation for technical implementation details.

## Command Line Options

ddogreen supports several command-line options for different use cases on both Linux and Windows:

```bash
Usage: ddogreen [OPTIONS]
Options:
  -d, --daemon           Run as daemon/service
  -h, --help             Show this help message
  -v, --version          Show version information
  -i, --install          Install system service
  -u, --uninstall        Uninstall system service
```

## Configuration

ddogreen requires a configuration file. Packages ship a template at `share/ddogreen/ddogreen.conf.default`.

Default configuration paths
- Linux: `/etc/ddogreen/ddogreen.conf`
- Windows: `%ProgramData%\DDOSoft\ddogreen\ddogreen.conf`

Quick setup (Linux)
```bash
sudo mkdir -p /etc/ddogreen
sudo cp /usr/share/ddogreen/ddogreen.conf.default /etc/ddogreen/ddogreen.conf
sudo nano /etc/ddogreen/ddogreen.conf
```

Quick setup (Windows, ZIP)
1. Create `%ProgramData%\DDOSoft\ddogreen` if missing
2. Copy `share\ddogreen\ddogreen.conf.default` to `%ProgramData%\DDOSoft\ddogreen\ddogreen.conf`
3. Edit values as needed

### Configuration Format

The configuration file uses a simple key=value format:

```ini
# CPU load thresholds (as decimal percentages per core)
# Example: 0.70 = 70% load per core - for aggressive performance switching
# Example: 0.30 = 30% load per core - for conservative power saving
# The gap between thresholds (30%-70%) creates hysteresis to prevent rapid mode switching
high_performance_threshold=0.70
power_save_threshold=0.30

# Monitoring frequency in seconds
# Example: 10 = check every 10 seconds for responsive power management
monitoring_frequency=10
```

### Configuration Parameters

- **high_performance_threshold**: CPU load per core threshold for switching to high performance mode (0.1-1.0)
- **power_save_threshold**: CPU load per core threshold for switching to power save mode (0.05-0.9)
- **monitoring_frequency**: How often to check system load in seconds (1-300)

### Hysteresis Behavior

The application uses hysteresis to prevent rapid mode switching:
- Switches to **high performance** when load > (cores × high_performance_threshold)
- Switches to **power save** when load < (cores × power_save_threshold)

Example with 20 cores and default thresholds:
- High performance trigger: 20 × 0.70 = 14.00 load average
- Power save trigger: 20 × 0.30 = 6.00 load average

### Service Management

- Linux: Services are installed and managed by DEB/RPM/TGZ installers. Use `systemctl` to control.
- Windows: MSI installs and manages the service. ZIP provides an `installer.bat` to install/uninstall.

### Interactive vs Daemon Mode

**Interactive Mode** (for testing and monitoring):
- **Linux**: `sudo ddogreen`
- **Windows**: `ddogreen.exe`
- Shows real-time activity on console
- Perfect for seeing how ddogreen responds to system load
- All log messages displayed with timestamps

**Daemon/Service Mode** (for production):
- **Linux**: `sudo ddogreen -d`
- **Windows**: `ddogreen.exe -d`
- Silent operation, logs to file only
- Suitable for systemd/Windows service deployment

## Check if It's Working

### Service Status

#### Linux
```bash
sudo systemctl status ddogreen
```

#### Windows
```cmd
sc query ddogreen
# or
services.msc  # Look for "ddogreen" service
```

### See What It's Doing

#### Linux
```bash
sudo tail -f /var/log/ddogreen.log
```

#### Windows
```cmd
type C:\Windows\System32\ddogreen.log
# or monitor with:
Get-Content C:\Windows\System32\ddogreen.log -Wait
```

You'll see messages like:
```
[2025-07-02 15:30:45] System became idle - switching to power saving mode
[2025-07-02 15:45:12] System became active - switching to high performance mode
```

### Check Your Current Power Mode

#### Linux
```bash
sudo tlp-stat -s
```

#### Windows
```cmd
powercfg /getactivescheme
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

### Linux

**Service won't start?**
- Make sure TLP is installed: `which tlp`
- Check service status: `sudo systemctl status ddogreen`

**Not switching modes?**
- Check the logs: `sudo tail /var/log/ddogreen.log`
- Verify TLP is working: `sudo tlp-stat`

### Windows

**Service won't start?**
- Make sure you ran the install command as Administrator
- Check service status: `sc query ddogreen`
- Verify power plans are available: `powercfg /list`

**Not switching modes?**
- Check the logs: `type C:\Windows\System32\ddogreen.log`
- Verify power plans: `powercfg /getactivescheme`
- Ensure power plans are enabled: `powercfg /list`

**Need help?**
Check the detailed logs for error messages:
- **Linux**: `/var/log/ddogreen.log`
- **Windows**: `C:\Windows\System32\ddogreen.log`

## Uninstall

### Linux
```bash
sudo ddogreen --uninstall
sudo rm -f /usr/local/bin/ddogreen
```

### Windows
```cmd
# Run as Administrator
ddogreen.exe --uninstall
del C:\path\to\ddogreen.exe
```

### Technical Details

### Cross-Platform Architecture
- Read-only configuration required at startup (no built-in defaults)
- Minimal resource usage with configurable monitoring frequency
- Platform abstraction in application layer; platform-specific implementations under the hood
- Safe: only switches power modes via platform backends

### Power Management
- **Linux**: Uses TLP (ThinkPad-Linux-Power) for power management
  - High Performance: `tlp ac` (AC adapter mode)
  - Power Saving: `tlp bat` (battery mode)
- **Windows**: Uses built-in Power Plans via `powercfg`
  - High Performance: High Performance power plan
  - Power Saving: Power Saver power plan

### System Monitoring
- Linux: Kernel load averages (`/proc/loadavg`)
- Windows: Performance Counters-based load equivalent
- Thresholds: Configurable per-core thresholds with hysteresis
- Monitoring frequency: Configurable (1–300 seconds)

### Service Management
- **Linux**: Integrates with systemd for service management
- **Windows**: Uses Service Control Manager (SCM) for Windows services
- **Installation**: Automatic service installation and configuration
- **Logging**: Platform-appropriate log file locations

---

**Make your laptop more sustainable with zero effort - on Linux and Windows!**

*Every laptop running ddogreen helps reduce global energy consumption.*

## Platform Support

- ✅ **Linux** - Full support with TLP integration
- ✅ **Windows** - Full support with Power Plans integration
- 🚧 **macOS** - Architecture ready, implementation pending

## Same Rules, Every Platform

ddogreen uses the same intelligent logic across all platforms:
- **Configurable CPU load thresholds** with hysteresis for stable performance/power-saving switching
- **Configurable monitoring interval** for optimal responsiveness vs system impact balance
- **Automatic service/daemon management** for production deployment
- **Cross-platform logging and monitoring** capabilities
