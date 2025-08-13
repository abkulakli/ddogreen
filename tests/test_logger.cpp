#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <algorithm>
#include "logger.h"

namespace fs = std::filesystem;

class TestLogger : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test log files
        testDir = fs::temp_directory_path() / "ddogreen_logger_test";
        fs::create_directories(testDir);
        
        // Store original cout/cerr buffers
        originalCoutBuffer = std::cout.rdbuf();
        originalCerrBuffer = std::cerr.rdbuf();
        
        // Redirect cout and cerr to our stringstreams for testing
        std::cout.rdbuf(coutCapture.rdbuf());
        std::cerr.rdbuf(cerrCapture.rdbuf());
    }

    void TearDown() override {
        // Restore original cout/cerr buffers
        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);
        
        // Clean up test files
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    std::string getTestLogPath(const std::string& filename) {
        return (testDir / filename).string();
    }

    std::string readLogFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void clearCaptureBuffers() {
        coutCapture.str("");
        coutCapture.clear();
        cerrCapture.str("");
        cerrCapture.clear();
    }

    fs::path testDir;
    std::stringstream coutCapture;
    std::stringstream cerrCapture;
    std::streambuf* originalCoutBuffer;
    std::streambuf* originalCerrBuffer;
};

// Test logger initialization with file output
TEST_F(TestLogger, test_init_with_file_output) {
    std::string logPath = getTestLogPath("test.log");
    
    // Initialize logger with file output, no console
    Logger::init(logPath, false);
    
    // Log a test message
    Logger::info("Test initialization message");
    
    // Check that log file was created and contains the message
    std::string logContent = readLogFile(logPath);
    EXPECT_FALSE(logContent.empty());
    EXPECT_TRUE(logContent.find("Test initialization message") != std::string::npos);
    EXPECT_TRUE(logContent.find("[INFO]") != std::string::npos);
}

// Test logger initialization with console output
TEST_F(TestLogger, test_init_with_console_output) {
    std::string logPath = getTestLogPath("console_test.log");
    
    // Initialize logger with console output enabled
    Logger::init(logPath, true);
    
    clearCaptureBuffers();
    
    // Log a test message
    Logger::info("Test console message");
    
    // Check that message appears in console output
    std::string consoleOutput = coutCapture.str();
    EXPECT_TRUE(consoleOutput.find("Test console message") != std::string::npos);
    EXPECT_TRUE(consoleOutput.find("[INFO]") != std::string::npos);
}

// Test different log levels
TEST_F(TestLogger, test_log_levels) {
    std::string logPath = getTestLogPath("levels.log");
    Logger::init(logPath, false);
    Logger::setLevel(LogLevel::DEBUG);  // Ensure debug messages are logged
    
    // Test all log levels
    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warning("Warning message");
    Logger::error("Error message");
    
    std::string logContent = readLogFile(logPath);
    
    // All messages should be in the log
    EXPECT_TRUE(logContent.find("Debug message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Info message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Warning message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Error message") != std::string::npos);
    
    // Check level tags
    EXPECT_TRUE(logContent.find("[DEBUG]") != std::string::npos);
    EXPECT_TRUE(logContent.find("[INFO]") != std::string::npos);
    EXPECT_TRUE(logContent.find("[WARNING]") != std::string::npos);
    EXPECT_TRUE(logContent.find("[ERROR]") != std::string::npos);
}

// Test direct log method with specific levels
TEST_F(TestLogger, test_direct_log_method) {
    std::string logPath = getTestLogPath("direct.log");
    Logger::init(logPath, false);
    Logger::setLevel(LogLevel::DEBUG);  // Ensure debug messages are logged
    
    // Use direct log method
    Logger::log(LogLevel::DEBUG, "Direct debug");
    Logger::log(LogLevel::INFO, "Direct info");
    Logger::log(LogLevel::WARNING, "Direct warning");
    Logger::log(LogLevel::ERROR, "Direct error");
    
    std::string logContent = readLogFile(logPath);
    
    EXPECT_TRUE(logContent.find("Direct debug") != std::string::npos);
    EXPECT_TRUE(logContent.find("Direct info") != std::string::npos);
    EXPECT_TRUE(logContent.find("Direct warning") != std::string::npos);
    EXPECT_TRUE(logContent.find("Direct error") != std::string::npos);
}
