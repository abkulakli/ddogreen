#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <filesystem>

// Simple test fixture that creates temporary config files
class TestConfigPlatform : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        testDir = std::filesystem::temp_directory_path() / "ddogreen_config_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    void createConfigFile(const std::string& filename, const std::string& content) {
        std::filesystem::path filePath = testDir / filename;
        std::ofstream file(filePath);
        file << content;
        file.close();
    }

    std::string getTestFilePath(const std::string& filename) {
        return (testDir / filename).string();
    }

    std::filesystem::path testDir;
};

// Test that Config uses platform utilities for default path
TEST_F(TestConfigPlatform, test_config_uses_platform_default_path)
{
    // Act - This will use the real platform utilities 
    std::string defaultPath = Config::getDefaultConfigPath();
    
    // Assert - Should return a valid path (not empty)
    EXPECT_FALSE(defaultPath.empty());
    EXPECT_TRUE(defaultPath.find("ddogreen.conf") != std::string::npos);
    
    // On Linux, should be the /etc path
    #ifndef _WIN32
    EXPECT_EQ("/etc/ddogreen/ddogreen.conf", defaultPath);
    #endif
}

// Test Config loading with explicit path (doesn't depend on platform)
TEST_F(TestConfigPlatform, test_config_loads_with_explicit_path)
{
    // Arrange
    std::string validConfig = 
        "# Test configuration\n"
        "high_performance_threshold=0.75\n"
        "power_save_threshold=0.25\n"
        "monitoring_frequency=15\n";
    
    createConfigFile("explicit.conf", validConfig);
    std::string configPath = getTestFilePath("explicit.conf");
    
    Config config;
    
    // Act
    bool result = config.loadFromFile(configPath);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(15, config.getMonitoringFrequency());
    EXPECT_DOUBLE_EQ(0.75, config.getHighPerformanceThreshold());
    EXPECT_DOUBLE_EQ(0.25, config.getPowerSaveThreshold());
}
