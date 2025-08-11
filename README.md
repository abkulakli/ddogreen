# ddogreen – Intelligent Green Power Management

A smart sustainability-focused service that automatically optimizes your PC and laptop power consumption by intelligently switching between high-performance and power-saving modes based on actual system usage patterns.

**Cross-Platform Support**: Linux (TLP) and Windows (Power Plans)

ddogreen - Power Management Service for Linux & Windows
======================================================

An intelligent power management daemon that automatically optimizes your system's power settings based on CPU load monitoring.

Developed by DDOSoft Solutions (www.ddosoft.com)

## Why Use ddogreen?

### Longer Battery Life
- **20-30% more battery time** with no effort from you
- Automatically switches to power-saving when you're not actively using the system
- Full performance when you need it for work

### Environmental Benefits
- **Reduces energy consumption** when your laptop is idle
- Lower carbon footprint from reduced electricity usage
- Contributes to sustainable computing practices

### Cost Savings
- **Lower electricity bills** from reduced power consumption
- Extended laptop lifespan from better thermal management

## How It Works

ddogreen watches how busy your computer is and automatically switches power modes:

- **High Performance Mode**: When you're actively working (high system load)
  - **Linux**: TLP AC mode (`tlp ac`)
  - **Windows**: High Performance power plan
- **Power Saving Mode**: When your laptop is mostly idle (low system load)
  - **Linux**: TLP battery mode (`tlp bat`)
  - **Windows**: Power Saver power plan

## Installation

See the latest Release for platform-specific installation instructions (Linux DEB/RPM/TGZ, Windows MSI/ZIP):

https://github.com/abkulakli/ddogreen/releases/latest

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
- Linux (TGZ): Use the installer script’s uninstall option if available, or disable/stop the service and remove installed files
  - `sudo systemctl disable --now ddogreen`
  - Remove installed files under your chosen install prefix
- Windows (MSI): Uninstall from “Apps & features”
- Windows (ZIP): Use `installer.bat` uninstall option (run as Administrator)

## Advanced Usage

### Manual Control
Run interactively (foreground) for testing. Service deployment handles background operation automatically.

- Linux: `sudo ddogreen` (interactive) - For services, use systemd packages or installer
- Windows: `ddogreen.exe` (interactive) - For services, use MSI installer
```
Usage: ddogreen [OPTIONS]
Options:
  -c, --config PATH      Use custom configuration file
  -h, --help             Show this help message
  -v, --version          Show version information
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
- **Requires a configuration file** - packages ship a template you can copy and edit
- **Minimal resource usage** - configurable monitoring frequency (1–300 seconds)
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
  -c, --config PATH      Use custom configuration file
  -h, --help             Show this help message
  -v, --version          Show version information
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

### Interactive vs Service Mode

**Interactive Mode** (for testing and monitoring):
- **Linux**: `sudo ddogreen`
- **Windows**: `ddogreen.exe`
- Shows real-time activity on console
- Perfect for seeing how ddogreen responds to system load
- All log messages displayed with timestamps

**Service Mode** (for production):
- **Linux**: Managed by systemd via package installers
- **Windows**: Managed by Windows Service Control Manager via MSI installer
- Background operation with file logging
- Automatic startup and process supervision by service manager

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
Use your package manager (preferred):
```bash
# Debian/Ubuntu
sudo apt remove ddogreen

# Fedora/RHEL
sudo rpm -e ddogreen
```

For TGZ installs, use the installer’s uninstall option if provided, or:
```bash
sudo systemctl disable --now ddogreen
# Then remove installed files (bin, config, service) as appropriate
```

### Windows
MSI installs: uninstall from “Apps & features”.

ZIP installs: run `installer.bat` and choose uninstall (run as Administrator).

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
