/*
 * ddotlp - Dynamic TLP Power Management Daemon
 *
 * Automatically manages TLP power settings based on system load monitoring
 *
 * Copyright (c) 2025 ddosoft (www.ddosoft.com)
 *
 * This program automatically switches between TLP AC and battery modes
 * based on 1-minute and 5-minute load average monitoring, providing
 * intelligent power management for Linux systems.
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
              << "Automatically switches between TLP AC and battery modes based on system load.\n";
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

    // Set up activity callback
    activityMonitor.setActivityCallback([&tlpManager](bool isActive) {
        if (isActive) {
            // System is active (1-min load > 15%), switch to TLP AC mode (tlp ac)
            tlpManager.setACMode();
        } else {
            // System is idle (5-min load <= 15%), switch to TLP battery mode (tlp bat)
            tlpManager.setBatteryMode();
        }
    });

    // Start activity monitoring
    if (!activityMonitor.start()) {
        Logger::error("Failed to start activity monitor");
        return 1;
    }

    Logger::info("ddotlp service started successfully");

    // Main loop - just keep the process alive while monitoring runs in background
    while (Daemon::shouldRun()) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
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
