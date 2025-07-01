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
    , m_loadThreshold(0.15)  // 15% load average threshold
    , m_callback(nullptr) {
}

ActivityMonitor::~ActivityMonitor() {
    stop();
}

bool ActivityMonitor::start() {
    if (m_running) {
        return true;
    }

    m_running = true;
    m_lastLoadCheckTime = std::chrono::steady_clock::now();

    // Start monitoring in a separate thread
    std::thread monitorThread(&ActivityMonitor::monitorLoop, this);
    monitorThread.detach();

    Logger::info("Activity monitor started (1-minute load average monitoring)");
    return true;
}

void ActivityMonitor::stop() {
    m_running = false;
    Logger::info("Activity monitor stopped");
}

void ActivityMonitor::setActivityCallback(ActivityCallback callback) {
    m_callback = callback;
}

void ActivityMonitor::setLoadThreshold(double threshold) {
    m_loadThreshold = threshold;
    Logger::info("Load threshold set to " + std::to_string(threshold) + " (1-minute load average)");
}

bool ActivityMonitor::isActive() const {
    return m_isActive;
}

double ActivityMonitor::getOneMinuteLoadAverage() {
    std::ifstream file("/proc/loadavg");
    if (!file.is_open()) {
        Logger::error("Could not open /proc/loadavg");
        return 0.0;
    }

    std::string line;
    if (!std::getline(file, line)) {
        Logger::error("Could not read from /proc/loadavg");
        return 0.0;
    }

    // Parse the first value (1-minute load average)
    std::istringstream iss(line);
    double loadAverage;
    iss >> loadAverage;

    return loadAverage;
}

void ActivityMonitor::monitorLoop() {
    while (m_running) {
        auto now = std::chrono::steady_clock::now();

        // Check load average every 60 seconds for minimal resource usage
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= 60) {
            double loadAverage = getOneMinuteLoadAverage();
            m_lastLoadCheckTime = now;

            Logger::debug("1-minute load average: " + std::to_string(loadAverage) + " (threshold: " + std::to_string(m_loadThreshold) + ")");

            // Determine activity state directly from current load average
            bool wasActive = m_isActive;
            m_isActive = (loadAverage > m_loadThreshold);

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                Logger::info(m_isActive ? "System became active (load: " + std::to_string(loadAverage) + ") - switching to TLP auto mode" :
                                        "System became idle (load: " + std::to_string(loadAverage) + ") - switching to TLP battery mode");
                m_callback(m_isActive);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
