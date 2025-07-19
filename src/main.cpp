/*
 * ddogreen - Intelligent Green Power Management for Sustainable Computing
 *
 * Automatically manages TLP power settings based on system load monitoring
 *
 * Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)
 *
 * This program automatically switches between performance and power-saving modes
 * based on load average monitoring, providing
 * intelligent power management for sustainable computing on PCs and laptops.
 */

#include "activity_monitor.h"
#include "daemon.h"
#include "logger.h"
#include "config.h"
#include "platform/platform_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --daemon           Run as daemon\n"
              << "  -c, --config PATH      Use custom configuration file\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n"
              << "  -i, --install          Install system service\n"
              << "  -u, --uninstall        Uninstall system service\n"
              << "\n"
              << "Service Management:\n"
              << "  Install:   sudo " << programName << " --install (or -i)\n"
              << "  Uninstall: sudo " << programName << " --uninstall (or -u)\n"
              << "\n"
              << "Configuration:\n"
              << "  Default:   /etc/ddogreen/ddogreen.conf\n"
              << "  Custom:    " << programName << " --config /path/to/config.conf\n"
              << "\n"
              << "Automatically switches between performance and power-saving modes based on system load.\n";
}

void printVersion() {
    std::cout << "ddogreen version 0.2.0\n"
              << "Intelligent Green Power Management for Sustainable Computing\n"
              << "Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)\n";
}

int installService() {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (!platformUtils || !platformUtils->isAvailable()) {
        std::cerr << "Platform utilities are not available" << std::endl;
        return 1;
    }

    // Check if running with required privileges
    if (!platformUtils->hasRequiredPrivileges()) {
        std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
        return 1;
    }

    // Get the full path to this executable
    std::string currentExecutablePath = platformUtils->getExecutablePath();
    if (currentExecutablePath.empty()) {
        std::cerr << "Failed to determine executable path" << std::endl;
        return 1;
    }

    auto serviceManager = PlatformFactory::createServiceManager();
    if (!serviceManager || !serviceManager->isAvailable()) {
        std::cerr << "Service management is not available on this platform" << std::endl;
        return 1;
    }

    std::string serviceName = "ddogreen";
    std::string description = "DDOSoft ddogreen - Intelligent Green Power Management for Sustainable Computing";

    std::cout << "Installing system service..." << std::endl;
    if (serviceManager->installService(serviceName, currentExecutablePath, description)) {
        std::cout << "Service installed, enabled, and started successfully!" << std::endl;
        std::cout << "ddogreen is now running and will auto-start on boot." << std::endl;
        std::cout << "To view logs, check your system log viewer." << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to install service" << std::endl;
        return 1;
    }
}

int uninstallService() {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (!platformUtils || !platformUtils->isAvailable()) {
        std::cerr << "Platform utilities are not available" << std::endl;
        return 1;
    }

    // Check if running with required privileges
    if (!platformUtils->hasRequiredPrivileges()) {
        std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
        return 1;
    }

    auto serviceManager = PlatformFactory::createServiceManager();
    if (!serviceManager || !serviceManager->isAvailable()) {
        std::cerr << "Service management is not available on this platform" << std::endl;
        return 1;
    }

    std::string serviceName = "ddogreen";

    std::cout << "Uninstalling system service..." << std::endl;
    if (serviceManager->uninstallService(serviceName)) {
        std::cout << "Service uninstalled successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to uninstall service" << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    // Create platform utilities
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (!platformUtils || !platformUtils->isAvailable()) {
        std::cerr << "Platform utilities are not available on this system" << std::endl;
        return 1;
    }

    // Parse command line arguments
    ParsedArgs args = platformUtils->parseCommandLine(argc, argv);
    
    if (args.hasUnknownOptions) {
        std::cerr << "Unknown option: " << args.unknownOption << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    if (args.showHelp) {
        printUsage(argv[0]);
        return 0;
    }
    
    if (args.showVersion) {
        printVersion();
        return 0;
    }
    
    if (args.install) {
        return installService();
    }
    
    if (args.uninstall) {
        return uninstallService();
    }

    // Check if running with required privileges
    if (!platformUtils->hasRequiredPrivileges()) {
        std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
        return 1;
    }

    // Initialize logger with console output when not running as daemon
    std::string logPath = platformUtils->getDefaultLogPath();
    Logger::init(logPath, !args.runAsDaemon);

    // Always convert relative config paths to absolute paths for consistency
    // This prevents issues with working directory changes and provides clear path resolution
    if (!args.configPath.empty() && args.configPath[0] != '/') {
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            std::string tempPath = std::string(cwd) + "/" + args.configPath;
            free(cwd);
            
            // Use realpath to resolve all . and .. components and get canonical path
            char* resolvedPath = realpath(tempPath.c_str(), nullptr);
            if (resolvedPath) {
                args.configPath = std::string(resolvedPath);
                free(resolvedPath);
                Logger::info("Converted relative config path to absolute: " + args.configPath);
            } else {
                // If realpath fails, use the simple concatenation as fallback
                args.configPath = tempPath;
                Logger::warning("Could not resolve path components, using: " + args.configPath);
            }
        } else {
            Logger::warning("Failed to get current working directory, using config path as-is");
        }
    }

    // Initialize daemon
    if (!Daemon::initialize()) {
        Logger::error("Failed to initialize daemon");
        return 1;
    }

    if (args.runAsDaemon) {
        if (!Daemon::daemonize()) {
            Logger::error("Failed to daemonize");
            return 1;
        }
    }

    // Setup signal handlers
    Daemon::setupSignalHandlers();

    // Load configuration
    Config config;
    std::string configPath = args.configPath.empty() ? Config::getDefaultConfigPath() : args.configPath;
    
    // Load configuration - application fails if config file doesn't exist or has errors
    if (!config.loadFromFile(configPath)) {
        return 1;
    }

    // Initialize components
    ActivityMonitor activityMonitor;
    auto powerManager = PlatformFactory::createPowerManager();

    // Check if power management is available
    if (!powerManager || !powerManager->isAvailable()) {
        Logger::error("Power management is not installed or not available");
        return 1;
    }

    // Configure activity monitor with settings from config
    activityMonitor.setLoadThresholds(config.getHighPerformanceThreshold(), config.getPowerSaveThreshold());
    activityMonitor.setMonitoringFrequency(config.getMonitoringFrequency());

    // Set up activity callback
    activityMonitor.setActivityCallback([&powerManager](bool isActive) {
        if (isActive) {
            // System is active (load above high performance threshold), switch to performance mode
            powerManager->setPerformanceMode();
        } else {
            // System is idle (load below power save threshold), switch to power saving mode
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
    Daemon::cleanup();

    // Remove PID file
    if (args.runAsDaemon) {
        std::string pidPath = platformUtils->getDefaultPidPath();
        std::remove(pidPath.c_str());
    }

    return 0;
}
