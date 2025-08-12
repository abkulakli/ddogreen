#include "platform/isignal_handler.h"
#include "logger.h"
#include <csignal>
#include <atomic>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <memory>

class MacOSSignalHandler : public ISignalHandler {
public:
    MacOSSignalHandler();
    ~MacOSSignalHandler() override = default;

    void setupSignalHandlers() override;
    bool shouldRun() override;
    void waitForSignal() override;

private:
    static std::atomic<bool> s_running;
    static void signalHandler(int signal);
};

// Static member definition
std::atomic<bool> MacOSSignalHandler::s_running{true};

MacOSSignalHandler::MacOSSignalHandler() {
    // Constructor
}

void MacOSSignalHandler::setupSignalHandlers() {
    Logger::debug("Setting up macOS signal handlers");
    
    // Set up Unix signal handlers (same as Linux)
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    
    Logger::debug("macOS signal handlers configured successfully");
}

bool MacOSSignalHandler::shouldRun() {
    return s_running.load();
}

void MacOSSignalHandler::waitForSignal() {
    // Wait for termination signal (no busy waiting)
    while (s_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void MacOSSignalHandler::signalHandler(int signal) {
    Logger::info("Received macOS termination signal: " + std::to_string(signal));
    s_running = false;
}

// Factory function
std::unique_ptr<ISignalHandler> createMacOSSignalHandler() {
    return std::make_unique<MacOSSignalHandler>();
}
