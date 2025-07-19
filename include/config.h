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

    // Load configuration from file
    bool loadFromFile(const std::string& configPath);

    // Getters for configuration values
    int getMonitoringFrequency() const { return m_monitoringFrequency; }
    double getHighPerformanceThreshold() const { return m_highPerformanceThreshold; }
    double getPowerSaveThreshold() const { return m_powerSaveThreshold; }

    // Get default configuration file path
    static std::string getDefaultConfigPath();

private:
    // Configuration values - must be loaded from file
    int m_monitoringFrequency;        // Check frequency in seconds
    double m_highPerformanceThreshold; // Threshold to switch to high performance
    double m_powerSaveThreshold;       // Threshold to switch to power save

    // Helper methods
    std::string trim(const std::string& str) const;
    bool parseLine(const std::string& line);
};
