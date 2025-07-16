#pragma once

#include <string>

/**
 * Interface for platform-specific power management functionality
 * Handles switching between performance and power-saving modes
 */
class IPowerManager {
public:
    virtual ~IPowerManager() = default;

    /**
     * Set system to high performance mode
     * Linux: tlp ac (AC adapter mode)
     * Windows: High Performance power plan
     * @return true if successful
     */
    virtual bool setPerformanceMode() = 0;

    /**
     * Set system to power saving mode
     * Linux: tlp bat (battery mode)
     * Windows: Power Saver power plan
     * @return true if successful
     */
    virtual bool setPowerSavingMode() = 0;

    /**
     * Get current power management mode
     * @return string describing current mode ("performance", "powersaving", "unknown")
     */
    virtual std::string getCurrentMode() = 0;

    /**
     * Check if power management is available on this platform
     * Linux: Check if TLP is installed
     * Windows: Check if power plans are accessible
     * @return true if power management is supported and working
     */
    virtual bool isAvailable() = 0;
};
