#include "platform/idaemon.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

class LinuxDaemon : public IDaemon {
public:
    bool daemonize() override;
    void setupSignalHandlers() override;
    bool shouldRun() override;

private:
    static void signalHandler(int signal);
};

bool LinuxDaemon::daemonize() {
    // Fork the first time
    pid_t pid = fork();

    if (pid < 0) {
        Logger::error("First fork failed");
        return false;
    }

    if (pid > 0) {
        // Parent process: wait for child to signal readiness via PID file
        // Check for PID file creation with timeout
        int attempts = 0;
        const int max_attempts = 100; // 1 second total timeout

        while (attempts < max_attempts) {
            std::ifstream pidFile("/run/ddogreen.pid");
            if (pidFile.is_open()) {
                pidFile.close();
                Logger::info("Parent process: PID file detected, exiting");
                exit(0);
            }
            usleep(10000); // 10ms delay
            attempts++;
        }

        Logger::error("Parent process: Timeout waiting for PID file");
        exit(1);
    }

    // Child continues
    // Create a new session
    if (setsid() < 0) {
        Logger::error("setsid failed");
        return false;
    }

    // Fork the second time
    pid = fork();

    if (pid < 0) {
        Logger::error("Second fork failed");
        return false;
    }

    if (pid > 0) {
        // First child exits
        exit(0);
    }

    // Second child continues as daemon
    // Write PID file to signal parent we're ready
    std::ofstream pidFile("/run/ddogreen.pid");
    if (pidFile.is_open()) {
        pidFile << getpid() << std::endl;
        pidFile.flush(); // Ensure data is written to disk
        pidFile.close();

        // Sync to ensure write is complete
        sync();

        Logger::info("PID file written by daemon: /run/ddogreen.pid with PID: " + std::to_string(getpid()));
    } else {
        Logger::error("Failed to write PID file: /run/ddogreen.pid");
        return false;
    }

    // Change working directory to root
    if (chdir("/") < 0) {
        Logger::error("chdir to / failed");
        return false;
    }

    // Set file permissions
    umask(0);

    // Close all open file descriptors
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }

    // Redirect stdin, stdout, stderr to /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > 2) {
            close(fd);
        }
    }

    Logger::info("Daemon started with PID: " + std::to_string(getpid()));
    return true;
}

void LinuxDaemon::setupSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
}

void LinuxDaemon::signalHandler(int signal) {
    switch (signal) {
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
            Logger::info("Received termination signal: " + std::to_string(signal));
            s_running = false;
            break;
        case SIGHUP:
            Logger::info("Received SIGHUP (configuration reload not supported - using hardcoded settings)");
            break;
        default:
            Logger::warning("Received unknown signal: " + std::to_string(signal));
            break;
    }
}

bool LinuxDaemon::shouldRun() {
    return s_running;
}

// Factory function
std::unique_ptr<IDaemon> createLinuxDaemon() {
    return std::make_unique<LinuxDaemon>();
}
