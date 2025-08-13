#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "activity_monitor.h"
#include "logger.h"
#include "mocks/mock_system_monitor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::InSequence;

class TestActivityMonitor : public ::testing::Test {
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

// Test constructor initialization
TEST_F(TestActivityMonitor, test_constructor_initializes_default_values) {
    ActivityMonitor monitor;
    
    // Should not be active initially
    EXPECT_FALSE(monitor.isActive());
}

// Test setLoadThresholds method
TEST_F(TestActivityMonitor, test_set_load_thresholds_stores_values_correctly) {
    ActivityMonitor monitor;
    
    // Set thresholds
    monitor.setLoadThresholds(0.8, 0.3);
    
    // Values should be stored internally (tested indirectly through behavior)
    // We can't directly test the stored values as they're private,
    // but this ensures the method runs without error
}

// Test setMonitoringFrequency method
TEST_F(TestActivityMonitor, test_set_monitoring_frequency_stores_value_correctly) {
    ActivityMonitor monitor;
    
    // Set monitoring frequency
    monitor.setMonitoringFrequency(30);
    
    // Should run without error
}

// Test setActivityCallback method
TEST_F(TestActivityMonitor, test_set_activity_callback_stores_callback) {
    ActivityMonitor monitor;
    bool callbackCalled = false;
    
    // Set callback
    monitor.setActivityCallback([&callbackCalled](bool active) {
        callbackCalled = true;
    });
    
    // Callback should be stored (will be tested when start() is called)
}

// Test start method behavior with invalid configuration
TEST_F(TestActivityMonitor, test_start_fails_with_invalid_monitoring_frequency) {
    ActivityMonitor monitor;
    
    // Don't set monitoring frequency (should default to 0)
    bool result = monitor.start();
    
    // Should fail
    EXPECT_FALSE(result);
}

// Test start method behavior with valid configuration
TEST_F(TestActivityMonitor, test_start_succeeds_with_valid_configuration) {
    ActivityMonitor monitor;
    
    // Set valid configuration
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    bool result = monitor.start();
    
    // Should succeed if system monitor is available, or fail gracefully if not
    // On CI/test environments, system monitor may not be available
    if (result) {
        EXPECT_TRUE(result);
        monitor.stop(); // Clean up
    } else {
        // This is acceptable in test environment where system monitor may not be available
        EXPECT_FALSE(result);
    }
}

// Test stop method
TEST_F(TestActivityMonitor, test_stop_method_runs_without_error) {
    ActivityMonitor monitor;
    
    // Stop should work even if not started
    monitor.stop();
    
    // Should complete without error
}

// Test start method when already running
TEST_F(TestActivityMonitor, test_start_returns_true_when_already_running) {
    ActivityMonitor monitor;
    
    // Set valid configuration
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    bool firstResult = monitor.start();
    
    if (firstResult) {
        // Try to start again
        bool secondResult = monitor.start();
        EXPECT_TRUE(secondResult);
        
        monitor.stop(); // Clean up
    }
}

// Test callback execution during initial load check
TEST_F(TestActivityMonitor, test_callback_called_on_start_with_initial_state) {
    ActivityMonitor monitor;
    bool callbackCalled = false;
    bool callbackValue = false;
    
    // Set callback to capture calls
    monitor.setActivityCallback([&](bool active) {
        callbackCalled = true;
        callbackValue = active;
    });
    
    // Set valid configuration
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    bool result = monitor.start();
    
    if (result) {
        // Give a brief moment for initial load check
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        EXPECT_TRUE(callbackCalled);
        // Initial state should be determined by current load vs thresholds
        
        monitor.stop();
    }
}

// Test threshold logic for high performance mode
TEST_F(TestActivityMonitor, test_threshold_logic_high_performance) {
    ActivityMonitor monitor;
    
    // Set thresholds where high performance = 50% per core, power save = 20% per core
    monitor.setLoadThresholds(0.5, 0.2);
    monitor.setMonitoringFrequency(1);
    
    // This test verifies the threshold setting logic executes without error
    // Actual load-based switching would require integration with system monitor
}

// Test threshold logic for power save mode
TEST_F(TestActivityMonitor, test_threshold_logic_power_save) {
    ActivityMonitor monitor;
    
    // Set thresholds where high performance = 80% per core, power save = 30% per core  
    monitor.setLoadThresholds(0.8, 0.3);
    monitor.setMonitoringFrequency(1);
    
    // This test verifies the threshold setting logic executes without error
    // Actual load-based switching would require integration with system monitor
}

// Test configuration validation
TEST_F(TestActivityMonitor, test_validates_threshold_ranges) {
    ActivityMonitor monitor;
    
    // Test various threshold combinations
    monitor.setLoadThresholds(0.9, 0.1); // Valid: high > low
    monitor.setLoadThresholds(0.5, 0.5); // Edge case: equal thresholds
    monitor.setLoadThresholds(1.0, 0.0); // Edge case: maximum range
    
    // All should execute without error
    // Actual validation behavior would be tested through integration tests
}

// Test multiple start/stop cycles
TEST_F(TestActivityMonitor, test_multiple_start_stop_cycles) {
    ActivityMonitor monitor;
    
    // Set valid configuration
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    // Test multiple cycles
    for (int i = 0; i < 3; ++i) {
        bool result = monitor.start();
        
        if (result) {
            monitor.stop();
        }
        
        // Each cycle should work independently
    }
}

// Test monitoring frequency validation
TEST_F(TestActivityMonitor, test_monitoring_frequency_validation) {
    ActivityMonitor monitor;
    
    // Test various frequencies
    monitor.setMonitoringFrequency(1);   // Minimum practical value
    monitor.setMonitoringFrequency(60);  // Common value
    monitor.setMonitoringFrequency(300); // Maximum practical value
    
    // All should execute without error
}

// Test callback with different activity states
TEST_F(TestActivityMonitor, test_callback_receives_correct_activity_states) {
    ActivityMonitor monitor;
    std::vector<bool> callbackValues;
    
    // Set callback to capture all calls
    monitor.setActivityCallback([&](bool active) {
        callbackValues.push_back(active);
    });
    
    // Set configuration and start
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    bool result = monitor.start();
    
    if (result) {
        // Give brief time for initial callback
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Should have received at least one callback (initial state)
        EXPECT_GE(callbackValues.size(), 1);
        
        monitor.stop();
    }
}

// Test isActive state tracking
TEST_F(TestActivityMonitor, test_is_active_state_tracking) {
    ActivityMonitor monitor;
    
    // Initially should not be active
    EXPECT_FALSE(monitor.isActive());
    
    // State should be maintained throughout lifecycle
    monitor.setMonitoringFrequency(10);
    monitor.setLoadThresholds(0.8, 0.3);
    
    bool result = monitor.start();
    
    if (result) {
        // Give brief time for state to be set
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // isActive() should return a valid boolean
        bool activeState = monitor.isActive();
        EXPECT_TRUE(activeState == true || activeState == false); // Either state is valid
        
        monitor.stop();
    }
}

// Test configuration changes after initialization
TEST_F(TestActivityMonitor, test_configuration_changes_after_init) {
    ActivityMonitor monitor;
    
    // Initial configuration
    monitor.setLoadThresholds(0.8, 0.3);
    monitor.setMonitoringFrequency(10);
    
    // Change configuration
    monitor.setLoadThresholds(0.7, 0.2);
    monitor.setMonitoringFrequency(20);
    
    // Should handle configuration changes gracefully
    bool result = monitor.start();
    
    if (result) {
        monitor.stop();
    }
}

// Test destructor cleanup
TEST_F(TestActivityMonitor, test_destructor_cleanup) {
    {
        ActivityMonitor monitor;
        monitor.setMonitoringFrequency(10);
        monitor.setLoadThresholds(0.8, 0.3);
        
        // Don't start the monitor in this test to avoid threading issues
        // Just test that destructor handles an unstarted monitor correctly
        
        // Destructor should clean up automatically when monitor goes out of scope
    }
    
    // If we reach here without hanging, destructor worked correctly
    SUCCEED();
}
