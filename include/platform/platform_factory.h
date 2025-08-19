#ifndef DDOGREEN_PLATFORM_FACTORY_H
#define DDOGREEN_PLATFORM_FACTORY_H

#include "platform/isystem_monitor.h"
#include "platform/ipower_manager.h"
#include "platform/iplatform_utils.h"
#include "platform/isignal_handler.h"
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
     * Create platform utilities for the current platform
     * @return unique_ptr to platform-specific platform utilities implementation
     */
    static std::unique_ptr<IPlatformUtils> createPlatformUtils();

    /**
     * Create a signal handler for the current platform
     * @return unique_ptr to platform-specific signal handler implementation
     */
    static std::unique_ptr<ISignalHandler> createSignalHandler();

    /**
     * Get the current platform name
     * @return "linux", "windows", or "unknown"
     */
    static std::string getCurrentPlatform();

private:
    PlatformFactory() = default;
};

#endif // DDOGREEN_PLATFORM_FACTORY_H
