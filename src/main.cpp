/*
 * DDOGreen - Intelligent Green Power Management for Sustainable Computing
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
#include <cstdlib>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --daemon           Run as daemon\n"
              << "  -c, --config PATH      Use custom configuration file\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n"
              << "\n"
              << "Configuration:\n"
              << "  Default:   /etc/ddogreen/ddogreen.conf\n"
              << "  Custom:    " << programName << " --config /path/to/config.conf\n"
              << "\n"
              << "Automatically switches between performance and power-saving modes based on system load.\n"
              << "\n"
              << "Installation:\n"
              << "  Use your package manager or the provided installer scripts in the packaging/ directory.\n";
}

void printVersion() {
    std::cout << "DDOGreen version " << DDOGREEN_VERSION << "\n"
              << "Intelligent Green Power Management for Sustainable Computing\n"
              << "Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)\n";
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

    // Check if running with required privileges
    if (!platformUtils->hasRequiredPrivileges()) {
        std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
        return 1;
    }

    // Initialize logger with console output when not running as daemon
    std::string logPath = platformUtils->getDefaultLogPath();
    Logger::init(logPath, !args.runAsDaemon);

    // Log version information
    Logger::info("Starting DDOGreen - Intelligent Green Power Management");
    Logger::info("Version: " + std::string(DDOGREEN_VERSION));
    Logger::info("Copyright (c) 2025 DDOSoft Sustainability Solutions");

    // Always convert relative config paths to absolute paths for consistency
    // This prevents issues with working directory changes and provides clear path resolution
    if (!args.configPath.empty()) {
        std::string resolvedPath = platformUtils->resolveAbsolutePath(args.configPath);
        if (resolvedPath != args.configPath) {
            args.configPath = resolvedPath;
            Logger::info("Converted relative config path to absolute: " + args.configPath);
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
    
    Logger::info("Loading configuration from: " + configPath);
    
    // Load configuration - application fails if config file doesn't exist or has errors
    if (!config.loadFromFile(configPath)) {
        Logger::error("Failed to load configuration file: " + configPath);
        return 1;
    }
    
    Logger::info("Configuration loaded successfully");

    // Initialize components
    ActivityMonitor activityMonitor;
    auto powerManager = PlatformFactory::createPowerManager();

    // Check if power management is available
    Logger::info("Checking power management availability...");
    if (!powerManager || !powerManager->isAvailable()) {
        Logger::error("Power management is not installed or not available");
        Logger::error("Please install TLP: sudo apt install tlp (Debian/Ubuntu) or sudo dnf install tlp (Fedora)");
        return 1;
    }
    Logger::info("Power management system (TLP) is available");

    // Configure activity monitor with settings from config
    Logger::info("Configuring activity monitor...");
    activityMonitor.setLoadThresholds(config.getHighPerformanceThreshold(), config.getPowerSaveThreshold());
    activityMonitor.setMonitoringFrequency(config.getMonitoringFrequency());
    
    Logger::info("High performance threshold: " + std::to_string(config.getHighPerformanceThreshold()));
    Logger::info("Power save threshold: " + std::to_string(config.getPowerSaveThreshold()));
    Logger::info("Monitoring frequency: " + std::to_string(config.getMonitoringFrequency()) + " seconds");

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

    Logger::info("DDOGreen service started successfully");
    Logger::info("Version: " + std::string(DDOGREEN_VERSION));
    Logger::info("Copyright (c) 2025 DDOSoft Sustainability Solutions (www.ddosoft.com)");

    // Main loop - just keep the process alive while monitoring runs in background
    while (Daemon::shouldRun()) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    // Cleanup
    Logger::info("Shutting down DDOGreen service");
    activityMonitor.stop();
    Daemon::cleanup();

    // Remove PID file
    if (args.runAsDaemon) {
        std::string pidPath = platformUtils->getDefaultPidPath();
        std::remove(pidPath.c_str());
    }

    return 0;
}
