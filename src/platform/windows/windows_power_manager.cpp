#include "platform/ipower_manager.h"
#include "logger.h"
#include <memory>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <cstdio>
#endif

/**
 * Windows-specific power manager implementation
 * Uses Windows powercfg command for power plan management
 */
class WindowsPowerManager : public IPowerManager {
public:
    WindowsPowerManager() {
        Logger::info("Windows Power Manager initialized");
        
        // Verify powercfg is available
        if (!isAvailable()) {
            Logger::warning("powercfg command not available - power management may not work");
        }
    }

    virtual ~WindowsPowerManager() = default;

    /**
     * Set system to high performance mode using Windows Power Plans
     * @return true if successful
     */
    bool setPerformanceMode() override {
        const std::string currentMode = getCurrentMode();
        if (currentMode == "performance") {
            Logger::info("Already in performance mode");
            return true;
        }

        Logger::info("Switching to Windows High Performance power plan");
        
        // Execute powercfg command to set High Performance power plan
        const std::string command = "powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Successfully switched to High Performance power plan");
            return true;
        } else {
            Logger::error("Failed to switch to High Performance power plan");
            return false;
        }
    }

    /**
     * Set system to power saving mode using Windows Power Plans
     * @return true if successful
     */
    bool setPowerSavingMode() override {
        const std::string currentMode = getCurrentMode();
        if (currentMode == "powersaving") {
            Logger::info("Already in power saving mode");
            return true;
        }

        Logger::info("Switching to Windows Power Saver power plan");
        
        // Execute powercfg command to set Power Saver power plan
        const std::string command = "powercfg /setactive a1841308-3541-4fab-bc81-f71556f20b4a";
        const bool success = executeCommand(command);
        
        if (success) {
            Logger::info("Successfully switched to Power Saver power plan");
            return true;
        } else {
            Logger::error("Failed to switch to Power Saver power plan");
            return false;
        }
    }

    /**
     * Get current Windows power plan
     * @return "performance", "powersaving", or "unknown"
     */
    std::string getCurrentMode() override {
        Logger::debug("Getting current Windows power plan");
        
        // Execute powercfg command to get active scheme
        const std::string command = "powercfg /getactivescheme";
        const std::string output = executeCommandWithOutput(command);
        
        if (output.empty()) {
            Logger::debug("Failed to get current power plan");
            return "unknown";
        }
        
        // Parse output to determine current mode
        // High Performance GUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
        // Power Saver GUID: a1841308-3541-4fab-bc81-f71556f20b4a
        
        if (output.find("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c") != std::string::npos ||
            output.find("High performance") != std::string::npos) {
            Logger::debug("Current power plan: High Performance");
            return "performance";
        } else if (output.find("a1841308-3541-4fab-bc81-f71556f20b4a") != std::string::npos ||
                   output.find("Power saver") != std::string::npos) {
            Logger::debug("Current power plan: Power Saver");
            return "powersaving";
        } else {
            Logger::debug("Current power plan: Unknown/Other");
            return "unknown";
        }
    }

    /**
     * Check if Windows power management is available
     * @return true if power plans are accessible
     */
    bool isAvailable() override {
        Logger::debug("Checking if Windows power management is available");
        
        // Test if powercfg command is available
        const std::string command = "powercfg /list";
        const std::string output = executeCommandWithOutput(command);
        
        if (!output.empty() && output.find("GUID") != std::string::npos) {
            Logger::debug("Windows power management is available");
            return true;
        } else {
            Logger::warning("Windows power management not available - powercfg command failed");
            return false;
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
        if (command.find("/getactivescheme") != std::string::npos) {
            return "Power Scheme GUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c  (High performance)";
        } else if (command.find("/list") != std::string::npos) {
            return "Existing Power Schemes (* Active)\nGUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c  (High performance)";
        }
        
        return "Mock Windows command output";
#endif
    }
};

// Factory function for creating Windows power manager
std::unique_ptr<IPowerManager> createWindowsPowerManager() {
    return std::make_unique<WindowsPowerManager>();
}
