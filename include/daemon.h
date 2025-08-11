#pragma once

#include "platform/platform_factory.h"
#include <memory>

class Daemon {
public:
    // Initialize the daemon (creates platform-specific implementation)
    static bool initialize();

    // Daemonize the process
    static bool daemonize();

    // Setup signal handlers
    static void setupSignalHandlers();

    // Check if daemon should continue running
    static bool shouldRun();

    // Wait for termination signal (blocks until signal received)
    static void waitForSignal();

    // Cleanup daemon resources
    static void cleanup();

private:
    static std::unique_ptr<IDaemon> s_daemon;
};
