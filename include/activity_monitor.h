#pragma once

#include <chrono>
#include <functional>

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

    // Set idle timeout in seconds (default: 300 seconds = 5 minutes)
    void setIdleTimeout(int seconds);

    // Set CPU usage threshold for activity detection (default: 15%)
    void setCpuThreshold(double threshold);

    // Check if system is currently active
    bool isActive() const;

private:
    double getCurrentCpuUsage();
    void monitorLoop();

    bool m_isActive;
    bool m_running;
    int m_idleTimeout;
    double m_cpuThreshold;
    ActivityCallback m_callback;

    std::chrono::steady_clock::time_point m_lastActivityTime;
    std::chrono::steady_clock::time_point m_lastCpuCheckTime;
    double m_lastTotalTime;
    double m_lastIdleTime;
};
