#include "platform/idaemon.h"
#include "logger.h"
#include <memory>
#include <windows.h>
#include <io.h>

class WindowsDaemon : public IDaemon {
public:
    bool daemonize() override;
    void setupSignalHandlers() override;
    bool shouldRun() override;
};

bool WindowsDaemon::daemonize() {
    // On Windows, we don't need to fork/daemonize in the traditional Unix sense
    // The service framework handles this for us
    Logger::info("Running on Windows - service mode");
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
