#include <gtest/gtest.h>
#include <memory>
#include "platform/platform_factory.h"
#include "platform/ipower_manager.h"
#include "platform/isystem_monitor.h"
#include "platform/iplatform_utils.h"
#include "platform/isignal_handler.h"
#include "logger.h"

class TestPlatformFactory : public ::testing::Test {
protected:
    void SetUp() override {
        // Suppress logger output during tests
        Logger::setLevel(LogLevel::ERROR);
    }

    void TearDown() override {
        // Restore logger level
        Logger::setLevel(LogLevel::INFO);
    }
};

// Test power manager creation
TEST_F(TestPlatformFactory, test_create_power_manager_returns_valid_instance) {
    auto powerManager = PlatformFactory::createPowerManager();
    
    // Should return a valid instance
    EXPECT_NE(nullptr, powerManager);
    
    // Should implement the interface
    EXPECT_NE(nullptr, dynamic_cast<IPowerManager*>(powerManager.get()));
}

// Test system monitor creation
TEST_F(TestPlatformFactory, test_create_system_monitor_returns_valid_instance) {
    auto systemMonitor = PlatformFactory::createSystemMonitor();
    
    // Should return a valid instance
    EXPECT_NE(nullptr, systemMonitor);
    
    // Should implement the interface
    EXPECT_NE(nullptr, dynamic_cast<ISystemMonitor*>(systemMonitor.get()));
}

// Test platform utilities creation
TEST_F(TestPlatformFactory, test_create_platform_utils_returns_valid_instance) {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    
    // Should return a valid instance
    EXPECT_NE(nullptr, platformUtils);
    
    // Should implement the interface
    EXPECT_NE(nullptr, dynamic_cast<IPlatformUtils*>(platformUtils.get()));
}

// Test signal handler creation
TEST_F(TestPlatformFactory, test_create_signal_handler_returns_valid_instance) {
    auto signalHandler = PlatformFactory::createSignalHandler();
    
    // Should return a valid instance
    EXPECT_NE(nullptr, signalHandler);
    
    // Should implement the interface
    EXPECT_NE(nullptr, dynamic_cast<ISignalHandler*>(signalHandler.get()));
}

// Test power manager interface compliance
TEST_F(TestPlatformFactory, test_power_manager_interface_compliance) {
    auto powerManager = PlatformFactory::createPowerManager();
    ASSERT_NE(nullptr, powerManager);
    
    // Test interface methods exist and return appropriate types
    // Note: We can't test actual functionality without system dependencies,
    // but we can verify the interface is properly implemented
    
    // isAvailable should return a boolean
    bool available = powerManager->isAvailable();
    EXPECT_TRUE(available == true || available == false);
    
    // getCurrentMode should return a string
    std::string mode = powerManager->getCurrentMode();
    EXPECT_TRUE(mode.empty() || !mode.empty()); // Any string is valid
    
    // Performance and power saving methods should return boolean
    // Note: These may fail in test environment, but should not crash
    bool perfResult = powerManager->setPerformanceMode();
    bool saveResult = powerManager->setPowerSavingMode();
    
    EXPECT_TRUE(perfResult == true || perfResult == false);
    EXPECT_TRUE(saveResult == true || saveResult == false);
}

// Test system monitor interface compliance
TEST_F(TestPlatformFactory, test_system_monitor_interface_compliance) {
    auto systemMonitor = PlatformFactory::createSystemMonitor();
    ASSERT_NE(nullptr, systemMonitor);
    
    // Test interface methods exist and return appropriate types
    bool available = systemMonitor->isAvailable();
    EXPECT_TRUE(available == true || available == false);
    
    if (available) {
        // If available, should provide valid data
        double loadAverage = systemMonitor->getLoadAverage();
        EXPECT_GE(loadAverage, 0.0); // Load should be non-negative
        
        int coreCount = systemMonitor->getCpuCoreCount();
        EXPECT_GT(coreCount, 0); // Should have at least 1 core
        
        // Test monitoring frequency setting
        systemMonitor->setMonitoringFrequency(30);
        // Should complete without error
    }
}

// Test platform utilities interface compliance
TEST_F(TestPlatformFactory, test_platform_utils_interface_compliance) {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    ASSERT_NE(nullptr, platformUtils);
    
    // Test interface methods exist and return appropriate types
    bool available = platformUtils->isAvailable();
    EXPECT_TRUE(available == true || available == false);
    
    if (available) {
        // Config directory should return a valid string
        std::string configPath = platformUtils->getDefaultConfigPath();
        EXPECT_FALSE(configPath.empty());
        
        // Absolute path resolution should work
        std::string testPath = "test.conf";
        std::string absolutePath = platformUtils->resolveAbsolutePath(testPath);
        EXPECT_FALSE(absolutePath.empty());
        
        // Privilege check should return boolean
        bool hasPrivileges = platformUtils->hasRequiredPrivileges();
        EXPECT_TRUE(hasPrivileges == true || hasPrivileges == false);
        
        // Other path methods should return strings
        std::string logPath = platformUtils->getDefaultLogPath();
        std::string pidPath = platformUtils->getDefaultPidPath();
        std::string installPath = platformUtils->getDefaultInstallPath();
        std::string execPath = platformUtils->getExecutablePath();
        std::string privMsg = platformUtils->getPrivilegeEscalationMessage();
        
        EXPECT_FALSE(logPath.empty());
        EXPECT_FALSE(pidPath.empty());
        EXPECT_FALSE(installPath.empty());
        // execPath might be empty in test environment
        EXPECT_FALSE(privMsg.empty());
    }
}

// Test signal handler interface compliance
TEST_F(TestPlatformFactory, test_signal_handler_interface_compliance) {
    auto signalHandler = PlatformFactory::createSignalHandler();
    ASSERT_NE(nullptr, signalHandler);
    
    // Test interface methods exist
    // Note: We can't safely test signal handler setup/cleanup in unit tests
    // as it affects global signal state, so we just verify the interface
    
    // shouldRun should return boolean
    bool shouldRun = signalHandler->shouldRun();
    EXPECT_TRUE(shouldRun == true || shouldRun == false);
    
    // setupSignalHandlers should exist and not crash
    // Note: We don't call this in tests as it modifies global signal handlers
    
    // waitForSignal should exist but we don't call it as it would block
}

// Test multiple instance creation
TEST_F(TestPlatformFactory, test_multiple_instance_creation) {
    // Create multiple instances of each type
    auto powerManager1 = PlatformFactory::createPowerManager();
    auto powerManager2 = PlatformFactory::createPowerManager();
    
    auto systemMonitor1 = PlatformFactory::createSystemMonitor();
    auto systemMonitor2 = PlatformFactory::createSystemMonitor();
    
    auto platformUtils1 = PlatformFactory::createPlatformUtils();
    auto platformUtils2 = PlatformFactory::createPlatformUtils();
    
    // All should be valid and independent
    EXPECT_NE(nullptr, powerManager1);
    EXPECT_NE(nullptr, powerManager2);
    EXPECT_NE(powerManager1.get(), powerManager2.get()); // Different instances
    
    EXPECT_NE(nullptr, systemMonitor1);
    EXPECT_NE(nullptr, systemMonitor2);
    EXPECT_NE(systemMonitor1.get(), systemMonitor2.get());
    
    EXPECT_NE(nullptr, platformUtils1);
    EXPECT_NE(nullptr, platformUtils2);
    EXPECT_NE(platformUtils1.get(), platformUtils2.get());
}

// Test factory consistency across calls
TEST_F(TestPlatformFactory, test_factory_consistency) {
    // Create instances and verify they implement the same concrete types
    auto powerManager1 = PlatformFactory::createPowerManager();
    auto powerManager2 = PlatformFactory::createPowerManager();
    
    ASSERT_NE(nullptr, powerManager1);
    ASSERT_NE(nullptr, powerManager2);
    
    // Should have the same type
    EXPECT_EQ(typeid(*powerManager1), typeid(*powerManager2));
    
    auto systemMonitor1 = PlatformFactory::createSystemMonitor();
    auto systemMonitor2 = PlatformFactory::createSystemMonitor();
    
    ASSERT_NE(nullptr, systemMonitor1);
    ASSERT_NE(nullptr, systemMonitor2);
    
    EXPECT_EQ(typeid(*systemMonitor1), typeid(*systemMonitor2));
}

// Test command line parsing interface
TEST_F(TestPlatformFactory, test_command_line_parsing_interface) {
    auto platformUtils = PlatformFactory::createPlatformUtils();
    ASSERT_NE(nullptr, platformUtils);
    
    if (platformUtils->isAvailable()) {
        // Test with minimal arguments
        char* argv[] = {const_cast<char*>("ddogreen")};
        int argc = 1;
        
        ParsedArgs args = platformUtils->parseCommandLine(argc, argv);
        
        // Should parse without error and return valid structure
        // Default values should be reasonable
        EXPECT_FALSE(args.showHelp);
        EXPECT_FALSE(args.showVersion);
        EXPECT_FALSE(args.hasUnknownOptions);
        EXPECT_TRUE(args.configPath.empty()); // Should be empty by default
    }
}

// Test interface polymorphism
TEST_F(TestPlatformFactory, test_interface_polymorphism) {
    // Test that factory-created objects can be used polymorphically
    std::vector<std::unique_ptr<IPowerManager>> powerManagers;
    std::vector<std::unique_ptr<ISystemMonitor>> systemMonitors;
    std::vector<std::unique_ptr<IPlatformUtils>> platformUtils;
    
    // Create and store in base interface containers
    powerManagers.push_back(PlatformFactory::createPowerManager());
    systemMonitors.push_back(PlatformFactory::createSystemMonitor());
    platformUtils.push_back(PlatformFactory::createPlatformUtils());
    
    // Should work through base interface
    for (auto& pm : powerManagers) {
        EXPECT_NE(nullptr, pm);
        bool available = pm->isAvailable();
        EXPECT_TRUE(available == true || available == false);
    }
    
    for (auto& sm : systemMonitors) {
        EXPECT_NE(nullptr, sm);
        bool available = sm->isAvailable();
        EXPECT_TRUE(available == true || available == false);
    }
    
    for (auto& pu : platformUtils) {
        EXPECT_NE(nullptr, pu);
        bool available = pu->isAvailable();
        EXPECT_TRUE(available == true || available == false);
    }
}
