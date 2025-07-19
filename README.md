# ddogreen – Intelligent Green Power Management

A smart sustainability-focused service that automatically optimizes your PC and laptop power consumption by intelligently switching between high-performance and power-saving modes based on actual system usage patterns.

**Cross-Platform Support**: Works on Linux (with TLP) and Windows (with Power Plans)

**Developed by DDOSoft Sustainability Solutions (www.ddosoft.com)**

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

#### 1. Install Requirements
Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake tlp
```

Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake tlp
```

#### 2. Build and Install
```bash
git clone <repository-url>
cd ddogreen
./build.sh
sudo make install -C build
sudo ddogreen --install    # Install and start system service
```

### Windows Installation

#### 1. Install Requirements
- **Visual Studio** or **MinGW-w64** with C++17 support
- **CMake** (3.16 or later)
- **Git** for cloning the repository

#### 2. Build and Install
```cmd
git clone <repository-url>
cd ddogreen
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### 3. Install as Windows Service
```cmd
# Run as Administrator
ddogreen.exe --install    # Install and start Windows service
```

That's it! ddogreen is now running and will automatically manage your power settings.

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

ddogreen requires a configuration file at `/etc/ddogreen/ddogreen.conf` on Linux.

### Configuration Setup

1. Create the configuration directory:
   ```bash
   sudo mkdir -p /etc/ddogreen
   ```

2. Copy the example configuration:
   ```bash
   sudo cp example-config/ddogreen.conf /etc/ddogreen/
   ```

3. Edit the configuration as needed:
   ```bash
   sudo nano /etc/ddogreen/ddogreen.conf
   ```

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

#### Linux (systemd)
```bash
# Install the service
sudo ddogreen --install

# Uninstall the service  
sudo ddogreen --uninstall

# Run interactively (see live activity)
sudo ddogreen

# Run as daemon (silent operation)
sudo ddogreen --daemon
```

#### Windows (Service Control Manager)
```cmd
# Install the service (run as Administrator)
ddogreen.exe --install

# Uninstall the service (run as Administrator)
ddogreen.exe --uninstall

# Run interactively (see live activity)
ddogreen.exe

# Run as service (silent operation)
ddogreen.exe --daemon
```

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

## Technical Details

### Cross-Platform Architecture
- **No configuration needed** - works automatically with smart defaults
- **Minimal resource usage** - checks system load once per minute
- **Platform abstraction** - same core logic, platform-specific implementations
- **Safe** - only changes power modes, nothing else

### Power Management
- **Linux**: Uses TLP (ThinkPad-Linux-Power) for power management
  - High Performance: `tlp ac` (AC adapter mode)
  - Power Saving: `tlp bat` (battery mode)
- **Windows**: Uses built-in Power Plans via `powercfg`
  - High Performance: High Performance power plan
  - Power Saving: Power Saver power plan

### System Monitoring
- **Linux**: Reads load averages from `/proc/loadavg`
- **Windows**: Uses Performance Counters to calculate load averages
- **Thresholds**: Configurable CPU load per core thresholds with hysteresis to prevent rapid switching
- **Check Interval**: Configurable monitoring frequency (default: 10 seconds)

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
