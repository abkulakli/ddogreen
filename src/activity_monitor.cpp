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
    , m_loadThreshold(0.15)  // 15% load per core threshold
    , m_coreCount(getCpuCoreCount())
    , m_callback(nullptr) {
    Logger::info("Detected " + std::to_string(m_coreCount) + " CPU core(s)");
    Logger::info("Load threshold: " + std::to_string(m_loadThreshold) + " (" + std::to_string(m_loadThreshold * 100) + "% per core)");
    Logger::info("Absolute load threshold: " + std::to_string(m_loadThreshold * m_coreCount));
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

    // Perform initial load check to set correct mode immediately
    if (m_callback) {
        double loadAverage = getOneMinuteLoadAverage();
        double absoluteThreshold = m_loadThreshold * m_coreCount;
        m_isActive = (loadAverage > absoluteThreshold);
        
        double loadPercentage = (loadAverage / m_coreCount) * 100;
        Logger::info(m_isActive ?
            "Initial state: System active (load: " + std::to_string(loadAverage) +
            " = " + std::to_string(loadPercentage) + "% avg per core) - switching to TLP AC mode" :
            "Initial state: System idle (load: " + std::to_string(loadAverage) +
            " = " + std::to_string(loadPercentage) + "% avg per core) - switching to TLP battery mode");
        m_callback(m_isActive);
    }

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
    double absoluteThreshold = threshold * m_coreCount;
    Logger::info("Load threshold set to " + std::to_string(threshold) + " (" + std::to_string(threshold * 100) + "% per core)");
    Logger::info("Absolute load threshold: " + std::to_string(absoluteThreshold) + " (for " + std::to_string(m_coreCount) + " cores)");
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

int ActivityMonitor::getCpuCoreCount() {
    // Read /proc/cpuinfo to count processor entries
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        Logger::warning("Could not open /proc/cpuinfo, defaulting to 1 core");
        return 1;
    }

    int coreCount = 0;
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("processor") == 0) {
            coreCount++;
        }
    }

    if (coreCount > 0) {
        return coreCount;
    }

    // Final fallback if no processors found in /proc/cpuinfo
    Logger::warning("No processors found in /proc/cpuinfo, defaulting to 1 core");
    return 1;
}

void ActivityMonitor::monitorLoop() {
    while (m_running) {
        auto now = std::chrono::steady_clock::now();

        // Check load average every 60 seconds for minimal resource usage
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= 60) {
            double loadAverage = getOneMinuteLoadAverage();
            m_lastLoadCheckTime = now;

            // Calculate the actual threshold based on core count
            double absoluteThreshold = m_loadThreshold * m_coreCount;

            Logger::debug("1-minute load average: " + std::to_string(loadAverage) +
                         " (threshold: " + std::to_string(absoluteThreshold) +
                         " = " + std::to_string(m_loadThreshold * 100) + "% of " + std::to_string(m_coreCount) + " cores)");

            // Determine activity state: compare load average to absolute threshold
            bool wasActive = m_isActive;
            m_isActive = (loadAverage > absoluteThreshold);

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                double loadPercentage = (loadAverage / m_coreCount) * 100;
                Logger::info(m_isActive ?
                    "System became active (load: " + std::to_string(loadAverage) +
                    " = " + std::to_string(loadPercentage) + "% avg per core) - switching to TLP AC mode" :
                    "System became idle (load: " + std::to_string(loadAverage) +
                    " = " + std::to_string(loadPercentage) + "% avg per core) - switching to TLP battery mode");
                m_callback(m_isActive);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
