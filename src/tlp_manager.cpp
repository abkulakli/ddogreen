#include "tlp_manager.h"
#include "logger.h"
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

bool TLPManager::setAutoMode() {
    if (m_currentMode == "auto") {
        return true;  // Already in auto mode
    }

    Logger::info("Switching to auto mode (tlp start)");
    std::string output = executeCommandWithOutput("tlp start 2>&1");

    if (!output.empty()) {
        std::string cleanedOutput = cleanTLPOutput(output);
        if (!cleanedOutput.empty()) {
            Logger::info("TLP output: " + cleanedOutput);
        }
    }

    // Check if command was successful (TLP doesn't always return proper exit codes)
    if (output.find("Error") == std::string::npos && output.find("error") == std::string::npos) {
        m_currentMode = "auto";
        Logger::info("Successfully switched to auto mode");
        return true;
    } else {
        Logger::error("Failed to switch to auto mode");
        return false;
    }
}

bool TLPManager::setBatteryMode() {
    if (m_currentMode == "battery") {
        return true;  // Already in battery mode
    }

    Logger::info("Switching to battery mode (tlp bat)");
    std::string output = executeCommandWithOutput("tlp bat 2>&1");

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
    std::string output = executeCommandWithOutput("tlp-stat -s");

    // Parse the output to determine current mode
    // Look for "Mode" followed by "=" and then the actual mode value
    // This handles any amount of whitespace between Mode, =, and the value

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
                        m_currentMode = "auto";
                    } else if (mode == "battery") {
                        m_currentMode = "battery";
                    }
                }
            }
        }
    }

    // Fallback: check for older TLP_DEFAULT_MODE format
    if (m_currentMode == "unknown") {
        if (output.find("TLP_DEFAULT_MODE=AC") != std::string::npos) {
            m_currentMode = "auto";
        } else if (output.find("TLP_DEFAULT_MODE=BAT") != std::string::npos) {
            m_currentMode = "battery";
        }
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
