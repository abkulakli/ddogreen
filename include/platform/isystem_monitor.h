#pragma once

#include <tuple>

/**
 * Interface for platform-specific system monitoring functionality
 * Provides CPU load and core count information
 */
class ISystemMonitor {
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
};
