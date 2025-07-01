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
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <getopt.h>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --daemon      Run as daemon\n"
              << "  -h, --help        Show this help message\n"
              << "  -v, --version     Show version information\n"
              << "\n"
              << "Hardcoded settings:\n"
              << "  - Load threshold: 0.15 (15% of one CPU core)\n"
              << "  - Check interval: 60 seconds (1 minute)\n"
              << "  - Decision based on: Most recent 1-minute load average\n";
}

void printVersion() {
    std::cout << "ddotlp version 1.0.0\n"
              << "Dynamic TLP power management daemon\n"
              << "Copyright (c) 2025 ddosoft (www.ddosoft.com)\n";
}

int main(int argc, char* argv[]) {
    bool runAsDaemon = false;

    // Parse command line arguments (simplified)
    static struct option long_options[] = {
        {"daemon",   no_argument, 0, 'd'},
        {"help",     no_argument, 0, 'h'},
        {"version",  no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "dhv", long_options, &option_index)) != -1) {
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
            case '?':
                printUsage(argv[0]);
                return 1;
            default:
                break;
        }
    }

    // Initialize logger first
    Logger::init();

    // Use hardcoded default values for simplicity
    const double loadThreshold = 0.15; // 15% load average (0.15 load)

    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root to execute TLP commands." << std::endl;
        return 1;
    }

    // Initialize logger with default log file
    Logger::init("/var/log/ddotlp.log");

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

    // Set hardcoded values
    activityMonitor.setLoadThreshold(loadThreshold);

    // Set up activity callback
    activityMonitor.setActivityCallback([&tlpManager](bool isActive) {
        if (isActive) {
            // System is active, switch to TLP auto mode (tlp start)
            tlpManager.setAutoMode();
        } else {
            // System is idle, switch to TLP battery mode (tlp bat)
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
        unlink("/run/ddotlp.pid");
    }

    return 0;
}
