#pragma once
#include <string>

class Config {
public:
    static Config& getInstance();
    bool loadFromFile(const std::string& configPath = "/etc/ddotlp/ddotlp.conf");

    // Getters
    int getIdleTimeout() const { return idleTimeout; }
    double getCpuThreshold() const { return cpuThreshold; }
    const std::string& getLogLevel() const { return logLevel; }
    const std::string& getLogFile() const { return logFile; }
    bool isAutoSwitchEnabled() const { return autoSwitchEnabled; }
    const std::string& getTlpStartCommand() const { return tlpStartCommand; }
    const std::string& getTlpBatCommand() const { return tlpBatCommand; }
    const std::string& getTlpStatusCommand() const { return tlpStatusCommand; }

    // Constants
    static constexpr const char* DEFAULT_CONFIG_FILE = "/etc/ddotlp/ddotlp.conf";
    static constexpr const char* DEFAULT_LOG_FILE = "/var/log/ddotlp.log";
    static constexpr const char* PID_FILE = "/run/ddotlp.pid";
    static constexpr int DEFAULT_IDLE_TIMEOUT = 300;
    static constexpr double DEFAULT_CPU_THRESHOLD = 15.0;

private:
    Config() = default;
    void setDefaults();
    std::string trim(const std::string& str);

    // Configuration values
    int idleTimeout = DEFAULT_IDLE_TIMEOUT;
    double cpuThreshold = DEFAULT_CPU_THRESHOLD;
    std::string logLevel = "INFO";
    std::string logFile = DEFAULT_LOG_FILE;
    bool autoSwitchEnabled = true;
    std::string tlpStartCommand = "tlp start";
    std::string tlpBatCommand = "tlp bat";
    std::string tlpStatusCommand = "tlp-stat -s";
};
