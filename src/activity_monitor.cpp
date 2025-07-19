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
    , m_highPerformanceThreshold(0.70)  // 70% load per core threshold to switch to high performance
    , m_powerSaveThreshold(0.30)        // 30% load per core threshold to switch to power save
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
        // If load > 70%, switch to high performance
        // If load < 30%, switch to power save
        // Between 30-70%, maintain current state (but start with power save as default)
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

    Logger::info("Activity monitor started (load > 70% = performance mode, load < 30% = power saving mode, 30-70% = maintain current mode)");
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

        // Check load average every 10 seconds for responsive switching
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= 10) {
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
            // - If currently in power save mode and load > 70%, switch to high performance
            // - If currently in high performance mode and load < 30%, switch to power save
            // - Otherwise, maintain current state
            if (!m_isActive && load1min > highPerformanceAbsoluteThreshold) {
                m_isActive = true;  // Switch to high performance
            } else if (m_isActive && load1min < powerSaveAbsoluteThreshold) {
                m_isActive = false;  // Switch to power save
            }
            // Else: maintain current state (hysteresis zone between 30-70%)

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                double load1minPercentage = (load1min / m_coreCount) * 100;
                
                if (m_isActive) {
                    Logger::info("System became active (load: " + formatNumber(load1min) +
                                " = " + formatNumber(load1minPercentage) + "% avg per core > 70%) - switching to performance mode");
                } else {
                    Logger::info("System became idle (load: " + formatNumber(load1min) +
                                " = " + formatNumber(load1minPercentage) + "% avg per core < 30%) - switching to power saving mode");
                }
                m_callback(m_isActive);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
