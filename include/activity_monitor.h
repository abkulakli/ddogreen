#pragma once

#include <chrono>
#include <functional>
#include <vector>

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

    // Set load average threshold for activity detection (default: 0.10)
    void setLoadThreshold(double threshold);

    // Check if system is currently active
    bool isActive() const;

private:
    std::tuple<double, double, double> getLoadAverages();  // Get 1-min, 5-min, and 15-min load averages from /proc/loadavg
    int getCpuCoreCount();  // Get CPU core count using /proc/cpuinfo
    void monitorLoop();

    bool m_isActive;
    bool m_running;
    double m_loadThreshold;  // Threshold as percentage (0.10 = 10%)
    int m_coreCount;  // Number of CPU cores
    ActivityCallback m_callback;

    std::chrono::steady_clock::time_point m_lastLoadCheckTime;
};
