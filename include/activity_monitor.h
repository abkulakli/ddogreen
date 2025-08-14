#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "platform/isystem_monitor.h"

class ActivityMonitor
{
public:
    ActivityMonitor();
    ~ActivityMonitor();

    using ActivityCallback = std::function<void(bool)>;

    bool start();
    void stop();
    void setActivityCallback(ActivityCallback callback);
    void setLoadThresholds(double highPerformanceThreshold, double powerSaveThreshold);
    void setMonitoringFrequency(int frequencySeconds);
    bool isActive() const;

private:
    double getLoadAverage();
    int getCpuCoreCount();
    void monitorLoop();

    bool m_isActive;
    std::atomic<bool> m_running;
    std::atomic<bool> m_threadReady;
    std::mutex m_readyMutex;
    std::condition_variable m_readyCondition;
    std::mutex m_monitorMutex;
    std::condition_variable m_monitorCondition;
    double m_highPerformanceThreshold;
    double m_powerSaveThreshold;
    int m_monitoringFrequencySeconds;
    int m_cpuCoreCount;
    ActivityCallback m_callback;
    std::unique_ptr<ISystemMonitor> m_systemMonitor;

    std::chrono::steady_clock::time_point m_lastLoadCheckTime;
    std::chrono::steady_clock::time_point m_lastStateChangeTime;
    static constexpr int MINIMUM_STATE_CHANGE_INTERVAL = 60;
};
