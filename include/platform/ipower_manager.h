#pragma once

#include <string>
#include <span>
#include <cstring>
#include <algorithm>

/**
 * Interface for platform-specific power management functionality
 * Handles switching between performance and power-saving modes
 */
class IPowerManager
{
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

    /**
     * Apply power management configuration from buffer data
     * @param configData span containing power management configuration
     * @return true if configuration was applied successfully
     */
    virtual bool applyPowerConfiguration(std::span<const char> configData)
    {
        // Default implementation - platforms can override for span-based config
        return !configData.empty();
    }

    /**
     * Get current power status information
     * @param statusBuffer span to fill with status information
     * @return number of bytes written to buffer
     */
    virtual size_t getPowerStatus(std::span<char> statusBuffer) const
    {
        // Default implementation - basic status
        const char* status = "unknown";
        size_t statusLen = strlen(status);
        if (statusBuffer.size() >= statusLen)
        {
            std::copy_n(status, statusLen, statusBuffer.data());
            return statusLen;
        }
        return 0;
    }
};
