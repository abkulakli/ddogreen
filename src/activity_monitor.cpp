#include "activity_monitor.h"
#include "logger.h"
#include "platform/platform_factory.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>

ActivityMonitor::ActivityMonitor()
    : m_isActive(false)
    , m_running(false)
    , m_loadThreshold(0.10)  // 10% load per core threshold
    , m_coreCount(0)
    , m_callback(nullptr)
    , m_systemMonitor(nullptr) {
    
    // Create platform-specific system monitor
    m_systemMonitor = PlatformFactory::createSystemMonitor();
    
    if (m_systemMonitor && m_systemMonitor->isAvailable()) {
        m_coreCount = m_systemMonitor->getCpuCoreCount();
        Logger::info("Detected " + std::to_string(m_coreCount) + " CPU core(s)");
        Logger::info("Load threshold: " + std::to_string(m_loadThreshold) + " (" + std::to_string(m_loadThreshold * 100) + "% per core)");
        Logger::info("Absolute load threshold: " + std::to_string(m_loadThreshold * m_coreCount));
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
        double absoluteThreshold = m_loadThreshold * m_coreCount;
        
        // Apply threshold logic: 1-min > 10% = AC, 1-min <= 10% = BAT
        if (load1min > absoluteThreshold) {
            m_isActive = true;
        } else {
            m_isActive = false;
        }
        
        double load1minPercentage = (load1min / m_coreCount) * 100;
        Logger::info("Initial state: load: " + std::to_string(load1min) +
                    " (" + std::to_string(load1minPercentage) + "% avg per core)");
        Logger::info(m_isActive ?
            "System active - switching to performance mode" :
            "System idle - switching to power saving mode");
        m_callback(m_isActive);
    }

    // Start monitoring in a separate thread
    std::thread monitorThread(&ActivityMonitor::monitorLoop, this);
    monitorThread.detach();

    Logger::info("Activity monitor started (load > 10% = performance mode, load <= 10% = power saving mode)");
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

        // Check load average every 30 seconds as requested
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastLoadCheckTime).count() >= 30) {
            double load1min = getLoadAverage();
            m_lastLoadCheckTime = now;

            // Calculate the actual threshold based on core count
            double absoluteThreshold = m_loadThreshold * m_coreCount;

            Logger::debug("Load average: " + std::to_string(load1min) +
                         " (threshold: " + std::to_string(absoluteThreshold) +
                         " = " + std::to_string(m_loadThreshold * 100) + "% of " + std::to_string(m_coreCount) + " cores)");

            // Determine activity state using threshold logic
            bool wasActive = m_isActive;
            
            // Simple logic: 1-min > threshold = active, 1-min <= threshold = idle
            if (load1min > absoluteThreshold) {
                m_isActive = true;
            } else {
                m_isActive = false;
            }

            // If activity state changed, notify callback
            if (wasActive != m_isActive && m_callback) {
                double load1minPercentage = (load1min / m_coreCount) * 100;
                
                if (m_isActive) {
                    Logger::info("System became active (load: " + std::to_string(load1min) +
                                " = " + std::to_string(load1minPercentage) + "% avg per core > 10%) - switching to performance mode");
                } else {
                    Logger::info("System became idle (load: " + std::to_string(load1min) +
                                " = " + std::to_string(load1minPercentage) + "% avg per core <= 10%) - switching to power saving mode");
                }
                m_callback(m_isActive);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}
