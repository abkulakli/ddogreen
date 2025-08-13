#pragma once

#include <string>
#include <span>

/**
 * Configuration management for ddogreen
 * Handles loading and storing configuration settings
 */
class Config
{
public:
    Config();
    ~Config() = default;

    bool loadFromFile(const std::string& configPath);
    bool loadFromBuffer(std::span<const char> configData);

    int getMonitoringFrequency() const { return m_monitoringFrequency; }
    double getHighPerformanceThreshold() const { return m_highPerformanceThreshold; }
    double getPowerSaveThreshold() const { return m_powerSaveThreshold; }

    static std::string getDefaultConfigPath();

private:
    int m_monitoringFrequency;
    double m_highPerformanceThreshold;
    double m_powerSaveThreshold;

    std::string trim(std::span<const char> str) const;
    bool parseLine(std::span<const char> line);
    bool parseLineString(const std::string& line);
    bool parseKeyValue(const std::string& key, const std::string& value);
    bool validateConfiguration() const;
};
