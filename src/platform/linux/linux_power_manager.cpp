#include "platform/ipower_manager.h"
#include "logger.h"
#include <cstdlib>
#include <memory>
#include <array>
#include <string>

/**
 * Linux-specific power manager implementation
 * Uses TLP (ThinkPad-Linux-Power) for power management
 */
class LinuxPowerManager : public IPowerManager {
public:
    LinuxPowerManager() : m_currentMode("unknown") {
        // TLP availability will be checked by the caller
    }

    virtual ~LinuxPowerManager() = default;

    /**
     * Set system to AC mode using 'tlp ac'
     * @return true if successful
     */
    bool setPerformanceMode() override {
        if (m_currentMode == "performance") {
            return true;  // Already in performance mode
        }

        Logger::info("Switching to performance mode (tlp ac)");
        std::string output = executeCommandWithOutput("tlp ac 2>&1");

        if (!output.empty()) {
            std::string cleanedOutput = cleanTLPOutput(output);
            if (!cleanedOutput.empty()) {
                Logger::info("TLP output: " + cleanedOutput);
            }
        }

        // Check if command was successful (TLP doesn't always return proper exit codes)
        if (output.find("Error") == std::string::npos && output.find("error") == std::string::npos) {
            m_currentMode = "performance";
            Logger::info("Successfully switched to performance mode");
            return true;
        } else {
            Logger::error("Failed to switch to performance mode");
            return false;
        }
    }

    /**
     * Set system to battery mode using 'tlp bat'
     * @return true if successful
     */
    bool setPowerSavingMode() override {
        if (m_currentMode == "powersaving") {
            return true;  // Already in power saving mode
        }

        Logger::info("Switching to power saving mode (tlp bat)");
        std::string output = executeCommandWithOutput("tlp bat 2>&1");

        if (!output.empty()) {
            std::string cleanedOutput = cleanTLPOutput(output);
            if (!cleanedOutput.empty()) {
                Logger::info("TLP output: " + cleanedOutput);
            }
        }

        // Check if command was successful (TLP doesn't always return proper exit codes)
        if (output.find("Error") == std::string::npos && output.find("error") == std::string::npos) {
            m_currentMode = "powersaving";
            Logger::info("Successfully switched to power saving mode");
            return true;
        } else {
            Logger::error("Failed to switch to power saving mode");
            return false;
        }
    }

    /**
     * Get current TLP mode
     * @return "performance", "powersaving", or "unknown"
     */
    std::string getCurrentMode() override {
        std::string output = executeCommandWithOutput("tlp-stat -s");

        // Parse the output to determine current mode
        // Look for "Mode" followed by "=" and then the actual mode value
        size_t modePos = output.find("Mode");
        if (modePos != std::string::npos) {
            // Find the equals sign after "Mode"
            size_t equalsPos = output.find("=", modePos);
            if (equalsPos != std::string::npos) {
                // Find the end of the line
                size_t endPos = output.find("\n", equalsPos);
                if (endPos != std::string::npos) {
                    // Extract the value after the equals sign
                    std::string modeValue = output.substr(equalsPos + 1, endPos - equalsPos - 1);

                    // Trim whitespace and extract just the mode part (before any parentheses)
                    size_t start = modeValue.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        size_t end = modeValue.find_first_of(" \t(", start);
                        if (end == std::string::npos) end = modeValue.length();

                        std::string mode = modeValue.substr(start, end - start);

                        if (mode == "AC") {
                            m_currentMode = "performance";
                        } else if (mode == "battery") {
                            m_currentMode = "powersaving";
                        }
                    }
                }
            }
        }

        // Fallback: check for older TLP_DEFAULT_MODE format
        if (m_currentMode == "unknown") {
            if (output.find("TLP_DEFAULT_MODE=AC") != std::string::npos) {
                m_currentMode = "performance";
            } else if (output.find("TLP_DEFAULT_MODE=BAT") != std::string::npos) {
                m_currentMode = "powersaving";
            }
        }

        return m_currentMode;
    }

    /**
     * Check if TLP is installed and available
     * @return true if TLP commands are accessible
     */
    bool isAvailable() override {
        return executeCommand("which tlp > /dev/null 2>&1");
    }

private:
    /**
     * Execute a command and return success status
     * @param command command to execute
     * @return true if command succeeded
     */
    bool executeCommand(const std::string& command) {
        Logger::debug("Executing command: " + command);
        int result = std::system(command.c_str());
        return (result == 0);
    }

    /**
     * Execute a command and return its output
     * @param command command to execute
     * @return command output as string
     */
    std::string executeCommandWithOutput(const std::string& command) {
        Logger::debug("Executing command with output: " + command);

        std::array<char, 128> buffer;
        std::string result;

        // Use a simple FILE* with manual cleanup to avoid template attribute warnings
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            Logger::error("Failed to execute command: " + command);
            return "";
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

        pclose(pipe);
        return result;
    }

    /**
     * Clean TLP output for logging
     * @param output raw command output
     * @return cleaned output string
     */
    std::string cleanTLPOutput(const std::string& output) {
        std::string cleaned = output;
        
        // Remove trailing newlines
        while (!cleaned.empty() && (cleaned.back() == '\n' || cleaned.back() == '\r')) {
            cleaned.pop_back();
        }
        
        // Remove any leading whitespace
        size_t start = cleaned.find_first_not_of(" \t\n\r");
        if (start != std::string::npos) {
            cleaned = cleaned.substr(start);
        }
        
        return cleaned;
    }

    std::string m_currentMode;
};

// Factory function for creating Linux power manager
std::unique_ptr<IPowerManager> createLinuxPowerManager() {
    return std::make_unique<LinuxPowerManager>();
}
