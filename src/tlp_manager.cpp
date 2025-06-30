#include "tlp_manager.h"
#include "logger.h"
#include "config.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

TLPManager::TLPManager() : m_currentMode("unknown") {
    if (!isTLPAvailable()) {
        Logger::warning("TLP is not available on this system");
    }
}

TLPManager::~TLPManager() {
}

bool TLPManager::setPerformanceMode() {
    if (m_currentMode == "performance") {
        return true;  // Already in performance mode
    }
    
    Logger::info("Switching to performance mode (tlp start)");
    std::string output = executeCommandWithOutput("sudo " + std::string(Config::TLP_START_COMMAND) + " 2>&1");
    
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

bool TLPManager::setBatteryMode() {
    if (m_currentMode == "battery") {
        return true;  // Already in battery mode
    }
    
    Logger::info("Switching to battery mode (tlp bat)");
    std::string output = executeCommandWithOutput("sudo " + std::string(Config::TLP_BAT_COMMAND) + " 2>&1");
    
    if (!output.empty()) {
        std::string cleanedOutput = cleanTLPOutput(output);
        if (!cleanedOutput.empty()) {
            Logger::info("TLP output: " + cleanedOutput);
        }
    }
    
    // Check if command was successful (TLP doesn't always return proper exit codes)
    if (output.find("Error") == std::string::npos && output.find("error") == std::string::npos) {
        m_currentMode = "battery";
        Logger::info("Successfully switched to battery mode");
        return true;
    } else {
        Logger::error("Failed to switch to battery mode");
        return false;
    }
}

std::string TLPManager::getCurrentMode() {
    std::string output = executeCommandWithOutput("sudo " + std::string(Config::TLP_STATUS_COMMAND));

    // Parse the output to determine current mode
    if (output.find("TLP_DEFAULT_MODE=AC") != std::string::npos ||
        output.find("Mode = AC") != std::string::npos) {
        m_currentMode = "performance";
    } else if (output.find("TLP_DEFAULT_MODE=BAT") != std::string::npos ||
               output.find("Mode = BAT") != std::string::npos) {
        m_currentMode = "battery";
    }

    return m_currentMode;
}

bool TLPManager::isTLPAvailable() {
    return executeCommand("which tlp > /dev/null 2>&1");
}

bool TLPManager::executeCommand(const std::string& command) {
    Logger::debug("Executing command: " + command);
    int result = std::system(command.c_str());
    return (result == 0);
}

std::string TLPManager::executeCommandWithOutput(const std::string& command) {
    Logger::debug("Executing command with output: " + command);

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        Logger::error("Failed to execute command: " + command);
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

std::string TLPManager::cleanTLPOutput(const std::string& output) {
    std::string cleaned = output;
    
    // Remove trailing newlines and whitespace
    while (!cleaned.empty() && (cleaned.back() == '\n' || cleaned.back() == '\r' || cleaned.back() == ' ')) {
        cleaned.pop_back();
    }
    
    // Replace multiple spaces/tabs with single space
    std::string result;
    bool lastWasSpace = false;
    for (char c : cleaned) {
        if (c == '\n' || c == '\r') {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        } else if (c == ' ' || c == '\t') {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        } else {
            result += c;
            lastWasSpace = false;
        }
    }
    
    return result;
}
