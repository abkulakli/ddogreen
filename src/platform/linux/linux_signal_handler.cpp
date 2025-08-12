#include "platform/isignal_handler.h"
#include "logger.h"
#include <csignal>
#include <atomic>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <memory>

class LinuxSignalHandler : public ISignalHandler {
public:
    LinuxSignalHandler();
    ~LinuxSignalHandler() override = default;

    void setupSignalHandlers() override;
    bool shouldRun() override;
    void waitForSignal() override;

private:
    static std::atomic<bool> s_running;
    static void signalHandler(int signal);
};

// Static member definition
std::atomic<bool> LinuxSignalHandler::s_running{true};

LinuxSignalHandler::LinuxSignalHandler() {
    // Constructor
}

void LinuxSignalHandler::setupSignalHandlers() {
    Logger::debug("Setting up Unix signal handlers");
    
    // Set up Unix signal handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    
    Logger::debug("Unix signal handlers configured successfully");
}

bool LinuxSignalHandler::shouldRun() {
    return s_running.load();
}

void LinuxSignalHandler::waitForSignal() {
    // Wait for termination signal (no busy waiting)
    while (s_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void LinuxSignalHandler::signalHandler(int signal) {
    Logger::info("Received Unix termination signal: " + std::to_string(signal));
    s_running = false;
}

// Factory function
std::unique_ptr<ISignalHandler> createLinuxSignalHandler() {
    return std::make_unique<LinuxSignalHandler>();
}
