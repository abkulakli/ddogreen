#pragma once

/**
 * Interface for platform-specific daemon implementations
 * Provides abstraction for daemon/service lifecycle management
 */
class IDaemon {
public:
    virtual ~IDaemon() = default;

    /**
     * Daemonize the process (platform-specific)
     * @return true if successful, false otherwise
     */
    virtual bool daemonize() = 0;

    /**
     * Setup platform-specific signal handlers
     */
    virtual void setupSignalHandlers() = 0;

    /**
     * Check if daemon should continue running
     * @return true if daemon should continue, false to shutdown
     */
    virtual bool shouldRun() = 0;

    /**
     * Wait for a termination signal (blocks until signal received)
     * This eliminates the need for busy waiting in the main loop
     */
    virtual void waitForSignal() = 0;

protected:
    static volatile bool s_running;
};
