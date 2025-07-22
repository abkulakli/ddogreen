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
    , m_highPerformanceThreshold(0.0)  // Must be set via setLoadThresholds()
    , m_powerSaveThreshold(0.0)        // Must be set via setLoadThresholds()
    , m_monitoringFrequency(0)         // Must be set via setMonitoringFrequency()
    , m_coreCount(0)
    , m_callback(nullptr)
    , m_systemMonitor(nullptr) {
    
    // Create platform-specific system monitor
    m_systemMonitor = PlatformFactory::createSystemMonitor();
    
    if (m_systemMonitor && m_systemMonitor->isAvailable()) {
        m_coreCount = m_systemMonitor->getCpuCoreCount();
        Logger::info("Detected " + std::to_string(m_coreCount) + " CPU core(s)");
        Logger::info("High performance threshold: " + formatNumber(m_highPerformanceThreshold) + " (" + formatNumber(m_highPerformanceThreshold * 100) + "% per core)");
        Logger::info("Power save threshold: " + formatNumber(m_powerSaveThreshold) + " (" + formatNumber(m_powerSaveThreshold * 100) + "% per core)");
        Logger::info("Absolute high performance threshold: " + formatNumber(m_highPerformanceThreshold * m_coreCount));
        Logger::info("Absolute power save threshold: " + formatNumber(m_powerSaveThreshold * m_coreCount));
    } else {
        Logger::error("Failed to initialize platform-specific system monitor");
        m_coreCount = 1; // fallback
    }
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
        double load1min = getLoadAverage();
        double highPerformanceAbsoluteThreshold = m_highPerformanceThreshold * m_coreCount;
        
        // Apply dual threshold logic with hysteresis:
        // High performance when load > high_performance_threshold
        // Power save when load < power_save_threshold
        // Between thresholds, maintain current state (but start with power save as default)
        if (load1min > highPerformanceAbsoluteThreshold) {
            m_isActive = true;
        } else {
            m_isActive = false;  // Default to power save for initial state
        }
        
        double load1minPercentage = (load1min / m_coreCount) * 100;
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
    double highPerformanceAbsoluteThreshold = highPerformanceThreshold * m_coreCount;
    double powerSaveAbsoluteThreshold = powerSaveThreshold * m_coreCount;
    Logger::info("High performance threshold set to " + formatNumber(highPerformanceThreshold) + " (" + formatNumber(highPerformanceThreshold * 100) + "% per core)");
    Logger::info("Power save threshold set to " + formatNumber(powerSaveThreshold) + " (" + formatNumber(powerSaveThreshold * 100) + "% per core)");
    Logger::info("Absolute high performance threshold: " + formatNumber(highPerformanceAbsoluteThreshold) + " (for " + std::to_string(m_coreCount) + " cores)");
    Logger::info("Absolute power save threshold: " + formatNumber(powerSaveAbsoluteThreshold) + " (for " + std::to_string(m_coreCount) + " cores)");
}

void ActivityMonitor::setMonitoringFrequency(int frequencySeconds) {
    m_monitoringFrequency = frequencySeconds;
    
    // Update the system monitor with the new frequency
    if (m_systemMonitor && m_systemMonitor->isAvailable()) {
        m_systemMonitor->setMonitoringFrequency(frequencySeconds);
    }
    
    Logger::info("Monitoring frequency set to " + std::to_string(frequencySeconds) + " seconds");
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

        // Check load average at configured frequency
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= m_monitoringFrequency) {
            double load1min = getLoadAverage();
            m_lastLoadCheckTime = now;

            // Calculate the actual thresholds based on core count
            double highPerformanceAbsoluteThreshold = m_highPerformanceThreshold * m_coreCount;
            double powerSaveAbsoluteThreshold = m_powerSaveThreshold * m_coreCount;

            Logger::debug("Load average: " + formatNumber(load1min) +
                         " (high perf threshold: " + formatNumber(highPerformanceAbsoluteThreshold) +
                         " = " + formatNumber(m_highPerformanceThreshold * 100) + "% of " + std::to_string(m_coreCount) + " cores, " +
                         "power save threshold: " + formatNumber(powerSaveAbsoluteThreshold) +
                         " = " + formatNumber(m_powerSaveThreshold * 100) + "% of " + std::to_string(m_coreCount) + " cores)");

            // Determine activity state using hysteresis logic
            bool wasActive = m_isActive;
            
            // Hysteresis logic:
            // - If currently in power save mode and load > high performance threshold, switch to high performance
            // - If currently in high performance mode and load < power save threshold, switch to power save
            // - Otherwise, maintain current state
            if (!m_isActive && load1min > highPerformanceAbsoluteThreshold) {
                m_isActive = true;  // Switch to high performance
            } else if (m_isActive && load1min < powerSaveAbsoluteThreshold) {
                m_isActive = false;  // Switch to power save
            }
            // Else: maintain current state (hysteresis zone between thresholds)

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                double load1minPercentage = (load1min / m_coreCount) * 100;
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
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(m_monitoringFrequency));
    }
}
