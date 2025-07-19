#include "daemon.h"
#include "platform/platform_factory.h"
#include "logger.h"

std::unique_ptr<IDaemon> Daemon::s_daemon = nullptr;

bool Daemon::initialize() {
    if (s_daemon) {
        Logger::warning("Daemon already initialized");
        return true;
    }

    s_daemon = PlatformFactory::createDaemon();
    if (!s_daemon) {
        Logger::error("Failed to create platform-specific daemon");
        return false;
    }

    Logger::info("Daemon initialized for platform: " + PlatformFactory::getCurrentPlatform());
    return true;
}

bool Daemon::daemonize() {
    if (!s_daemon) {
        Logger::error("Daemon not initialized. Call initialize() first.");
        return false;
    }
    
    return s_daemon->daemonize();
}

void Daemon::setupSignalHandlers() {
    if (!s_daemon) {
        Logger::error("Daemon not initialized. Call initialize() first.");
        return;
    }
    
    s_daemon->setupSignalHandlers();
}

bool Daemon::shouldRun() {
    if (!s_daemon) {
        Logger::error("Daemon not initialized. Call initialize() first.");
        return false;
    }
    
    return s_daemon->shouldRun();
}

void Daemon::cleanup() {
    s_daemon.reset();
    Logger::info("Daemon cleanup completed");
}
