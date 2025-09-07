#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <span>
#include "config.h"
#include "logger.h"

class TestConfigBuffer : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize config object
        config = std::make_unique<Config>();

        // Suppress logger output during tests (set to highest level)
        Logger::setLevel(LogLevel::ERROR);
    }

    void TearDown() override
    {
        // Reset config
        config.reset();

        // Restore logger level
        Logger::setLevel(LogLevel::INFO);
    }

    std::unique_ptr<Config> config;
};

// Test successful configuration loading from buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_returns_true_with_valid_config)
{
    // Arrange
    std::string validConfig =
        "# ddogreen configuration\n"
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{validConfig.data(), validConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.7, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test loading from buffer with comments and empty lines
TEST_F(TestConfigBuffer, test_load_from_buffer_handles_comments_and_empty_lines)
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

    std::span<const char> buffer{configWithComments.data(), configWithComments.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(15, config->getMonitoringFrequency());
    EXPECT_EQ(0.8, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.2, config->getPowerSaveThreshold());
}

// Test loading from buffer with whitespace
TEST_F(TestConfigBuffer, test_load_from_buffer_handles_whitespace)
{
    // Arrange
    std::string configWithWhitespace =
        "  monitoring_frequency  =  20  \n"
        "\t high_performance_threshold\t=\t0.6\t\n"
        " power_save_threshold = 0.4 \n";

    std::span<const char> buffer{configWithWhitespace.data(), configWithWhitespace.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(20, config->getMonitoringFrequency());
    EXPECT_EQ(0.6, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.4, config->getPowerSaveThreshold());
}

// Test loading from buffer with Windows line endings (CRLF)
TEST_F(TestConfigBuffer, test_load_from_buffer_handles_windows_line_endings)
{
    // Arrange
    std::string configWithCRLF =
        "monitoring_frequency=30\r\n"
        "high_performance_threshold=0.9\r\n"
        "power_save_threshold=0.1\r\n";

    std::span<const char> buffer{configWithCRLF.data(), configWithCRLF.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(30, config->getMonitoringFrequency());
    EXPECT_EQ(0.9, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.1, config->getPowerSaveThreshold());
}

// Test empty buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_returns_false_when_empty)
{
    // Arrange
    std::string emptyConfig;
    std::span<const char> buffer{emptyConfig.data(), emptyConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test buffer with only comments
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_comments_only_buffer)
{
    // Arrange
    std::string commentsOnly =
        "# This is a comment\n"
        "# Another comment\n"
        "# Yet another comment\n";

    std::span<const char> buffer{commentsOnly.data(), commentsOnly.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test missing required configuration - monitoring_frequency
TEST_F(TestConfigBuffer, test_load_from_buffer_returns_false_when_missing_monitoring_frequency)
{
    // Arrange
    std::string incompleteConfig =
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{incompleteConfig.data(), incompleteConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test missing required configuration - high_performance_threshold
TEST_F(TestConfigBuffer, test_load_from_buffer_returns_false_when_missing_high_performance_threshold)
{
    // Arrange
    std::string incompleteConfig =
        "monitoring_frequency=10\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{incompleteConfig.data(), incompleteConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test missing required configuration - power_save_threshold
TEST_F(TestConfigBuffer, test_load_from_buffer_returns_false_when_missing_power_save_threshold)
{
    // Arrange
    std::string incompleteConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.7\n";

    std::span<const char> buffer{incompleteConfig.data(), incompleteConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test invalid threshold cross-validation: power_save_threshold >= high_performance_threshold
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_invalid_threshold_relationship_equal)
{
    // Arrange - thresholds equal
    std::string invalidConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.5\n"
        "power_save_threshold=0.5\n";

    std::span<const char> buffer{invalidConfig.data(), invalidConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test invalid threshold cross-validation: power_save_threshold > high_performance_threshold
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_invalid_threshold_relationship_greater)
{
    // Arrange - power_save_threshold > high_performance_threshold
    std::string invalidConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.3\n"
        "power_save_threshold=0.7\n";

    std::span<const char> buffer{invalidConfig.data(), invalidConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test boundary values for thresholds
TEST_F(TestConfigBuffer, test_load_from_buffer_accepts_valid_threshold_boundaries)
{
    // Arrange - minimum valid gap between thresholds
    std::string validConfig =
        "monitoring_frequency=10\n"
        "high_performance_threshold=0.31\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{validConfig.data(), validConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(10, config->getMonitoringFrequency());
    EXPECT_EQ(0.31, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.3, config->getPowerSaveThreshold());
}

// Test invalid values - monitoring frequency
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_monitoring_frequency_below_minimum)
{
    // Arrange
    std::string invalidConfig =
        "monitoring_frequency=0\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{invalidConfig.data(), invalidConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test invalid values - monitoring frequency above maximum
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_monitoring_frequency_above_maximum)
{
    // Arrange
    std::string invalidConfig =
        "monitoring_frequency=301\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{invalidConfig.data(), invalidConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test malformed lines in buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_malformed_lines)
{
    // Arrange
    std::string malformedConfig =
        "monitoring_frequency=10\n"
        "invalid_line_without_equals\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{malformedConfig.data(), malformedConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test non-numeric values in buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_non_numeric_values)
{
    // Arrange
    std::string nonNumericConfig =
        "monitoring_frequency=abc\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{nonNumericConfig.data(), nonNumericConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test unknown configuration keys in buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_rejects_unknown_keys)
{
    // Arrange
    std::string unknownKeyConfig =
        "monitoring_frequency=10\n"
        "unknown_key=value\n"
        "high_performance_threshold=0.7\n"
        "power_save_threshold=0.3\n";

    std::span<const char> buffer{unknownKeyConfig.data(), unknownKeyConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_FALSE(result);
}

// Test buffer without null terminator
TEST_F(TestConfigBuffer, test_load_from_buffer_handles_non_null_terminated_buffer)
{
    // Arrange
    std::string configString =
        "monitoring_frequency=25\n"
        "high_performance_threshold=0.8\n"
        "power_save_threshold=0.2\n";

    // Create buffer without relying on null termination
    std::vector<char> bufferData(configString.begin(), configString.end());
    std::span<const char> buffer{bufferData.data(), bufferData.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(25, config->getMonitoringFrequency());
    EXPECT_EQ(0.8, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.2, config->getPowerSaveThreshold());
}

// Test very large buffer
TEST_F(TestConfigBuffer, test_load_from_buffer_handles_large_buffer_with_many_comments)
{
    // Arrange
    std::string largeConfig = "# Large configuration file with many comments\n";
    for (int i = 0; i < 1000; ++i) {
        largeConfig += "# Comment line " + std::to_string(i) + "\n";
    }
    largeConfig += "monitoring_frequency=60\n";
    largeConfig += "# More comments\n";
    for (int i = 0; i < 500; ++i) {
        largeConfig += "# Another comment " + std::to_string(i) + "\n";
    }
    largeConfig += "high_performance_threshold=0.75\n";
    largeConfig += "power_save_threshold=0.25\n";

    std::span<const char> buffer{largeConfig.data(), largeConfig.size()};

    // Act
    bool result = config->loadFromBuffer(buffer);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(60, config->getMonitoringFrequency());
    EXPECT_EQ(0.75, config->getHighPerformanceThreshold());
    EXPECT_EQ(0.25, config->getPowerSaveThreshold());
}
