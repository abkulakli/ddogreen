#include "platform/iservice_manager.h"
#include "logger.h"
#include <memory>
#include <string>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <cstdio>
#endif

/**
 * Windows-specific service manager implementation
 * Uses Windows Service Control Manager for service management
 */
class WindowsServiceManager : public IServiceManager {
public:
    WindowsServiceManager() = default;
    virtual ~WindowsServiceManager() = default;

    /**
     * Install Windows service
     * @param serviceName name of the service
     * @param executablePath full path to executable
     * @param description service description
     * @return true if successful
     */
    bool installService(const std::string& serviceName, 
                       const std::string& executablePath,
                       const std::string& description) override {
        
        Logger::info("Installing Windows service: " + serviceName);
        Logger::info("Executable path: " + executablePath);
        Logger::info("Description: " + description);
        
        // Build sc create command
        std::ostringstream cmd;
        cmd << "sc create \"" << serviceName << "\" "
            << "binPath= \"" << executablePath << " --daemon\" "
            << "DisplayName= \"" << description << "\" "
            << "start= auto "
            << "type= own";
        
        const bool success = executeCommand(cmd.str());
        
        if (success) {
            Logger::info("Windows service installed successfully");
            Logger::info("To start the service: sc start " + serviceName);
            Logger::info("To enable auto-start: sc config " + serviceName + " start= auto");
        } else {
            Logger::error("Failed to install Windows service");
        }
        
        return success;
    }

    /**
     * Uninstall Windows service
     * @param serviceName name of the service to uninstall
     * @return true if successful
     */
    bool uninstallService(const std::string& serviceName) override {
        Logger::info("Uninstalling Windows service: " + serviceName);
        
        // Stop service first
        stopService(serviceName);
        
        // Delete the service
        const std::string command = "sc delete \"" + serviceName + "\"";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Windows service uninstalled successfully");
        } else {
            Logger::error("Failed to uninstall Windows service");
        }
        
        return success;
    }

    /**
     * Start Windows service
     * @param serviceName name of the service to start
     * @return true if successful
     */
    bool startService(const std::string& serviceName) override {
        Logger::info("Starting Windows service: " + serviceName);
        
        const std::string command = "sc start \"" + serviceName + "\"";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Windows service started successfully");
        } else {
            Logger::error("Failed to start Windows service");
        }
        
        return success;
    }

    /**
     * Stop Windows service
     * @param serviceName name of the service to stop
     * @return true if successful
     */
    bool stopService(const std::string& serviceName) override {
        Logger::info("Stopping Windows service: " + serviceName);
        
        const std::string command = "sc stop \"" + serviceName + "\"";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Windows service stopped successfully");
        } else {
            Logger::warning("Failed to stop Windows service (may not be running)");
        }
        
        return success;
    }

    /**
     * Enable Windows service for automatic startup
     * @param serviceName name of the service to enable
     * @return true if successful
     */
    bool enableService(const std::string& serviceName) override {
        Logger::info("Enabling Windows service for automatic startup: " + serviceName);
        
        const std::string command = "sc config \"" + serviceName + "\" start= auto";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Windows service enabled for automatic startup");
        } else {
            Logger::error("Failed to enable Windows service for automatic startup");
        }
        
        return success;
    }

    /**
     * Disable Windows service from starting automatically
     * @param serviceName name of the service to disable
     * @return true if successful
     */
    bool disableService(const std::string& serviceName) override {
        Logger::info("Disabling Windows service from automatic startup: " + serviceName);
        
        const std::string command = "sc config \"" + serviceName + "\" start= disabled";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Windows service disabled from automatic startup");
        } else {
            Logger::error("Failed to disable Windows service from automatic startup");
        }
        
        return success;
    }

    /**
     * Check if Windows service management is available
     * @return true if service management is supported
     */
    bool isAvailable() override {
        Logger::debug("Checking if Windows service management is available");
        
        // Test if sc command is available
        const std::string command = "sc query";
        const std::string output = executeCommandWithOutput(command);
        
        if (!output.empty()) {
            Logger::debug("Windows service management is available");
            return true;
        } else {
            Logger::warning("Windows service management not available - sc command failed");
            return false;
        }
    }

    /**
     * Get Windows service status
     * @param serviceName name of the service
     * @return status string ("running", "stopped", "unknown")
     */
    std::string getServiceStatus(const std::string& serviceName) override {
        Logger::debug("Getting status for Windows service: " + serviceName);
        
        const std::string command = "sc query \"" + serviceName + "\"";
        const std::string output = executeCommandWithOutput(command);
        
        if (output.empty()) {
            Logger::debug("Failed to query service status");
            return "unknown";
        }
        
        // Parse output to determine status
        if (output.find("RUNNING") != std::string::npos) {
            Logger::debug("Service status: running");
            return "running";
        } else if (output.find("STOPPED") != std::string::npos) {
            Logger::debug("Service status: stopped");
            return "stopped";
        } else {
            Logger::debug("Service status: unknown");
            return "unknown";
        }
    }

private:
    /**
     * Execute a Windows command
     * @param command Command to execute
     * @return true if command executed successfully (exit code 0)
     */
    bool executeCommand(const std::string& command) {
        Logger::debug("Executing command: " + command);
        
#ifdef _WIN32
        // Use Windows system() call
        const int result = system(command.c_str());
        const bool success = (result == 0);
        
        if (success) {
            Logger::debug("Command executed successfully");
        } else {
            Logger::error("Command failed with exit code: " + std::to_string(result));
        }
        
        return success;
#else
        // On non-Windows platforms, just log what would happen (for cross-compilation)
        Logger::info("CROSS-PLATFORM: Would execute on Windows: " + command);
        return true; // Assume success for cross-compilation
#endif
    }
    
    /**
     * Execute a Windows command and capture output
     * @param command Command to execute
     * @return Command output as string, empty if failed
     */
    std::string executeCommandWithOutput(const std::string& command) {
        Logger::debug("Executing command with output: " + command);
        
#ifdef _WIN32
        std::string result;
        
        // Use _popen to execute command and capture output
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            Logger::error("Failed to execute command: " + command);
            return "";
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
        const int exitCode = _pclose(pipe);
        if (exitCode != 0) {
            Logger::error("Command failed with exit code: " + std::to_string(exitCode));
            return "";
        }
        
        Logger::debug("Command output captured successfully");
        return result;
#else
        // On non-Windows platforms, return mock output (for cross-compilation)
        Logger::info("CROSS-PLATFORM: Would execute on Windows: " + command);
        
        // Return mock output for testing
        if (command.find("sc query") != std::string::npos) {
            return "SERVICE_NAME: testservice\nSTATE: 4 RUNNING";
        }
        
        return "Mock Windows command output";
#endif
    }
};

// Factory function for creating Windows service manager
std::unique_ptr<IServiceManager> createWindowsServiceManager() {
    return std::make_unique<WindowsServiceManager>();
}
