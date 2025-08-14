/*
 * DDOGreen - Intelligent Green Power Management
 *
 * Automatically manages TLP power settings based on system load monitoring
 *
 * Copyright (c) 2025 DDOSoft Solutions (www.ddosoft.com)
 *
 * This program automatically switches between performance and power-saving modes
 * based on load average monitoring, providing
 * intelligent power management for sustainable computing on PCs and laptops.
 */

#include "activity_monitor.h"
#include "logger.h"
#include "config.h"
#include "platform/platform_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -c, --config PATH      Use custom configuration file\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n"
              << "\n"
              << "Automatically switches between performance and power-saving modes based on system load.\n"
              << "When run as a service, process management is handled by the service manager (systemd/SCM).\n";
}

void printVersion()
{
    std::cout << "DDOGreen version " << DDOGREEN_VERSION << "\n"
              << "DDOGreen - Intelligent Green Power Management\n"
              << "Copyright (c) 2025 DDOSoft Solutions (www.ddosoft.com)\n";
}

void configurePowerManagement(ActivityMonitor& activityMonitor, std::unique_ptr<IPowerManager>& powerManager)
{
    activityMonitor.setActivityCallback([&powerManager](bool isActive) {
        if (isActive)
        {
            powerManager->setPerformanceMode();
        }
        else
        {
            powerManager->setPowerSavingMode();
        }
    });
}

bool validatePowerManagement(const std::unique_ptr<IPowerManager>& powerManager)
{
    Logger::info("Checking power management availability...");
    if (!powerManager || !powerManager->isAvailable())
    {
        Logger::error("Power management backend is not available on this system");
        Logger::error("Ensure a supported power management backend is installed and accessible");
        std::cerr << "Power management backend is not available on this system" << std::endl;
        std::cerr << "Ensure a supported power management backend is installed and accessible" << std::endl;
        return false;
    }
    Logger::info("Power management backend is available");
    return true;
}

void configureMonitoring(ActivityMonitor& activityMonitor, const Config& config)
{
    Logger::info("Configuring activity monitor...");
    activityMonitor.setLoadThresholds(config.getHighPerformanceThreshold(), config.getPowerSaveThreshold());
    activityMonitor.setMonitoringFrequency(config.getMonitoringFrequency());

    Logger::info("High performance threshold: " + std::to_string(config.getHighPerformanceThreshold()));
    Logger::info("Power save threshold: " + std::to_string(config.getPowerSaveThreshold()));
    Logger::info("Monitoring frequency: " + std::to_string(config.getMonitoringFrequency()) + " seconds");
}

void resolveConfigPath(ParsedArgs& args, const std::unique_ptr<IPlatformUtils>& platformUtils)
{
    if (!args.configPath.empty())
    {
        std::string resolvedPath = platformUtils->resolveAbsolutePath(args.configPath);
        if (resolvedPath != args.configPath)
        {
            args.configPath = resolvedPath;
            Logger::info("Converted relative config path to absolute: " + args.configPath);
        }
    }
}

int main(int argc, char* argv[])
{
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (!platformUtils || !platformUtils->isAvailable())
    {
        std::cerr << "Platform utilities are not available on this system" << std::endl;
        return 1;
    }

    ParsedArgs args = platformUtils->parseCommandLine(argc, argv);

    if (args.hasUnknownOptions)
    {
        std::cerr << "Unknown option: " << args.unknownOption << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    if (args.showHelp)
    {
        printUsage(argv[0]);
        return 0;
    }

    if (args.showVersion)
    {
        printVersion();
        return 0;
    }

    if (!platformUtils->hasRequiredPrivileges())
    {
        std::cerr << platformUtils->getPrivilegeEscalationMessage() << std::endl;
        return 1;
    }

    std::cout << "DDOGreen starting" << std::endl;
    std::string logPath = platformUtils->getDefaultLogPath();
    Logger::init(logPath, false);
    std::cout << "DDOGreen is logging to file " << logPath << std::endl;

    Logger::info("Starting DDOGreen - Intelligent Green Power Management");
    Logger::info("Version: " + std::string(DDOGREEN_VERSION));
    Logger::info("Copyright (c) 2025 DDOSoft Solutions");

    resolveConfigPath(args, platformUtils);

    auto signalHandler = PlatformFactory::createSignalHandler();
    if (!signalHandler)
    {
        Logger::error("Failed to create signal handler");
        std::cerr << "Failed to create signal handler" << std::endl;
        return 1;
    }
    signalHandler->setupSignalHandlers();

    Config config;
    std::string configPath = args.configPath.empty() ? Config::getDefaultConfigPath() : args.configPath;

    Logger::info("Loading configuration from: " + configPath);

    if (!config.loadFromFile(configPath))
    {
        Logger::error("Failed to load configuration file: " + configPath);
        std::cerr << "Failed to load configuration file: " << configPath << std::endl;
        return 1;
    }

    Logger::info("Configuration loaded successfully");

    ActivityMonitor activityMonitor;
    auto powerManager = PlatformFactory::createPowerManager();

    if (!validatePowerManagement(powerManager))
    {
        return 1;
    }

    configureMonitoring(activityMonitor, config);
    configurePowerManagement(activityMonitor, powerManager);

    if (!activityMonitor.start())
    {
        Logger::error("Failed to start activity monitor");
        std::cerr << "Failed to start activity monitor" << std::endl;
        return 1;
    }

    Logger::info("DDOGreen service started successfully");
    Logger::info("Version: " + std::string(DDOGREEN_VERSION));
    Logger::info("Copyright (c) 2025 DDOSoft Solutions (www.ddosoft.com)");

    std::cout << "DDOGreen service running - press Ctrl+C to stop" << std::endl;

    try
    {
        signalHandler->waitForSignal();
    }
    catch (const std::exception& e)
    {
        Logger::error("Exception during signal handling: " + std::string(e.what()));
    }

    std::cout << "DDOGreen stopping" << std::endl;
    Logger::info("Shutting down DDOGreen service");

    try
    {
        activityMonitor.stop();
    }
    catch (const std::exception& e)
    {
        Logger::error("Exception during shutdown: " + std::string(e.what()));
    }

    std::cout << "DDOGreen stopped" << std::endl;

    return 0;
}
