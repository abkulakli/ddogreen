/*
 * ddogreen - Intelligent Green Power Management for Sustainable Computing
 *
 * Automatically manages TLP power settings based on system load monitoring
 *
 * Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)
 *
 * This program automatically switches between performance and power-saving modes
 * based on 1-minute and 5-minute load average monitoring, providing
 * intelligent power management for sustainable computing on PCs and laptops.
 */

#include "activity_monitor.h"
#include "daemon.h"
#include "logger.h"
#include "platform/platform_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <linux/limits.h>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --daemon           Run as daemon\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n"
              << "  -i, --install          Install system service\n"
              << "  -u, --uninstall        Uninstall system service\n"
              << "\n"
              << "Service Management:\n"
              << "  Install:   sudo " << programName << " --install (or -i)\n"
              << "  Uninstall: sudo " << programName << " --uninstall (or -u)\n"
              << "\n"
              << "Automatically switches between performance and power-saving modes based on system load.\n";
}

void printVersion() {
    std::cout << "ddogreen version 1.0.0\n"
              << "Intelligent Green Power Management for Sustainable Computing\n"
              << "Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)\n";
}

std::string getExecutablePath() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        return std::string(path);
    }
    return "";
}

int installService() {
    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "Service installation requires root privileges. Please run with sudo." << std::endl;
        return 1;
    }

    // Get the full path to this executable
    std::string currentExecutablePath = getExecutablePath();
    if (currentExecutablePath.empty()) {
        std::cerr << "Failed to determine executable path" << std::endl;
        return 1;
    }

    // Define the target installation path
    std::string targetExecutablePath = "/usr/local/bin/ddogreen";

    auto serviceManager = PlatformFactory::createServiceManager();
    if (!serviceManager->isAvailable()) {
        std::cerr << "Service management is not available on this platform" << std::endl;
        return 1;
    }

    std::cout << "Setting up directories and permissions..." << std::endl;
    
    // Copy executable to system location
    std::cout << "Installing executable to " << targetExecutablePath << "..." << std::endl;
    std::string copyCommand = "cp \"" + currentExecutablePath + "\" \"" + targetExecutablePath + "\"";
    if (system(copyCommand.c_str()) != 0) {
        std::cerr << "Failed to copy executable to " << targetExecutablePath << std::endl;
        return 1;
    }
    
    // Set proper permissions
    if (system(("chmod 755 \"" + targetExecutablePath + "\"").c_str()) != 0) {
        std::cerr << "Warning: Failed to set executable permissions" << std::endl;
    }
    
    // Create log directory and file
    if (system("mkdir -p /var/log") != 0) {
        std::cerr << "Warning: Failed to create /var/log directory" << std::endl;
    }
    if (system("touch /var/log/ddogreen.log") != 0) {
        std::cerr << "Warning: Failed to create log file" << std::endl;
    }
    if (system("chmod 644 /var/log/ddogreen.log") != 0) {
        std::cerr << "Warning: Failed to set log file permissions" << std::endl;
    }
    
    // Create config directory
    if (system("mkdir -p /etc/ddogreen") != 0) {
        std::cerr << "Warning: Failed to create config directory" << std::endl;
    }

    std::string serviceName = "ddogreen";
    std::string description = "DDOSoft ddogreen - Intelligent Green Power Management for Sustainable Computing";

    std::cout << "Installing system service..." << std::endl;
    if (serviceManager->installService(serviceName, targetExecutablePath, description)) {
        std::cout << "Service installed, enabled, and started successfully!" << std::endl;
        std::cout << "ddogreen is now running and will auto-start on boot." << std::endl;
        std::cout << "To view logs:" << std::endl;
        std::cout << "  sudo journalctl -u " << serviceName << " -f" << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to install service" << std::endl;
        return 1;
    }
}

int uninstallService() {
    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "Service uninstallation requires root privileges. Please run with sudo." << std::endl;
        return 1;
    }

    auto serviceManager = PlatformFactory::createServiceManager();
    if (!serviceManager->isAvailable()) {
        std::cerr << "Service management is not available on this platform" << std::endl;
        return 1;
    }

    std::string serviceName = "ddogreen";

    std::cout << "Uninstalling system service..." << std::endl;
    if (serviceManager->uninstallService(serviceName)) {
        std::cout << "Cleaning up files..." << std::endl;
        
        // Remove installed executable
        if (system("rm -f /usr/local/bin/ddogreen") != 0) {
            std::cerr << "Warning: Failed to remove installed executable" << std::endl;
        }
        
        // Clean up log and config files
        if (system("rm -f /var/log/ddogreen.log") != 0) {
            std::cerr << "Warning: Failed to remove log file" << std::endl;
        }
        if (system("rm -f /run/ddogreen.pid") != 0) {
            std::cerr << "Warning: Failed to remove PID file" << std::endl;
        }
        if (system("rm -rf /etc/ddogreen") != 0) {
            std::cerr << "Warning: Failed to remove config directory" << std::endl;
        }
        
        std::cout << "Service uninstalled successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to uninstall service" << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    bool runAsDaemon = false;

    // Parse command line arguments (simplified)
    static struct option long_options[] = {
        {"daemon",      no_argument, 0, 'd'},
        {"help",        no_argument, 0, 'h'},
        {"version",     no_argument, 0, 'v'},
        {"install",     no_argument, 0, 'i'},
        {"uninstall",   no_argument, 0, 'u'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "dhviu", long_options, &option_index)) != -1) {
        switch (c) {
            case 'd':
                runAsDaemon = true;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'v':
                printVersion();
                return 0;
            case 'i':  // --install
                return installService();
            case 'u':  // --uninstall
                return uninstallService();
            case '?':
                printUsage(argv[0]);
                return 1;
            default:
                break;
        }
    }

    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root to execute TLP commands." << std::endl;
        return 1;
    }

    // Initialize logger with console output when not running as daemon
    Logger::init("/var/log/ddogreen.log", !runAsDaemon);

    if (runAsDaemon) {
        if (!Daemon::daemonize()) {
            Logger::error("Failed to daemonize");
            return 1;
        }
    }

    // Setup signal handlers
    Daemon::setupSignalHandlers();

    // Initialize components
    ActivityMonitor activityMonitor;
    auto powerManager = PlatformFactory::createPowerManager();

    // Check if power management is available
    if (!powerManager->isAvailable()) {
        Logger::error("Power management (TLP) is not installed or not available");
        return 1;
    }

    // Set up activity callback
    activityMonitor.setActivityCallback([&powerManager](bool isActive) {
        if (isActive) {
            // System is active (1-min load > 10%), switch to performance mode
            powerManager->setPerformanceMode();
        } else {
            // System is idle (both 5-min AND 15-min load <= 10%), switch to power saving mode
            powerManager->setPowerSavingMode();
        }
    });

    // Start activity monitoring
    if (!activityMonitor.start()) {
        Logger::error("Failed to start activity monitor");
        return 1;
    }

    Logger::info("ddogreen service started successfully");

    // Main loop - just keep the process alive while monitoring runs in background
    while (Daemon::shouldRun()) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    // Cleanup
    Logger::info("Shutting down ddogreen service");
    activityMonitor.stop();

    // Remove PID file
    if (runAsDaemon) {
        unlink("/run/ddogreen.pid");
    }

    return 0;
}
