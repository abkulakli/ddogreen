#pragma once

#include <string>

/**
 * Configuration management for ddogreen
 * Handles loading and storing configuration settings
 */
class Config {
public:
    Config();
    ~Config() = default;

    bool loadFromFile(const std::string& configPath);

    int getMonitoringFrequency() const { return m_monitoringFrequency; }
    double getHighPerformanceThreshold() const { return m_highPerformanceThreshold; }
    double getPowerSaveThreshold() const { return m_powerSaveThreshold; }

    static std::string getDefaultConfigPath();

private:
    int m_monitoringFrequency;
    double m_highPerformanceThreshold;
    double m_powerSaveThreshold;

    std::string trim(const std::string& str) const;
    bool parseLine(const std::string& line);
    bool validateConfiguration() const;
};
