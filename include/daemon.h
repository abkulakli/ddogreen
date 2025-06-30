#pragma once

class Daemon {
public:
    // Daemonize the process
    static bool daemonize();

    // Setup signal handlers
    static void setupSignalHandlers();

    // Signal handler function
    static void signalHandler(int signal);

    // Check if daemon should continue running
    static bool shouldRun();

private:
    static volatile bool s_running;
};
