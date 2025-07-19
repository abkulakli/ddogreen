#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include "platform/isystem_monitor.h"

class ActivityMonitor {
public:
    ActivityMonitor();
    ~ActivityMonitor();

    // Callback function type for activity events
    using ActivityCallback = std::function<void(bool)>;

    // Start monitoring system activity
    bool start();

    // Stop monitoring
    void stop();

    // Set callback for activity changes
    void setActivityCallback(ActivityCallback callback);

    // Set load average thresholds for activity detection with hysteresis
    void setLoadThresholds(double highPerformanceThreshold, double powerSaveThreshold);

    // Set monitoring frequency in seconds
    void setMonitoringFrequency(int frequencySeconds);

    // Check if system is currently active
    bool isActive() const;

private:
    double getLoadAverage();  // Get load average using platform-specific monitor
    int getCpuCoreCount();  // Get CPU core count using platform-specific monitor
    void monitorLoop();

    bool m_isActive;
    bool m_running;
    double m_highPerformanceThreshold;  // Threshold to switch to high performance
    double m_powerSaveThreshold;        // Threshold to switch to power save
    int m_monitoringFrequency;          // Monitoring frequency in seconds
    int m_coreCount;  // Number of CPU cores
    ActivityCallback m_callback;
    std::unique_ptr<ISystemMonitor> m_systemMonitor;  // Platform-specific system monitor

    std::chrono::steady_clock::time_point m_lastLoadCheckTime;
};
