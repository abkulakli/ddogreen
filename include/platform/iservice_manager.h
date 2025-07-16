#pragma once

#include <string>

/**
 * Interface for platform-specific service management functionality
 * Handles installation, uninstallation, and control of system services
 */
class IServiceManager {
public:
    virtual ~IServiceManager() = default;

    /**
     * Install the service
     * Linux: Install systemd service
     * Windows: Install Windows Service
     * @param serviceName name of the service
     * @param executablePath full path to the executable
     * @param description service description
     * @return true if successful
     */
    virtual bool installService(const std::string& serviceName, 
                               const std::string& executablePath,
                               const std::string& description) = 0;

    /**
     * Uninstall the service
     * @param serviceName name of the service to uninstall
     * @return true if successful
     */
    virtual bool uninstallService(const std::string& serviceName) = 0;

    /**
     * Start the service
     * @param serviceName name of the service to start
     * @return true if successful
     */
    virtual bool startService(const std::string& serviceName) = 0;

    /**
     * Stop the service
     * @param serviceName name of the service to stop
     * @return true if successful
     */
    virtual bool stopService(const std::string& serviceName) = 0;

    /**
     * Enable the service to start automatically
     * @param serviceName name of the service to enable
     * @return true if successful
     */
    virtual bool enableService(const std::string& serviceName) = 0;

    /**
     * Disable the service from starting automatically
     * @param serviceName name of the service to disable
     * @return true if successful
     */
    virtual bool disableService(const std::string& serviceName) = 0;

    /**
     * Check if service management is available on this platform
     * @return true if service management is supported
     */
    virtual bool isAvailable() = 0;

    /**
     * Get service status
     * @param serviceName name of the service
     * @return status string ("running", "stopped", "unknown")
     */
    virtual std::string getServiceStatus(const std::string& serviceName) = 0;
};
