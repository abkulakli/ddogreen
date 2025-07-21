#pragma once

#include "platform/isystem_monitor.h"
#include "platform/ipower_manager.h"
#include "platform/iplatform_utils.h"
#include "platform/idaemon.h"
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
     * Create a daemon for the current platform
     * @return unique_ptr to platform-specific daemon implementation
     */
    static std::unique_ptr<IDaemon> createDaemon();

    /**
     * Get the current platform name
     * @return "linux", "windows", or "unknown"
     */
    static std::string getCurrentPlatform();

private:
    PlatformFactory() = default;  // Static class, no instances
};
