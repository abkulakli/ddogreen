/*
 * ddotlp - Dynamic TLP Power Management Daemon
 *
 * Automatically manages TLP power settings based on system activity
 *
 * Copyright (c) 2025 ddosoft (www.ddosoft.com)
 *
 * This program automatically switches between TLP performance and battery
 * modes based on CPU usage monitoring, providing intelligent power management
 * for Linux systems.
 */

#include "activity_monitor.h"
#include "tlp_manager.h"
#include "daemon.h"
#include "logger.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <getopt.h>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --daemon      Run as daemon\n"
              << "  -t, --timeout     Idle timeout in seconds (default: 300)\n"
              << "  -c, --cpu         CPU threshold for activity detection (default: 15.0)\n"
              << "  -h, --help        Show this help message\n"
              << "  -v, --version     Show version information\n";
}

void printVersion() {
    std::cout << "ddotlp version 1.0.0\n"
              << "Dynamic TLP power management daemon\n"
              << "Copyright (c) 2025 ddosoft (www.ddosoft.com)\n";
}

int main(int argc, char* argv[]) {
    bool runAsDaemon = false;
    int idleTimeout = Config::DEFAULT_IDLE_TIMEOUT;
    double cpuThreshold = 15.0;  // Default 15% CPU threshold

    // Parse command line arguments
    static struct option long_options[] = {
        {"daemon",   no_argument,       0, 'd'},
        {"timeout",  required_argument, 0, 't'},
        {"cpu",      required_argument, 0, 'c'},
        {"help",     no_argument,       0, 'h'},
        {"version",  no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "dt:c:hv", long_options, &option_index)) != -1) {
        switch (c) {
            case 'd':
                runAsDaemon = true;
                break;
            case 't':
                idleTimeout = std::atoi(optarg);
                if (idleTimeout <= 0) {
                    std::cerr << "Invalid timeout value: " << optarg << std::endl;
                    return 1;
                }
                break;
            case 'c':
                cpuThreshold = std::atof(optarg);
                if (cpuThreshold < 0.0 || cpuThreshold > 100.0) {
                    std::cerr << "Invalid CPU threshold value: " << optarg << std::endl;
                    return 1;
                }
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'v':
                printVersion();
                return 0;
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

    // Initialize logger
    Logger::init(Config::LOG_FILE);

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
    TLPManager tlpManager;

    // Check if TLP is available
    if (!tlpManager.isTLPAvailable()) {
        Logger::error("TLP is not installed or not available");
        return 1;
    }

    // Set idle timeout and CPU threshold
    activityMonitor.setIdleTimeout(idleTimeout);
    activityMonitor.setCpuThreshold(cpuThreshold);

    // Set up activity callback
    activityMonitor.setActivityCallback([&tlpManager](bool isActive) {
        if (isActive) {
            // User is active, switch to performance mode
            tlpManager.setPerformanceMode();
        } else {
            // User is idle, switch to battery mode
            tlpManager.setBatteryMode();
        }
    });

    // Start activity monitoring
    if (!activityMonitor.start()) {
        Logger::error("Failed to start activity monitor");
        return 1;
    }

    Logger::info("ddotlp service started successfully");

    // Main loop
    while (Daemon::shouldRun()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup
    Logger::info("Shutting down ddotlp service");
    activityMonitor.stop();

    // Remove PID file
    if (runAsDaemon) {
        unlink(Config::PID_FILE);
    }

    return 0;
}
