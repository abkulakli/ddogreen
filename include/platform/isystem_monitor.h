#pragma once

#include <tuple>
#include <span>
#include <cstring>
#include <algorithm>

/**
 * Interface for platform-specific system monitoring functionality
 * Provides CPU load and core count information
 */
class ISystemMonitor
{
public:
    virtual ~ISystemMonitor() = default;

    /**
     * Get system load average
     * @return load average
     * Note: On Windows, may simulate load average or use alternative metrics
     */
    virtual double getLoadAverage() = 0;

    /**
     * Get the number of CPU cores in the system
     * @return number of CPU cores/logical processors
     */
    virtual int getCpuCoreCount() = 0;

    /**
     * Check if system monitoring is available on this platform
     * @return true if monitoring is supported and working
     */
    virtual bool isAvailable() = 0;

    /**
     * Set the monitoring frequency for load average calculations
     * @param frequencySeconds Monitoring frequency in seconds
     */
    virtual void setMonitoringFrequency(int frequencySeconds) = 0;

    /**
     * Get detailed system metrics in a structured format
     * @param metricsBuffer span to fill with system metrics data
     * @return number of bytes written to buffer
     */
    virtual size_t getSystemMetrics(std::span<char> metricsBuffer) const
    {
        // Default implementation - basic metrics
        const char* metrics = "load=0.0,cores=1";
        size_t metricsLen = strlen(metrics);
        if (metricsBuffer.size() >= metricsLen)
        {
            std::copy_n(metrics, metricsLen, metricsBuffer.data());
            return metricsLen;
        }
        return 0;
    }

    /**
     * Process historical load data
     * @param loadHistory span containing historical load average data
     * @return computed trend information
     */
    virtual double analyzeLoadTrend(std::span<const double> loadHistory) const
    {
        // Default implementation - simple average
        if (loadHistory.empty()) return 0.0;
        
        double sum = 0.0;
        for (const auto& load : loadHistory)
        {
            sum += load;
        }
        return sum / static_cast<double>(loadHistory.size());
    }
};
