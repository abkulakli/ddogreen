#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "config.h"
#include "logger.h"

namespace fs = std::filesystem;

class TestConfig : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary directory for test files
        testDir = fs::temp_directory_path() / "ddogreen_test";
        fs::create_directories(testDir);
        
        // Initialize config object
        config = std::make_unique<Config>();
        
        // Suppress logger output during tests (set to highest level)
        Logger::setLevel(LogLevel::ERROR);
    }

    void TearDown() override
    {
        // Clean up test files
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
        
        // Reset config
        config.reset();
        
        // Restore logger level
        Logger::setLevel(LogLevel::INFO);
    }

    // Helper method to create test configuration file
    void createConfigFile(const std::string& filename, const std::string& content)
    {
        std::ofstream file(testDir / filename);
        file << content;
        file.close();
    }

    // Helper method to get test file path
    std::string getTestFilePath(const std::string& filename)
    {
        return (testDir / filename).string();
    }

    std::unique_ptr<Config> config;
    fs::path testDir;
};

// Test constructor initialization
TEST_F(TestConfig, test_constructor_initializes_values_to_zero)
{
    // Assert
    EXPECT_EQ(0, config->getMonitoringFrequency());
    EXPECT_EQ(0.0, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.0, config->getPowerSaveThreshold());
}

// Test default config path (platform-specific)
TEST_F(TestConfig, test_get_default_config_path_returns_platform_specific_path)
{
    // Act
    std::string defaultPath = Config::getDefaultConfigPath();
    
    // Assert - Should return a non-empty path that contains ddogreen.conf
    EXPECT_FALSE(defaultPath.empty());
    EXPECT_TRUE(defaultPath.find("ddogreen.conf") != std::string::npos);
    
    // Verify platform-specific path expectations
#if defined(_WIN32) || defined(_WIN64)
    // On Windows, should be in ProgramData
    EXPECT_TRUE(defaultPath.find("DDOSoft") != std::string::npos);
    EXPECT_TRUE(defaultPath.find("ddogreen") != std::string::npos);
#elif defined(__linux__)
    // On Linux, should be in /etc
    EXPECT_EQ("/etc/ddogreen/ddogreen.conf", defaultPath);
#elif defined(__APPLE__) && defined(__MACH__)
    // On macOS, should be in /etc (same as Linux for now)
    EXPECT_EQ("/etc/ddogreen/ddogreen.conf", defaultPath);
#endif
}

// Test successful configuration loading
TEST_F(TestConfig, test_load_from_file_returns_true_with_valid_config)
{
    // Arrange
    std::string validConfig = 
        "# ddogreen configuration\n"
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("valid.conf", validConfig);
    std::string configPath = getTestFilePath("valid.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test loading with comments and empty lines
TEST_F(TestConfig, test_load_from_file_handles_comments_and_empty_lines)
{
    // Arrange
    std::string configWithComments = 
        "# This is a comment\n"
        "\n"
        "# Another comment\n"
        "monitoring_frequency=15\n"
        "\n"
        "# Threshold settings\n"
        "high_performance_threshold=0.8\n"
        "\n"
        "power_save_threshold=0.2\n"
        "# End comment\n";
    
    createConfigFile("comments.conf", configWithComments);
    std::string configPath = getTestFilePath("comments.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(15, config->getMonitoringFrequency());
    EXPECT_EQ(0.8, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.2, config->getPowerSaveThreshold());
}

// Test loading with whitespace
TEST_F(TestConfig, test_load_from_file_handles_whitespace)
{
    // Arrange
    std::string configWithWhitespace = 
        "  monitoring_frequency  =  20  \n"
        "\t high_performance_threshold\t=\t0.6\t\n"
        " power_save_threshold = 0.4 \n";
    
    createConfigFile("whitespace.conf", configWithWhitespace);
    std::string configPath = getTestFilePath("whitespace.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(20, config->getMonitoringFrequency());
    EXPECT_EQ(0.6, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.4, config->getPowerSaveThreshold());
}

// Test file not found
TEST_F(TestConfig, test_load_from_file_returns_false_when_file_not_found)
{
    // Arrange
    std::string nonExistentPath = getTestFilePath("nonexistent.conf");
    
    // Act
    bool result = config->loadFromFile(nonExistentPath);
    
    // Assert
    EXPECT_FALSE(result);
}

// Test missing required configuration
TEST_F(TestConfig, test_load_from_file_returns_false_when_missing_monitoring_frequency)
{
    // Arrange
    std::string incompleteConfig = 
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("missing_freq.conf", incompleteConfig);
    std::string configPath = getTestFilePath("missing_freq.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_returns_false_when_missing_high_performance_threshold)
{
    // Arrange
    std::string incompleteConfig = 
        "monitoring_frequency=10\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("missing_high.conf", incompleteConfig);
    std::string configPath = getTestFilePath("missing_high.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_returns_false_when_missing_power_save_threshold)
{
    // Arrange
    std::string incompleteConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n";
    
    createConfigFile("missing_power.conf", incompleteConfig);
    std::string configPath = getTestFilePath("missing_power.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

// Test invalid values - monitoring frequency
TEST_F(TestConfig, test_load_from_file_rejects_monitoring_frequency_below_minimum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=0\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("freq_low.conf", invalidConfig);
    std::string configPath = getTestFilePath("freq_low.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_rejects_monitoring_frequency_above_maximum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=301\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("freq_high.conf", invalidConfig);
    std::string configPath = getTestFilePath("freq_high.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_accepts_monitoring_frequency_boundary_values)
{
    // Test minimum boundary
    std::string minConfig = 
        "monitoring_frequency=1\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("freq_min.conf", minConfig);
    std::string minPath = getTestFilePath("freq_min.conf");
    
    bool minResult = config->loadFromFile(minPath);
    EXPECT_TRUE(minResult);
    EXPECT_EQ(1, config->getMonitoringFrequency());
    
    // Reset config for next test
    config = std::make_unique<Config>();
    
    // Test maximum boundary
    std::string maxConfig = 
        "monitoring_frequency=300\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("freq_max.conf", maxConfig);
    std::string maxPath = getTestFilePath("freq_max.conf");
    
    bool maxResult = config->loadFromFile(maxPath);
    EXPECT_TRUE(maxResult);
    EXPECT_EQ(300, config->getMonitoringFrequency());
}

// Test invalid values - high performance threshold
TEST_F(TestConfig, test_load_from_file_rejects_high_performance_threshold_below_minimum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.05\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("high_low.conf", invalidConfig);
    std::string configPath = getTestFilePath("high_low.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_rejects_high_performance_threshold_above_maximum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=1.1\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("high_high.conf", invalidConfig);
    std::string configPath = getTestFilePath("high_high.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_accepts_high_performance_threshold_boundary_values)
{
    // Test minimum boundary
    std::string minConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.1\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("high_min.conf", minConfig);
    std::string minPath = getTestFilePath("high_min.conf");
    
    bool minResult = config->loadFromFile(minPath);
    EXPECT_TRUE(minResult);
    EXPECT_EQ(0.1, config->getHighPerformanceThreshold());
    
    // Reset config for next test
    config = std::make_unique<Config>();
    
    // Test maximum boundary
    std::string maxConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=1.0\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("high_max.conf", maxConfig);
    std::string maxPath = getTestFilePath("high_max.conf");
    
    bool maxResult = config->loadFromFile(maxPath);
    EXPECT_TRUE(maxResult);
    EXPECT_EQ(1.0, config->getHighPerformanceThreshold());
}

// Test invalid values - power save threshold
TEST_F(TestConfig, test_load_from_file_rejects_power_save_threshold_below_minimum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.04\n";
    
    createConfigFile("power_low.conf", invalidConfig);
    std::string configPath = getTestFilePath("power_low.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_rejects_power_save_threshold_above_maximum)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.91\n";
    
    createConfigFile("power_high.conf", invalidConfig);
    std::string configPath = getTestFilePath("power_high.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(TestConfig, test_load_from_file_accepts_power_save_threshold_boundary_values)
{
    // Test minimum boundary
    std::string minConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.05\n";
    
    createConfigFile("power_min.conf", minConfig);
    std::string minPath = getTestFilePath("power_min.conf");
    
    bool minResult = config->loadFromFile(minPath);
    EXPECT_TRUE(minResult);
    EXPECT_EQ(0.05, config->getPowerSaveThreshold());
    
    // Reset config for next test
    config = std::make_unique<Config>();
    
    // Test maximum boundary
    std::string maxConfig = 
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.9\n";
    
    createConfigFile("power_max.conf", maxConfig);
    std::string maxPath = getTestFilePath("power_max.conf");
    
    bool maxResult = config->loadFromFile(maxPath);
    EXPECT_TRUE(maxResult);
    EXPECT_EQ(0.9, config->getPowerSaveThreshold());
}

// Test malformed configuration lines
TEST_F(TestConfig, test_load_from_file_rejects_malformed_lines)
{
    // Arrange
    std::string malformedConfig = 
        "monitoring_frequency=10\n"
        "invalid_line_without_equals\n"
        "high_performance_threshold=0.7\n"
        "=value_without_key\n"
        "power_save_threshold=0.3\n"
        "key_without_value=\n";
    
    createConfigFile("malformed.conf", malformedConfig);
    std::string configPath = getTestFilePath("malformed.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result); // Should fail due to malformed lines
}

// Test invalid data types
TEST_F(TestConfig, test_load_from_file_rejects_non_numeric_values)
{
    // Arrange
    std::string invalidConfig = 
        "monitoring_frequency=abc\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("non_numeric.conf", invalidConfig);
    std::string configPath = getTestFilePath("non_numeric.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

// Test unknown configuration keys
TEST_F(TestConfig, test_load_from_file_rejects_unknown_keys)
{
    // Arrange
    std::string configWithUnknown = 
        "monitoring_frequency=10\n"
        "unknown_setting=value\n"
        "high_performance_threshold=0.7\n"
        "another_unknown=123\n"
        "power_save_threshold=0.3\n";
    
    createConfigFile("unknown_keys.conf", configWithUnknown);
    std::string configPath = getTestFilePath("unknown_keys.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result); // Should fail due to unknown keys
}

// Test getters return correct values
TEST_F(TestConfig, test_getters_return_loaded_values)
{
    // Arrange
    std::string validConfig = 
        "monitoring_frequency=25\n"
        "high_performance_threshold=0.85\n"
        "power_save_threshold=0.15\n";
    
    createConfigFile("getters.conf", validConfig);
    std::string configPath = getTestFilePath("getters.conf");
    
    config->loadFromFile(configPath);
    
    // Act & Assert
    EXPECT_EQ(25, config->getMonitoringFrequency());
    EXPECT_EQ(0.85, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.15, config->getPowerSaveThreshold());
}

// Test edge case: empty file
TEST_F(TestConfig, test_load_from_file_rejects_empty_file)
{
    // Arrange
    createConfigFile("empty.conf", "");
    std::string configPath = getTestFilePath("empty.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}

// Test edge case: only comments
TEST_F(TestConfig, test_load_from_file_rejects_comments_only_file)
{
    // Arrange
    std::string commentsOnly = 
        "# This is a comment\n"
        "# Another comment\n"
        "# Yet another comment\n";
    
    createConfigFile("comments_only.conf", commentsOnly);
    std::string configPath = getTestFilePath("comments_only.conf");
    
    // Act
    bool result = config->loadFromFile(configPath);
    
    // Assert
    EXPECT_FALSE(result);
}
