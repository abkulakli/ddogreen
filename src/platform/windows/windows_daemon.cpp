#include "platform/idaemon.h"
#include "logger.h"
#include <memory>
#include <windows.h>
#include <io.h>
#include <cstdio>

class WindowsDaemon : public IDaemon {
public:
    bool daemonize() override;
    void setupSignalHandlers() override;
    bool shouldRun() override;
};

bool WindowsDaemon::daemonize() {
    Logger::info("Daemonizing Windows process - detaching from console");
    
    // Detach from the parent console
    if (!FreeConsole()) {
        DWORD error = GetLastError();
        if (error != ERROR_INVALID_PARAMETER) {  // ERROR_INVALID_PARAMETER means no console to detach from
            Logger::error("Failed to detach from console. Error: " + std::to_string(error));
            return false;
        }
        // If no console was attached, that's fine - we're already detached
        Logger::debug("No console was attached - process already running detached");
    } else {
        Logger::debug("Successfully detached from parent console");
    }
    
    // Redirect standard streams to null to fully disconnect
    FILE* nullFile = nullptr;
    if (freopen_s(&nullFile, "NUL", "r", stdin) != 0) {
        Logger::warning("Failed to redirect stdin to NUL");
    }
    if (freopen_s(&nullFile, "NUL", "w", stdout) != 0) {
        Logger::warning("Failed to redirect stdout to NUL");
    }
    if (freopen_s(&nullFile, "NUL", "w", stderr) != 0) {
        Logger::warning("Failed to redirect stderr to NUL");
    }
    
    Logger::info("Windows daemon detached successfully - running in background");
    return true;
}

void WindowsDaemon::setupSignalHandlers() {
    // On Windows, we use SetConsoleCtrlHandler for handling termination
    SetConsoleCtrlHandler([](DWORD dwCtrlType) -> BOOL {
        switch (dwCtrlType) {
            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                Logger::info("Received Windows termination signal");
                s_running = false;
                return TRUE;
            default:
                return FALSE;
        }
    }, TRUE);
}

bool WindowsDaemon::shouldRun() {
    return s_running;
}

// Factory function
std::unique_ptr<IDaemon> createWindowsDaemon() {
    return std::make_unique<WindowsDaemon>();
}
