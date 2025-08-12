#include "platform/isignal_handler.h"
#include "logger.h"
#include <windows.h>
#include <atomic>
#include <memory>

class WindowsSignalHandler : public ISignalHandler {
public:
    WindowsSignalHandler();
    ~WindowsSignalHandler() override = default;

    void setupSignalHandlers() override;
    bool shouldRun() override;
    void waitForSignal() override;

private:
    static std::atomic<bool> s_running;
    static BOOL WINAPI consoleCtrlHandler(DWORD dwCtrlType);
};

// Static member definition
std::atomic<bool> WindowsSignalHandler::s_running{true};

WindowsSignalHandler::WindowsSignalHandler() {
    // Constructor
}

void WindowsSignalHandler::setupSignalHandlers() {
    Logger::debug("Setting up Windows console control handlers");
    
    // Set up Windows console control handler
    if (!SetConsoleCtrlHandler(consoleCtrlHandler, TRUE)) {
        Logger::error("Failed to set console control handler");
    } else {
        Logger::debug("Windows signal handlers configured successfully");
    }
}

bool WindowsSignalHandler::shouldRun() {
    return s_running.load();
}

void WindowsSignalHandler::waitForSignal() {
    // Wait for termination signal (no busy waiting)
    while (s_running.load()) {
        Sleep(1000);  // Sleep for 1 second and check again
    }
}

BOOL WINAPI WindowsSignalHandler::consoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            Logger::info("Received Windows termination signal: " + std::to_string(dwCtrlType));
            s_running = false;
            return TRUE;
        default:
            return FALSE;
    }
}

// Factory function
std::unique_ptr<ISignalHandler> createWindowsSignalHandler() {
    return std::make_unique<WindowsSignalHandler>();
}
