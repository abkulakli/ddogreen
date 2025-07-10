# ddotlp - Smart Power Management for Linux

A simple service that automatically saves energy by switching your Linux laptop between high-performance and power-saving modes based on how busy your system is.

**Developed by ddosoft (www.ddosoft.com)**

## Why Use ddotlp?

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

ddotlp watches how busy your computer is and automatically switches power modes:

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
cd ddotlp
./build.sh
sudo ./install.sh
```

### 3. Start the Service
```bash
sudo systemctl enable ddotlp
sudo systemctl start ddotlp
```

That's it! ddotlp is now running and will automatically manage your power settings.

## Check if It's Working

### Service Status
```bash
sudo systemctl status ddotlp
```

### See What It's Doing
```bash
sudo tail -f /var/log/ddotlp.log
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
- Check service status: `sudo systemctl status ddotlp`

**Not switching modes?**
- Check the logs: `sudo tail /var/log/ddotlp.log`
- Verify TLP is working: `sudo tlp-stat`

**Need help?**
Check the detailed logs in `/var/log/ddotlp.log` for error messages.

## Uninstall

```bash
sudo ./uninstall.sh
```

## Technical Details

- **No configuration needed** - works automatically with smart defaults
- **Minimal resource usage** - checks system load once per minute
- **Works everywhere** - any Linux system with TLP
- **Safe** - only changes TLP power modes, nothing else

---

**Make your Linux laptop more sustainable with zero effort!**

*Every laptop running ddotlp helps reduce global energy consumption.*
