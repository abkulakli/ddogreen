# ddotlp - Dynamic TLP Power Management

A Linux service that automatically manages TLP power settings based on system activity. It switches between performance mode when the system is active (high CPU usage) and battery saving mode when the system is idle (low CPU usage).

## Features

- **Automatic TLP Management**: Switches between `tlp start` (performance) and `tlp bat` (battery) modes
- **CPU-Based Activity Detection**: Uses CPU usage monitoring to detect system activity
- **Configurable Idle Timeout**: Default 5 minutes, customizable
- **Configurable CPU Threshold**: Default 15% CPU usage threshold for activity detection
- **Systemd Integration**: Runs as a proper Linux system service
- **Logging**: Comprehensive logging for debugging and monitoring
- **Signal Handling**: Graceful shutdown and configuration reload
- **No X11 Dependencies**: Works on headless servers and desktop systems alike

## Requirements

- Linux system
- TLP (ThinkPad power management tool)
- CMake 3.16 or higher
- C++17 compatible compiler
- Access to `/proc/stat` for CPU monitoring

## Installation

### Prerequisites

Install required packages on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake tlp
```

Install required packages on Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake tlp
```

### Build and Install

1. Clone or download the source code
2. Build the project using the provided script:
```bash
./build.sh
```

Or manually with CMake:
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

3. Install the service:
```bash
sudo make install
```

Or use the installation script:
```bash
sudo ./install.sh
```

4. Enable and start the service:
```bash
sudo systemctl enable ddotlp
sudo systemctl start ddotlp
```

## Usage

### Service Management

Start the service:
```bash
sudo systemctl start ddotlp
```

Stop the service:
```bash
sudo systemctl stop ddotlp
```

Check service status:
```bash
sudo systemctl status ddotlp
```

View logs:
```bash
sudo journalctl -u ddotlp -f
```

### Log Format

The service provides detailed logging with precise timestamps including milliseconds:

```
[2025-06-30 21:14:15.123] [INFO] ddotlp service started successfully
[2025-06-30 21:14:15.124] [INFO] Activity monitor started (CPU-based monitoring)
[2025-06-30 21:19:15.456] [INFO] System became idle (low CPU usage for 300 seconds)
[2025-06-30 21:19:15.457] [INFO] Switching to battery mode (tlp bat)
[2025-06-30 21:19:15.789] [INFO] TLP output: TLP started in battery mode (manual).
[2025-06-30 21:19:15.790] [INFO] Successfully switched to battery mode
```

The logging captures:
- **Service Events**: Startup, shutdown, mode changes
- **Activity Detection**: CPU usage monitoring and idle detection
- **TLP Output**: Complete output from TLP commands with timestamps
- **Error Handling**: Detailed error messages with context

### Manual Execution

You can also run the daemon manually for testing:
```bash
sudo ddotlp --daemon --timeout 180 --cpu 20.0
```

Options:
- `-d, --daemon`: Run as daemon
- `-t, --timeout`: Idle timeout in seconds (default: 300)
- `-c, --cpu`: CPU threshold for activity detection in percent (default: 15.0)
- `-h, --help`: Show help message
- `-v, --version`: Show version information

### Configuration

Edit the configuration file:
```bash
sudo nano /etc/ddotlp/ddotlp.conf
```

Available settings:
- `IDLE_TIMEOUT`: Time in seconds before switching to battery mode
- `CPU_THRESHOLD`: CPU usage percentage threshold for activity detection
- `LOG_LEVEL`: Logging verbosity (DEBUG, INFO, WARNING, ERROR)
- `LOG_FILE`: Path to log file
- `AUTO_SWITCH_ENABLED`: Enable/disable automatic switching

## How It Works

1. **Activity Monitoring**: The service monitors CPU usage by reading `/proc/stat` to detect system activity
2. **Idle Detection**: When CPU usage stays below the configured threshold for the specified timeout period, the system is considered idle
3. **TLP Management**:
   - **Active State**: Executes `sudo tlp start` for performance mode when CPU usage is above threshold
   - **Idle State**: Executes `sudo tlp bat` for battery saving mode when CPU usage is low
4. **State Transitions**: Only switches modes when the state actually changes to avoid unnecessary TLP calls

## Advantages of CPU-Based Monitoring

- **No X11 Dependencies**: Works on headless servers, containers, and systems without X11
- **Universal Compatibility**: Works regardless of display server (X11, Wayland, or headless)
- **System-Wide Activity**: Detects any CPU activity, not just user input
- **Lightweight**: Minimal system resource usage
- **Server Friendly**: Suitable for both desktop and server environments

## Troubleshooting

### Check Service Status
```bash
sudo systemctl status ddotlp
```

### View Logs
```bash
sudo tail -f /var/log/ddotlp.log
```

### Common Issues

1. **Permission Denied**: Ensure the service is running as root
2. **CPU Monitoring Failed**: Check if `/proc/stat` is readable
3. **TLP Not Found**: Install TLP package

### Tuning CPU Threshold

The default CPU threshold is 15%. You may need to adjust this based on your system:
- **Lower values (5-10%)**: More sensitive, switches to performance mode with light activity
- **Higher values (20-30%)**: Less sensitive, requires more CPU activity to stay in performance mode

### Debug Mode

Run in foreground with debug output:
```bash
sudo ddotlp
```

## Security

The service runs with minimal privileges and includes security hardening:
- Runs in a restricted environment
- Protected system directories
- Private temporary directory
- No new privileges allowed

## License

This project is open source. Please check the LICENSE file for details.

## Contributing

Contributions are welcome! Please submit pull requests or open issues for bugs and feature requests.
