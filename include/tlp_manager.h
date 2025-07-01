#pragma once

#include <string>

class TLPManager {
public:
    TLPManager();
    ~TLPManager();

    // Set TLP to auto mode (for active usage - tlp start)
    bool setAutoMode();

    // Set TLP to battery saving mode (for idle state - tlp bat)
    bool setBatteryMode();

    // Get current TLP status
    std::string getCurrentMode();

    // Check if TLP is installed and available
    bool isTLPAvailable();

private:
    bool executeCommand(const std::string& command);
    std::string executeCommandWithOutput(const std::string& command);
    std::string cleanTLPOutput(const std::string& output);

    std::string m_currentMode;
};
