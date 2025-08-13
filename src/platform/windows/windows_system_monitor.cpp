#include "platform/isystem_monitor.h"
#include "logger.h"
#include <memory>
#include <tuple>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <span>
#include <queue>
#include <algorithm>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")

/**
 * Windows-specific system monitor implementation
 * Uses Windows Performance Counters for system monitoring
 */
class WindowsSystemMonitor : public ISystemMonitor {
public:
    WindowsSystemMonitor() : m_coreCount(0), m_available(false), m_monitoringFrequency(10) {
        Logger::info("Windows System Monitor initialized");
        
        // Initialize CPU monitoring
        initializeCpuMonitoring();
        
        // Get CPU core count
        m_coreCount = getCpuCoreCountInternal();
        m_available = (m_coreCount > 0);
        
        if (!m_available) {
            Logger::error("Failed to initialize Windows system monitoring");
        }
    }

    virtual ~WindowsSystemMonitor() {
        // Cleanup PDH resources
        if (m_cpuQuery != nullptr) {
            PdhCloseQuery(m_cpuQuery);
        }
    }

    /**
     * Get system load average
     * Windows doesn't have native load averages, so we use processor queue length
     * @return load average
     */
    double getLoadAverage() override {
        Logger::debug("Getting Windows system load average via processor queue length");
        
        // Get current processor queue length (real load indicator)
        const double currentLoad = getCurrentQueueLength();
        
        // Update our moving averages
        updateLoadAverages(currentLoad);
        
        // Calculate average for approximately 1 minute worth of samples
        // Number of samples = 60 seconds / monitoring_frequency
        const size_t samplesFor1Min = (m_monitoringFrequency <= 0) ? 1 : 
                                      (60 / m_monitoringFrequency < 1) ? 1 : 
                                      (60 / m_monitoringFrequency);
        const double load1min = calculateAverage(m_loadHistory, samplesFor1Min);
        
        return load1min;
    }

    /**
     * Get CPU core count using Windows API
     * @return number of CPU cores/logical processors
     */
    int getCpuCoreCount() override {
        return m_coreCount;
    }

    /**
     * Check if Windows system monitoring is available
     * @return true if monitoring is supported and working
     */
    bool isAvailable() override {
        return m_available;
    }

    /**
     * Set the monitoring frequency for load average calculations
     * @param frequencySeconds Monitoring frequency in seconds
     */
    void setMonitoringFrequency(int frequencySeconds) override {
        m_monitoringFrequency = frequencySeconds;
        Logger::debug("Windows system monitor frequency set to " + std::to_string(frequencySeconds) + " seconds");
    }

private:
    /**
     * Initialize CPU monitoring using Performance Counters
     */
    void initializeCpuMonitoring() {
        Logger::debug("Initializing Windows Performance Counters");
        
        // Create PDH query
        PDH_STATUS status = PdhOpenQuery(NULL, 0, &m_cpuQuery);
        if (status != ERROR_SUCCESS) {
            Logger::error("Failed to open PDH query: " + std::to_string(status));
            return;
        }
        
        // Add CPU usage counter
        status = PdhAddCounterW(m_cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &m_cpuCounter);
        if (status != ERROR_SUCCESS) {
            Logger::error("Failed to add CPU counter: " + std::to_string(status));
            PdhCloseQuery(m_cpuQuery);
            m_cpuQuery = nullptr;
            return;
        }
        
        // Add processor queue length counter (Windows equivalent of load average)
        status = PdhAddCounterW(m_cpuQuery, L"\\System\\Processor Queue Length", 0, &m_queueCounter);
        if (status != ERROR_SUCCESS) {
            Logger::error("Failed to add processor queue length counter: " + std::to_string(status));
            PdhCloseQuery(m_cpuQuery);
            m_cpuQuery = nullptr;
            return;
        }
        
        // Initialize with first sample (baseline)
        status = PdhCollectQueryData(m_cpuQuery);
        if (status != ERROR_SUCCESS) {
            Logger::error("Failed to collect initial CPU baseline: " + std::to_string(status));
            PdhCloseQuery(m_cpuQuery);
            m_cpuQuery = nullptr;
            return;
        }
        
        Logger::debug("Performance Counters (CPU usage and processor queue length) initialized successfully");
    }
    
    /**
     * Get current CPU usage percentage
     * @return CPU usage as percentage (0.0 to 100.0)
     */
    double getCurrentCpuUsage() {
        if (m_cpuQuery == nullptr) {
            Logger::debug("CPU query not initialized, returning 0%");
            return 0.0;
        }
        
        // Collect current CPU data
        PDH_STATUS status = PdhCollectQueryData(m_cpuQuery);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to collect CPU data: " + std::to_string(status));
            return 0.0;
        }
        
        // Get formatted CPU usage value
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to get formatted CPU value: " + std::to_string(status));
            return 0.0;
        }
        
        const double cpuUsage = value.doubleValue;
        
        return cpuUsage;
    }
    
    /**
     * Get current processor queue length (Windows equivalent of load average)
     * @return Number of threads waiting for CPU time
     */
    double getCurrentQueueLength() {
        if (m_cpuQuery == nullptr) {
            Logger::debug("CPU query not initialized, returning 0 queue length");
            return 0.0;
        }
        
        // Collect current performance data
        PDH_STATUS status = PdhCollectQueryData(m_cpuQuery);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to collect queue length data: " + std::to_string(status));
            return 0.0;
        }
        
        // Get formatted queue length value
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(m_queueCounter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to get formatted queue length value: " + std::to_string(status));
            return 0.0;
        }
        
        const double queueLength = value.doubleValue;
        Logger::debug("Current processor queue length: " + std::to_string(queueLength));
        
        return queueLength;
    }
    
    /**
     * Update load average history with new sample
     * @param currentLoad Current load value
     */
    void updateLoadAverages(double currentLoad) {
        // Add current load to history
        m_loadHistory.push(currentLoad);
        
        // Keep only enough samples for load average calculation
        // We want approximately 1 minute of data: 60 seconds / monitoring_frequency
        const size_t maxSamples = (m_monitoringFrequency <= 0) ? 1 : 
                                  (60 / m_monitoringFrequency < 1) ? 1 : 
                                  (60 / m_monitoringFrequency);
        while (m_loadHistory.size() > maxSamples) {
            m_loadHistory.pop();
        }
    }
    
    /**
     * Calculate average load over specified number of samples
     * @param history Load history queue
     * @param samples Number of samples to average (or all if fewer available)
     * @return Average load over the period
     */
    double calculateAverage(const std::queue<double>& history, size_t samples) {
        if (history.empty()) {
            return 0.0;
        }
        
        // Convert queue to vector for easier access
        std::vector<double> values;
        std::queue<double> temp = history; // Copy queue to iterate
        while (!temp.empty())
        {
            values.push_back(temp.front());
            temp.pop();
        }
        
        // Calculate average over the requested period
        const size_t numSamples = (std::min)(samples, values.size());
        const size_t startIndex = values.size() - numSamples;
        
        double sum = 0.0;
        // Use range-based for loop with subspan for better bounds safety
        auto valueSpan = std::span<const double>{values}.subspan(startIndex);
        for (const auto& value : valueSpan)
        {
            sum += value;
        }
        
        return sum / numSamples;
    }
    
    /**
     * Get CPU core count using Windows API
     * @return number of CPU cores
     */
    int getCpuCoreCountInternal() {
        Logger::debug("Getting CPU core count from Windows");
        
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        const int coreCount = static_cast<int>(sysInfo.dwNumberOfProcessors);
        Logger::debug("Windows CPU core count: " + std::to_string(coreCount));
        
        return coreCount;
    }

    // Member variables
    int m_coreCount;
    bool m_available;
    int m_monitoringFrequency;
    std::queue<double> m_loadHistory;
    
    PDH_HQUERY m_cpuQuery = nullptr;
    PDH_HCOUNTER m_cpuCounter = nullptr;
    PDH_HCOUNTER m_queueCounter = nullptr;
};

// Factory function for creating Windows system monitor
std::unique_ptr<ISystemMonitor> createWindowsSystemMonitor() {
    return std::make_unique<WindowsSystemMonitor>();
}
