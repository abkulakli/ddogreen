#include "activity_monitor.h"
#include "logger.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>

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
        auto loadAverages = getLoadAverages();
        double load1min = std::get<0>(loadAverages);
        double load5min = std::get<1>(loadAverages);
        double load15min = std::get<2>(loadAverages);
        double absoluteThreshold = m_loadThreshold * m_coreCount;
        
        // Apply dual-threshold logic: 1-min > 15% = AC, both 5-min AND 15-min < 15% = BAT
        if (load1min > absoluteThreshold) {
            m_isActive = true;
        } else if (load5min <= absoluteThreshold && load15min <= absoluteThreshold) {
            m_isActive = false;
        }
        // If 1-min <= 15% AND (5-min > 15% OR 15-min > 15%), maintain current state (no change)
        
        double load1minPercentage = (load1min / m_coreCount) * 100;
        double load5minPercentage = (load5min / m_coreCount) * 100;
        double load15minPercentage = (load15min / m_coreCount) * 100;
        Logger::info("Initial state: 1-min load: " + std::to_string(load1min) +
                    " (" + std::to_string(load1minPercentage) + "% avg per core), " +
                    "5-min load: " + std::to_string(load5min) +
                    " (" + std::to_string(load5minPercentage) + "% avg per core), " +
                    "15-min load: " + std::to_string(load15min) +
                    " (" + std::to_string(load15minPercentage) + "% avg per core)");
        Logger::info(m_isActive ?
            "System active - switching to TLP AC mode" :
            "System idle - switching to TLP battery mode");
        m_callback(m_isActive);
    }

    // Start monitoring in a separate thread
    std::thread monitorThread(&ActivityMonitor::monitorLoop, this);
    monitorThread.detach();

    Logger::info("Activity monitor started (dual-threshold load monitoring: 1-min > 15% = AC, both 5-min AND 15-min <= 15% = BAT)");
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

std::tuple<double, double, double> ActivityMonitor::getLoadAverages() {
    std::ifstream file("/proc/loadavg");
    if (!file.is_open()) {
        Logger::error("Could not open /proc/loadavg");
        return {0.0, 0.0, 0.0};
    }

    std::string line;
    if (!std::getline(file, line)) {
        Logger::error("Could not read from /proc/loadavg");
        return {0.0, 0.0, 0.0};
    }

    // Parse the first three values (1-minute, 5-minute, and 15-minute load averages)
    std::istringstream iss(line);
    double load1min, load5min, load15min;
    iss >> load1min >> load5min >> load15min;

    return {load1min, load5min, load15min};
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

        // Check load averages every 30 seconds as requested
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= 30) {
            auto loadAverages = getLoadAverages();
            double load1min = std::get<0>(loadAverages);
            double load5min = std::get<1>(loadAverages);
            double load15min = std::get<2>(loadAverages);
            m_lastLoadCheckTime = now;

            // Calculate the actual threshold based on core count
            double absoluteThreshold = m_loadThreshold * m_coreCount;

            Logger::debug("Load averages - 1-min: " + std::to_string(load1min) +
                         ", 5-min: " + std::to_string(load5min) +
                         ", 15-min: " + std::to_string(load15min) +
                         " (threshold: " + std::to_string(absoluteThreshold) +
                         " = " + std::to_string(m_loadThreshold * 100) + "% of " + std::to_string(m_coreCount) + " cores)");

            // Determine activity state using dual-threshold logic
            bool wasActive = m_isActive;
            bool shouldSwitchToActive = (load1min > absoluteThreshold);
            bool shouldSwitchToIdle = (load5min <= absoluteThreshold && load15min <= absoluteThreshold);

            // Apply switching logic
            if (shouldSwitchToActive) {
                m_isActive = true;
            } else if (shouldSwitchToIdle) {
                m_isActive = false;
            }
            // If neither condition is met (1-min <= 15% AND (5-min > 15% OR 15-min > 15%)), maintain current state

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                double load1minPercentage = (load1min / m_coreCount) * 100;
                double load5minPercentage = (load5min / m_coreCount) * 100;
                double load15minPercentage = (load15min / m_coreCount) * 100;
                
                if (m_isActive) {
                    Logger::info("System became active (1-min load: " + std::to_string(load1min) +
                                " = " + std::to_string(load1minPercentage) + "% avg per core > 15%) - switching to TLP AC mode");
                } else {
                    Logger::info("System became idle (5-min load: " + std::to_string(load5min) +
                                " = " + std::to_string(load5minPercentage) + "% avg per core <= 15% AND 15-min load: " + std::to_string(load15min) +
                                " = " + std::to_string(load15minPercentage) + "% avg per core <= 15%) - switching to TLP battery mode");
                }
                m_callback(m_isActive);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}
