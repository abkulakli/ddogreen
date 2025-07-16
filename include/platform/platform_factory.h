#pragma once

#include "platform/isystem_monitor.h"
#include "platform/ipower_manager.h"
#include "platform/iservice_manager.h"
#include <memory>

/**
 * Platform factory for creating platform-specific implementations
 * Automatically detects the current platform and returns appropriate implementations
 */
class PlatformFactory {
public:
    /**
     * Create a system monitor for the current platform
     * @return unique_ptr to platform-specific system monitor implementation
     */
    static std::unique_ptr<ISystemMonitor> createSystemMonitor();

    /**
     * Create a power manager for the current platform
     * @return unique_ptr to platform-specific power manager implementation
     */
    static std::unique_ptr<IPowerManager> createPowerManager();

    /**
     * Create a service manager for the current platform
     * @return unique_ptr to platform-specific service manager implementation
     */
    static std::unique_ptr<IServiceManager> createServiceManager();

    /**
     * Get the current platform name
     * @return "linux", "windows", or "unknown"
     */
    static std::string getCurrentPlatform();

private:
    PlatformFactory() = default;  // Static class, no instances
};
