#include "activity_monitor.h"
#include "logger.h"
#include "platform/platform_factory.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>
#include <iomanip>

// Helper function to format numbers with exactly 2 decimal places
std::string formatNumber(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

ActivityMonitor::ActivityMonitor()
    : m_isActive(false)
    , m_running(false)
    , m_highPerformanceThreshold(0.0)
    , m_powerSaveThreshold(0.0)
    , m_monitoringFrequencySeconds(0)
    , m_cpuCoreCount(0)
    , m_callback(nullptr)
    , m_systemMonitor(nullptr) {
    
    auto now = std::chrono::steady_clock::now();
    m_lastLoadCheckTime = now;
    m_lastStateChangeTime = now;
    
    m_systemMonitor = PlatformFactory::createSystemMonitor();
    
    if (m_systemMonitor && m_systemMonitor->isAvailable()) {
        m_cpuCoreCount = m_systemMonitor->getCpuCoreCount();
        Logger::info("Detected " + std::to_string(m_cpuCoreCount) + " CPU core(s)");
        Logger::info("High performance threshold: " + formatNumber(m_highPerformanceThreshold) + " (" + formatNumber(m_highPerformanceThreshold * 100) + "% per core)");
        Logger::info("Power save threshold: " + formatNumber(m_powerSaveThreshold) + " (" + formatNumber(m_powerSaveThreshold * 100) + "% per core)");
        Logger::info("Absolute high performance threshold: " + formatNumber(m_highPerformanceThreshold * m_cpuCoreCount));
        Logger::info("Absolute power save threshold: " + formatNumber(m_powerSaveThreshold * m_cpuCoreCount));
    } else {
        Logger::error("Failed to initialize platform-specific system monitor");
        m_cpuCoreCount = 1;
    }
}

ActivityMonitor::~ActivityMonitor() {
    stop();
}

bool ActivityMonitor::start() {
    if (m_running) {
        Logger::warning("Activity monitor already running");
        return true;
    }
    
    if (!m_systemMonitor || !m_systemMonitor->isAvailable()) {
        Logger::error("Cannot start activity monitor: system monitor not available");
        return false;
    }
    
    if (m_monitoringFrequencySeconds <= 0) {
        Logger::error("Cannot start activity monitor: invalid monitoring frequency");
        return false;
    }

    m_running = true;
    m_lastLoadCheckTime = std::chrono::steady_clock::now();

    // Perform initial load check to set correct mode immediately
    if (m_callback) {
        double load1min = getLoadAverage();
        double highPerformanceAbsoluteThreshold = m_highPerformanceThreshold * m_cpuCoreCount;
        
        // Apply dual threshold logic with hysteresis:
        // High performance when load > high_performance_threshold
        // Power save when load < power_save_threshold
        // Between thresholds, maintain current state (but start with power save as default)
        if (load1min > highPerformanceAbsoluteThreshold) {
            m_isActive = true;
        } else {
            m_isActive = false;
        }
        
        double load1minPercentage = (load1min / m_cpuCoreCount) * 100;
        Logger::info("Initial state: load: " + formatNumber(load1min) +
                    " (" + formatNumber(load1minPercentage) + "% avg per core)");
        Logger::info(m_isActive ?
            "System active - switching to performance mode" :
            "System idle - switching to power saving mode");
        m_callback(m_isActive);
    }

    // Start monitoring in a separate thread
    std::thread monitorThread(&ActivityMonitor::monitorLoop, this);
    monitorThread.detach();

    Logger::info("Activity monitor started (load > " + formatNumber(m_highPerformanceThreshold * 100) + "% = performance mode, load < " + formatNumber(m_powerSaveThreshold * 100) + "% = power saving mode, " + formatNumber(m_powerSaveThreshold * 100) + "-" + formatNumber(m_highPerformanceThreshold * 100) + "% = maintain current mode)");
    return true;
}

void ActivityMonitor::stop() {
    m_running = false;
    Logger::info("Activity monitor stopped");
}

void ActivityMonitor::setActivityCallback(ActivityCallback callback) {
    m_callback = callback;
}

void ActivityMonitor::setLoadThresholds(double highPerformanceThreshold, double powerSaveThreshold) {
    m_highPerformanceThreshold = highPerformanceThreshold;
    m_powerSaveThreshold = powerSaveThreshold;
    double highPerformanceAbsoluteThreshold = highPerformanceThreshold * m_cpuCoreCount;
    double powerSaveAbsoluteThreshold = powerSaveThreshold * m_cpuCoreCount;
    Logger::info("High performance threshold set to " + formatNumber(highPerformanceThreshold) + " (" + formatNumber(highPerformanceThreshold * 100) + "% per core)");
    Logger::info("Power save threshold set to " + formatNumber(powerSaveThreshold) + " (" + formatNumber(powerSaveThreshold * 100) + "% per core)");
    Logger::info("Absolute high performance threshold: " + formatNumber(highPerformanceAbsoluteThreshold) + " (for " + std::to_string(m_cpuCoreCount) + " cores)");
    Logger::info("Absolute power save threshold: " + formatNumber(powerSaveAbsoluteThreshold) + " (for " + std::to_string(m_cpuCoreCount) + " cores)");
}

void ActivityMonitor::setMonitoringFrequency(int frequencySeconds) {
    m_monitoringFrequencySeconds = frequencySeconds;
    
    if (m_systemMonitor && m_systemMonitor->isAvailable()) {
        m_systemMonitor->setMonitoringFrequency(frequencySeconds);
    }
    
    Logger::info("Monitoring frequency set to " + std::to_string(frequencySeconds) + " seconds");
    Logger::info("Energy efficiency: minimum " + std::to_string(MINIMUM_STATE_CHANGE_INTERVAL) + "s between power state changes");
}

bool ActivityMonitor::isActive() const {
    return m_isActive;
}

double ActivityMonitor::getLoadAverage() {
    if (!m_systemMonitor || !m_systemMonitor->isAvailable()) {
        Logger::error("System monitor not available");
        return 0.0;
    }

    return m_systemMonitor->getLoadAverage();
}

int ActivityMonitor::getCpuCoreCount() {
    if (!m_systemMonitor || !m_systemMonitor->isAvailable()) {
        Logger::warning("System monitor not available, defaulting to 1 core");
        return 1;
    }

    return m_systemMonitor->getCpuCoreCount();
}

void ActivityMonitor::monitorLoop() {
    while (m_running) {
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= m_monitoringFrequencySeconds) {
            double load1min = getLoadAverage();
            m_lastLoadCheckTime = now;

            double highPerformanceAbsoluteThreshold = m_highPerformanceThreshold * m_cpuCoreCount;
            double powerSaveAbsoluteThreshold = m_powerSaveThreshold * m_cpuCoreCount;

            Logger::debug("Load average: " + formatNumber(load1min) +
                         " (high perf threshold: " + formatNumber(highPerformanceAbsoluteThreshold) +
                         " = " + formatNumber(m_highPerformanceThreshold * 100) + "% of " + std::to_string(m_cpuCoreCount) + " cores, " +
                         "power save threshold: " + formatNumber(powerSaveAbsoluteThreshold) +
                         " = " + formatNumber(m_powerSaveThreshold * 100) + "% of " + std::to_string(m_cpuCoreCount) + " cores)");

            bool wasActive = m_isActive;
            
            if (!m_isActive && load1min > highPerformanceAbsoluteThreshold) {
                m_isActive = true;
            } else if (m_isActive && load1min < powerSaveAbsoluteThreshold) {
                m_isActive = false;
            }

            if (wasActive != m_isActive && m_callback) {
                auto timeSinceLastChange = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastStateChangeTime).count();
                
                if (timeSinceLastChange >= MINIMUM_STATE_CHANGE_INTERVAL) {
                    double load1minPercentage = (load1min / m_cpuCoreCount) * 100;
                    double highPerfPercentage = m_highPerformanceThreshold * 100;
                    double powerSavePercentage = m_powerSaveThreshold * 100;
                    
                    if (m_isActive) {
                        Logger::info("System became active (load: " + formatNumber(load1min) +
                                    " = " + formatNumber(load1minPercentage) + "% avg per core > " + formatNumber(highPerfPercentage) + "%) - switching to performance mode");
                    } else {
                        Logger::info("System became idle (load: " + formatNumber(load1min) +
                                    " = " + formatNumber(load1minPercentage) + "% avg per core < " + formatNumber(powerSavePercentage) + "%) - switching to power saving mode");
                    }
                    m_callback(m_isActive);
                    m_lastStateChangeTime = now;
                } else {
                    m_isActive = wasActive;
                    Logger::debug("State change suppressed for energy efficiency (last change " + std::to_string(timeSinceLastChange) + "s ago, minimum " + std::to_string(MINIMUM_STATE_CHANGE_INTERVAL) + "s)");
                }
            }
        }

        int sleepDuration = std::max(m_monitoringFrequencySeconds, 10);
        std::this_thread::sleep_for(std::chrono::seconds(sleepDuration));
    }
}
