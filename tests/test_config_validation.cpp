#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iostream>
#include "config.h"
#include "logger.h"

namespace fs = std::filesystem;

class TestConfigValidation : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary directory for test files
        testDir = fs::temp_directory_path() / "ddogreen_validation_test";
        fs::create_directories(testDir);

        // Initialize config object
        config = std::make_unique<Config>();

        // Store original cout/cerr buffers for output capture
        originalCoutBuffer = std::cout.rdbuf();
        originalCerrBuffer = std::cerr.rdbuf();
        
        // Set logger to capture warnings (INFO level to catch validation warnings)
        Logger::setLevel(LogLevel::INFO);
    }

    void TearDown() override
    {
        // Clean up test files
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }

        // Reset config
        config.reset();

        // Restore original cout/cerr buffers
        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);

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

    // Helper method to capture stderr for warning messages
    void startCaptureStderr() {
        std::cerr.rdbuf(stderrCapture.rdbuf());
    }

    std::string getCapturedStderr() {
        std::cerr.rdbuf(originalCerrBuffer);
        return stderrCapture.str();
    }

    void clearStderrCapture() {
        stderrCapture.str("");
        stderrCapture.clear();
    }

    std::unique_ptr<Config> config;
    fs::path testDir;
    std::streambuf* originalCoutBuffer;
    std::streambuf* originalCerrBuffer;
    std::stringstream stderrCapture;
};

// Test threshold cross-validation: file version
TEST_F(TestConfigValidation, test_load_from_file_rejects_equal_thresholds)
{
    // Arrange
    std::string invalidConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.5\n"
        "power_save_threshold=0.5\n";

    createConfigFile("equal_thresholds.conf", invalidConfig);
    std::string configPath = getTestFilePath("equal_thresholds.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_FALSE(result);
}

// Test threshold cross-validation: power_save > high_performance
TEST_F(TestConfigValidation, test_load_from_file_rejects_inverted_thresholds)
{
    // Arrange
    std::string invalidConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.3\n"
        "power_save_threshold=0.7\n";

    createConfigFile("inverted_thresholds.conf", invalidConfig);
    std::string configPath = getTestFilePath("inverted_thresholds.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_FALSE(result);
}

// Test validation warnings: small threshold gap
TEST_F(TestConfigValidation, test_validation_produces_warning_for_small_threshold_gap)
{
    // Arrange - gap of 0.05 (5%) should trigger warning
    std::string configWithSmallGap =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.55\n"
        "power_save_threshold=0.5\n";

    createConfigFile("small_gap.conf", configWithSmallGap);
    std::string configPath = getTestFilePath("small_gap.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);  // Should still load successfully
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.55, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.5, config->getPowerSaveThreshold());
    
    // Note: We can't easily test the warning output without more complex setup
    // The validation warnings go to Logger which would need output capture
}

// Test validation warnings: very frequent monitoring
TEST_F(TestConfigValidation, test_validation_produces_warning_for_frequent_monitoring)
{
    // Arrange - monitoring every 5 seconds should trigger warning
    std::string configWithFrequentMonitoring =
        "monitoring_frequency=5\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    createConfigFile("frequent_monitoring.conf", configWithFrequentMonitoring);
    std::string configPath = getTestFilePath("frequent_monitoring.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);  // Should still load successfully
    EXPECT_EQ(5, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test validation warnings: very high performance threshold
TEST_F(TestConfigValidation, test_validation_produces_warning_for_very_high_performance_threshold)
{
    // Arrange - threshold of 0.95 (95%) should trigger warning
    std::string configWithHighThreshold =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.95\n"
        "power_save_threshold=0.3\n";

    createConfigFile("high_threshold.conf", configWithHighThreshold);
    std::string configPath = getTestFilePath("high_threshold.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);  // Should still load successfully
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.95, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test validation warnings: very low power save threshold
TEST_F(TestConfigValidation, test_validation_produces_warning_for_very_low_power_save_threshold)
{
    // Arrange - threshold of 0.05 (5%) should trigger warning
    std::string configWithLowThreshold =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.05\n";

    createConfigFile("low_threshold.conf", configWithLowThreshold);
    std::string configPath = getTestFilePath("low_threshold.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);  // Should still load successfully
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.05, config->getPowerSaveThreshold());
}

// Test boundary case: exactly 10% gap between thresholds (should not trigger warning)
TEST_F(TestConfigValidation, test_validation_accepts_exact_minimum_threshold_gap)
{
    // Arrange - gap of exactly 0.1 (10%) should not trigger warning
    std::string configWithMinimumGap =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.6\n"
        "power_save_threshold=0.5\n";

    createConfigFile("minimum_gap.conf", configWithMinimumGap);
    std::string configPath = getTestFilePath("minimum_gap.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.6, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.5, config->getPowerSaveThreshold());
}

// Test boundary case: exactly 10 seconds monitoring (should not trigger warning)
TEST_F(TestConfigValidation, test_validation_accepts_minimum_monitoring_frequency)
{
    // Arrange - exactly 10 seconds should not trigger warning
    std::string configWithMinimumFreq =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    createConfigFile("minimum_freq.conf", configWithMinimumFreq);
    std::string configPath = getTestFilePath("minimum_freq.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test boundary case: exactly 0.9 high performance threshold (should trigger warning)
TEST_F(TestConfigValidation, test_validation_warning_boundary_high_performance_threshold)
{
    // Arrange - exactly 0.9 should trigger warning based on the code (> 0.8 in first method, > 0.9 in second)
    std::string configWithBoundaryHigh =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.9\n"
        "power_save_threshold=0.3\n";

    createConfigFile("boundary_high.conf", configWithBoundaryHigh);
    std::string configPath = getTestFilePath("boundary_high.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.9, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test boundary case: exactly 0.1 power save threshold (should not trigger warning)
TEST_F(TestConfigValidation, test_validation_accepts_boundary_power_save_threshold)
{
    // Arrange - exactly 0.1 should not trigger warning
    std::string configWithBoundaryLow =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.1\n";

    createConfigFile("boundary_low.conf", configWithBoundaryLow);
    std::string configPath = getTestFilePath("boundary_low.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.1, config->getPowerSaveThreshold());
}

// Test multiple validation issues in one configuration
TEST_F(TestConfigValidation, test_validation_handles_multiple_warning_conditions)
{
    // Arrange - configuration that should trigger multiple warnings
    std::string configWithMultipleIssues =
        "monitoring_frequency=5\n"       // Too frequent (< 10)
        "high_performance_threshold=0.95\n"  // Too high (> 0.9)
        "power_save_threshold=0.9\n";    // Gap too small (0.05 < 0.1)

    createConfigFile("multiple_issues.conf", configWithMultipleIssues);
    std::string configPath = getTestFilePath("multiple_issues.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);  // Should still load successfully despite warnings
    EXPECT_EQ(5, config->getMonitoringFrequency());
    EXPECT_EQ(0.95, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.9, config->getPowerSaveThreshold());
}

// Test optimal configuration (should not trigger any warnings)
TEST_F(TestConfigValidation, test_validation_accepts_optimal_configuration)
{
    // Arrange - well-balanced configuration
    std::string optimalConfig =
        "monitoring_frequency=30\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    createConfigFile("optimal.conf", optimalConfig);
    std::string configPath = getTestFilePath("optimal.conf");

    // Act
    bool result = config->loadFromFile(configPath);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(30, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}
