#ifndef DDOGREEN_ISIGNAL_HANDLER_H
#define DDOGREEN_ISIGNAL_HANDLER_H

#include <span>

/**
 * Interface for platform-specific signal handling functionality
 * Provides graceful shutdown capabilities
 */
class ISignalHandler
{
public:
    virtual ~ISignalHandler() = default;

    /**
     * Setup platform-specific signal handlers
     */
    virtual void setupSignalHandlers() = 0;

    /**
     * Check if application should continue running
     * @return true if application should continue, false to shutdown
     */
    virtual bool shouldRun() = 0;

    /**
     * Wait for a termination signal (blocks until signal received)
     * This eliminates the need for busy waiting in the main loop
     */
    virtual void waitForSignal() = 0;
};

#endif // DDOGREEN_ISIGNAL_HANDLER_H