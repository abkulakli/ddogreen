#include "config.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::loadFromFile(const std::string& configPath) {
    setDefaults();

    std::ifstream file(configPath);
    if (!file.is_open()) {
        Logger::warning("Could not open config file: " + configPath + ". Using defaults.");
        return false;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            Logger::warning("Invalid config line " + std::to_string(lineNumber) + ": " + line);
            continue;
        }

        std::string key = trim(line.substr(0, equalPos));
        std::string value = trim(line.substr(equalPos + 1));

        // Remove quotes if present
        if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }

        // Parse configuration values
        if (key == "IDLE_TIMEOUT") {
            try {
                idleTimeout = std::stoi(value);
                Logger::info("Config: IDLE_TIMEOUT = " + std::to_string(idleTimeout));
            } catch (const std::exception& e) {
                Logger::warning("Invalid IDLE_TIMEOUT value: " + value);
            }
        } else if (key == "CPU_THRESHOLD") {
            try {
                cpuThreshold = std::stod(value);
                Logger::info("Config: CPU_THRESHOLD = " + std::to_string(cpuThreshold));
            } catch (const std::exception& e) {
                Logger::warning("Invalid CPU_THRESHOLD value: " + value);
            }
        } else if (key == "LOG_LEVEL") {
            logLevel = value;
            Logger::info("Config: LOG_LEVEL = " + logLevel);
        } else if (key == "LOG_FILE") {
            logFile = value;
            Logger::info("Config: LOG_FILE = " + logFile);
        } else if (key == "AUTO_SWITCH_ENABLED") {
            autoSwitchEnabled = (value == "true" || value == "1" || value == "yes");
            Logger::info("Config: AUTO_SWITCH_ENABLED = " + std::string(autoSwitchEnabled ? "true" : "false"));
        } else if (key == "TLP_START_COMMAND") {
            tlpStartCommand = value;
            Logger::info("Config: TLP_START_COMMAND = " + tlpStartCommand);
        } else if (key == "TLP_BAT_COMMAND") {
            tlpBatCommand = value;
            Logger::info("Config: TLP_BAT_COMMAND = " + tlpBatCommand);
        } else {
            Logger::warning("Unknown config key: " + key);
        }
    }

    file.close();
    Logger::info("Configuration loaded from: " + configPath);
    return true;
}

void Config::setDefaults() {
    idleTimeout = DEFAULT_IDLE_TIMEOUT;
    cpuThreshold = DEFAULT_CPU_THRESHOLD;
    logLevel = "INFO";
    logFile = DEFAULT_LOG_FILE;
    autoSwitchEnabled = true;
    tlpStartCommand = "tlp start";
    tlpBatCommand = "tlp bat";
    tlpStatusCommand = "tlp-stat -s";
}

std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
