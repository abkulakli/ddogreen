#include "config.h"
#include "logger.h"
#include "platform/platform_factory.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Config::Config() : m_monitoringFrequency(0), m_highPerformanceThreshold(0.0), m_powerSaveThreshold(0.0) {
}

std::string Config::getDefaultConfigPath() {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (platformUtils && platformUtils->isAvailable()) {
        return platformUtils->getDefaultConfigPath();
    }
    return "/etc/ddogreen/ddogreen.conf";
}

bool Config::loadFromFile(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        Logger::error("Configuration file not found: " + configPath);
        return false;
    }

    Logger::info("Loading configuration from: " + configPath);

    std::string line;
    int lineNumber = 0;
    bool hasErrors = false;

    while (std::getline(file, line)) {
        lineNumber++;

        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (!parseLine(line)) {
            Logger::warning("Invalid configuration line " + std::to_string(lineNumber) + ": " + line);
            hasErrors = true;
        }
    }

    file.close();

    if (m_monitoringFrequency == 0) {
        Logger::error("Missing required configuration: monitoring_frequency");
        hasErrors = true;
    }
    if (m_highPerformanceThreshold == 0.0) {
        Logger::error("Missing required configuration: high_performance_threshold");
        hasErrors = true;
    }
    if (m_powerSaveThreshold == 0.0) {
        Logger::error("Missing required configuration: power_save_threshold");
        hasErrors = true;
    }

    if (hasErrors) {
        Logger::error("Configuration contains errors in: " + configPath + " - please fix the configuration file and restart");
        return false;
    } else {
        Logger::info("Configuration loaded successfully from: " + configPath);
    }

    Logger::info("Monitoring frequency: " + std::to_string(m_monitoringFrequency) + " seconds");
    Logger::info("High performance threshold: " + std::to_string(m_highPerformanceThreshold) + " (" + std::to_string(m_highPerformanceThreshold * 100) + "%)");
    Logger::info("Power save threshold: " + std::to_string(m_powerSaveThreshold) + " (" + std::to_string(m_powerSaveThreshold * 100) + "%)");

    return true;
}

std::string Config::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool Config::parseLine(const std::string& line) {
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) {
        return false;
    }

    std::string key = trim(line.substr(0, equalPos));
    std::string value = trim(line.substr(equalPos + 1));

    if (key.empty() || value.empty()) {
        return false;
    }

    try {
        if (key == "monitoring_frequency") {
            int freq = std::stoi(value);
            if (freq >= 1 && freq <= 300) { // 1 second to 5 minutes
                m_monitoringFrequency = freq;
                return true;
            }
        } else if (key == "high_performance_threshold") {
            double threshold = std::stod(value);
            if (threshold >= 0.1 && threshold <= 1.0) { // 10% to 100%
                m_highPerformanceThreshold = threshold;
                return true;
            }
        } else if (key == "power_save_threshold") {
            double threshold = std::stod(value);
            if (threshold >= 0.05 && threshold <= 0.9) { // 5% to 90%
                m_powerSaveThreshold = threshold;
                return true;
            }
        }
    } catch (const std::exception& e) {
        return false;
    }

    return false;
}
