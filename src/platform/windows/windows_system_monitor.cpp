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
#include <queue>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")
#endif

/**
 * Windows-specific system monitor implementation
 * Uses Windows Performance Counters for system monitoring
 */
class WindowsSystemMonitor : public ISystemMonitor {
public:
    WindowsSystemMonitor() : m_coreCount(0), m_available(false) {
        Logger::info("Windows System Monitor initialized");
        
        // Initialize CPU monitoring
        initializeCpuMonitoring();
        
        // Get CPU core count
        m_coreCount = getCpuCoreCountInternal();
        m_available = (m_coreCount > 0);
        
        if (m_available) {
            Logger::info("Detected " + std::to_string(m_coreCount) + " CPU cores");
        } else {
            Logger::error("Failed to initialize Windows system monitoring");
        }
    }

    virtual ~WindowsSystemMonitor() {
#ifdef _WIN32
        // Cleanup PDH resources
        if (m_cpuQuery != nullptr) {
            PdhCloseQuery(m_cpuQuery);
        }
#endif
    }

    /**
     * Get system load averages
     * Windows doesn't have native load averages, so we calculate them from CPU usage
     * @return tuple of (1-minute, 5-minute, 15-minute) load averages
     */
    std::tuple<double, double, double> getLoadAverages() override {
        Logger::debug("Getting Windows system load averages");
        
        // Get current CPU usage percentage
        const double cpuUsage = getCurrentCpuUsage();
        
        // Convert CPU usage percentage to load average equivalent
        // Load average is typically: (CPU usage / 100) * core count
        const double currentLoad = (cpuUsage / 100.0) * m_coreCount;
        
        // Update our moving averages
        updateLoadAverages(currentLoad);
        
        // Calculate averages over different time periods
        const double load1min = calculateAverage(m_loadHistory, 60);    // 1 minute (60 samples at 1 sec each)
        const double load5min = calculateAverage(m_loadHistory, 300);   // 5 minutes (300 samples)
        const double load15min = calculateAverage(m_loadHistory, 900);  // 15 minutes (900 samples)
        
        Logger::debug("Load averages - 1min: " + std::to_string(load1min) + 
                     ", 5min: " + std::to_string(load5min) + 
                     ", 15min: " + std::to_string(load15min));
        
        return std::make_tuple(load1min, load5min, load15min);
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

private:
    /**
     * Initialize CPU monitoring using Performance Counters
     */
    void initializeCpuMonitoring() {
#ifdef _WIN32
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
        
        // Initialize with first sample
        PdhCollectQueryData(m_cpuQuery);
        
        Logger::debug("Performance Counters initialized successfully");
#else
        Logger::info("CROSS-PLATFORM: Would initialize Windows Performance Counters");
#endif
    }
    
    /**
     * Get current CPU usage percentage
     * @return CPU usage as percentage (0.0 to 100.0)
     */
    double getCurrentCpuUsage() {
#ifdef _WIN32
        if (m_cpuQuery == nullptr) {
            Logger::debug("CPU query not initialized, returning 0%");
            return 0.0;
        }
        
        // Collect data
        PDH_STATUS status = PdhCollectQueryData(m_cpuQuery);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to collect CPU data: " + std::to_string(status));
            return 0.0;
        }
        
        // Get formatted value
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS) {
            Logger::debug("Failed to get formatted CPU value: " + std::to_string(status));
            return 0.0;
        }
        
        const double cpuUsage = value.doubleValue;
        Logger::debug("Current CPU usage: " + std::to_string(cpuUsage) + "%");
        
        return cpuUsage;
#else
        // Mock for cross-compilation
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        double mockUsage = 10.0 + (seconds % 30) * 2.0; // Varies between 10% and 70%
        Logger::debug("CROSS-PLATFORM: Mock CPU usage: " + std::to_string(mockUsage) + "%");
        return mockUsage;
#endif
    }
    
    /**
     * Update load average history with new sample
     * @param currentLoad Current load value
     */
    void updateLoadAverages(double currentLoad) {
        // Add current load to history
        m_loadHistory.push(currentLoad);
        
        // Keep only last 15 minutes of data (900 samples at 1 second intervals)
        // Note: In practice, we might sample less frequently
        const size_t maxSamples = 900;
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
        while (!temp.empty()) {
            values.push_back(temp.front());
            temp.pop();
        }
        
        // Calculate average over the requested period
        const size_t numSamples = (std::min)(samples, values.size());
        const size_t startIndex = values.size() - numSamples;
        
        double sum = 0.0;
        for (size_t i = startIndex; i < values.size(); ++i) {
            sum += values[i];
        }
        
        return sum / numSamples;
    }
    
    /**
     * Get CPU core count using Windows API
     * @return number of CPU cores
     */
    int getCpuCoreCountInternal() {
#ifdef _WIN32
        Logger::debug("Getting CPU core count from Windows");
        
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        const int coreCount = static_cast<int>(sysInfo.dwNumberOfProcessors);
        Logger::debug("Detected " + std::to_string(coreCount) + " logical processors");
        
        return coreCount;
#else
        // Mock for cross-compilation
        Logger::debug("CROSS-PLATFORM: Mock CPU core count");
        return 4; // Mock 4 cores
#endif
    }

    // Member variables
    int m_coreCount;
    bool m_available;
    std::queue<double> m_loadHistory;
    
#ifdef _WIN32
    PDH_HQUERY m_cpuQuery = nullptr;
    PDH_HCOUNTER m_cpuCounter = nullptr;
#endif
};

// Factory function for creating Windows system monitor
std::unique_ptr<ISystemMonitor> createWindowsSystemMonitor() {
    return std::make_unique<WindowsSystemMonitor>();
}
