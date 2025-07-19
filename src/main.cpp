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
#include "platform/platform_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>

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

    if (args.runAsDaemon) {
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
    if (!powerManager || !powerManager->isAvailable()) {
        Logger::error("Power management is not installed or not available");
        return 1;
    }

    // Set up activity callback
    activityMonitor.setActivityCallback([&powerManager](bool isActive) {
        if (isActive) {
            // System is active (load > 70%), switch to performance mode
            powerManager->setPerformanceMode();
        } else {
            // System is idle (load < 30%), switch to power saving mode
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
    if (args.runAsDaemon) {
        std::string pidPath = platformUtils->getDefaultPidPath();
        std::remove(pidPath.c_str());
    }

    return 0;
}
