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
     * Get system load averages
     * @return tuple of (1-minute, 5-minute, 15-minute) load averages
     * Note: On Windows, may simulate load averages or use alternative metrics
     */
    virtual std::tuple<double, double, double> getLoadAverages() = 0;

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
};
