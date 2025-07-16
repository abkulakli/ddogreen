#include "daemon.h"
#include "logger.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

volatile bool Daemon::s_running = true;

bool Daemon::daemonize() {
    // Fork the first time
    pid_t pid = fork();

    if (pid < 0) {
        Logger::error("First fork failed");
        return false;
    }

    if (pid > 0) {
        // Parent process exits
        exit(0);
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
    // Write PID file BEFORE closing file descriptors
    std::ofstream pidFile("/run/ddops.pid");
    if (pidFile.is_open()) {
        pidFile << getpid() << std::endl;
        pidFile.close();
        Logger::info("PID file written: /run/ddops.pid with PID: " + std::to_string(getpid()));
    } else {
        Logger::error("Failed to write PID file: /run/ddops.pid");
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

void Daemon::setupSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
}

void Daemon::signalHandler(int signal) {
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

bool Daemon::shouldRun() {
    return s_running;
}
