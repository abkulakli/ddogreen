#include "activity_monitor.h"
#include "logger.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>

ActivityMonitor::ActivityMonitor()
    : m_isActive(false)
    , m_running(false)
    , m_idleTimeout(300)  // 5 minutes default
    , m_cpuThreshold(15.0)  // 15% CPU usage threshold
    , m_callback(nullptr)
    , m_lastTotalTime(0.0)
    , m_lastIdleTime(0.0) {
}

ActivityMonitor::~ActivityMonitor() {
    stop();
}

bool ActivityMonitor::start() {
    if (m_running) {
        return true;
    }

    m_running = true;
    m_lastActivityTime = std::chrono::steady_clock::now();
    m_lastCpuCheckTime = std::chrono::steady_clock::now();

    // Initialize CPU monitoring baseline
    getCurrentCpuUsage();

    // Start monitoring in a separate thread
    std::thread monitorThread(&ActivityMonitor::monitorLoop, this);
    monitorThread.detach();

    Logger::info("Activity monitor started (CPU-based monitoring)");
    return true;
}

void ActivityMonitor::stop() {
    m_running = false;
    Logger::info("Activity monitor stopped");
}

void ActivityMonitor::setActivityCallback(ActivityCallback callback) {
    m_callback = callback;
}

void ActivityMonitor::setIdleTimeout(int seconds) {
    m_idleTimeout = seconds;
    Logger::info("Idle timeout set to " + std::to_string(seconds) + " seconds");
}

void ActivityMonitor::setCpuThreshold(double threshold) {
    m_cpuThreshold = threshold;
    Logger::info("CPU threshold set to " + std::to_string(threshold) + "%");
}

bool ActivityMonitor::isActive() const {
    return m_isActive;
}

double ActivityMonitor::getCurrentCpuUsage() {
    std::ifstream statFile("/proc/stat");
    if (!statFile.is_open()) {
        Logger::error("Failed to open /proc/stat");
        return 0.0;
    }

    std::string line;
    std::getline(statFile, line);
    statFile.close();

    std::istringstream iss(line);
    std::string cpu;
    double user, nice, system, idle, iowait, irq, softirq, steal;

    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    double totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    double idleTime = idle + iowait;

    if (m_lastTotalTime == 0.0) {
        // First measurement, just store baseline
        m_lastTotalTime = totalTime;
        m_lastIdleTime = idleTime;
        return 0.0;
    }

    double totalDiff = totalTime - m_lastTotalTime;
    double idleDiff = idleTime - m_lastIdleTime;

    m_lastTotalTime = totalTime;
    m_lastIdleTime = idleTime;

    if (totalDiff == 0.0) {
        return 0.0;
    }

    double cpuUsage = 100.0 * (1.0 - idleDiff / totalDiff);
    return cpuUsage;
}

void ActivityMonitor::monitorLoop() {
    while (m_running) {
        auto now = std::chrono::steady_clock::now();

        // Check CPU usage every 5 seconds to avoid too frequent checks
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastCpuCheckTime).count() >= 5) {
            double cpuUsage = getCurrentCpuUsage();
            m_lastCpuCheckTime = now;

            // If CPU usage is above threshold, consider system active
            if (cpuUsage > m_cpuThreshold) {
                m_lastActivityTime = now;
            }

            Logger::debug("CPU usage: " + std::to_string(cpuUsage) + "%");
        }

        // Determine if system is currently active
        auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastActivityTime).count();
        bool wasActive = m_isActive;
        m_isActive = (idleTime < m_idleTimeout);

        // If activity state changed, notify callback
        if (wasActive != m_isActive && m_callback) {
            Logger::info(m_isActive ? "System became active (CPU usage detected)" :
                                    "System became idle (low CPU usage for " + std::to_string(idleTime) + " seconds)");
            m_callback(m_isActive);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
