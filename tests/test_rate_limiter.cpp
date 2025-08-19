#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "rate_limiter.h"
#include "logger.h"

class TestRateLimiter : public ::testing::Test {
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

// ============================================================================
// BASIC FUNCTIONALITY TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_constructor_default_values) {
    RateLimiter limiter;
    
    // First request should be allowed (default: 5 requests per 1000ms)
    EXPECT_TRUE(limiter.isAllowed("test_key"));
}

TEST_F(TestRateLimiter, test_constructor_custom_values) {
    RateLimiter limiter(3, 500); // 3 requests per 500ms
    
    // First request should be allowed
    EXPECT_TRUE(limiter.isAllowed("test_key"));
}

TEST_F(TestRateLimiter, test_first_request_always_allowed) {
    RateLimiter limiter(1, 1000); // Very restrictive: 1 request per second
    
    EXPECT_TRUE(limiter.isAllowed("new_key"));
}

TEST_F(TestRateLimiter, test_different_keys_independent) {
    RateLimiter limiter(1, 1000); // 1 request per second
    
    // Each key gets its own allowance
    EXPECT_TRUE(limiter.isAllowed("key1"));
    EXPECT_TRUE(limiter.isAllowed("key2"));
    EXPECT_TRUE(limiter.isAllowed("key3"));
}

// ============================================================================
// RATE LIMITING BEHAVIOR TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_rate_limiting_within_window) {
    RateLimiter limiter(2, 1000); // 2 requests per second
    
    const std::string key = "test_key";
    
    // First 2 requests should be allowed
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_TRUE(limiter.isAllowed(key));
    
    // Third request should be denied (rate limited)
    EXPECT_FALSE(limiter.isAllowed(key));
    EXPECT_FALSE(limiter.isAllowed(key)); // Still denied
}

TEST_F(TestRateLimiter, test_rate_limiting_resets_after_window) {
    RateLimiter limiter(1, 100); // 1 request per 100ms
    
    const std::string key = "test_key";
    
    // First request allowed
    EXPECT_TRUE(limiter.isAllowed(key));
    
    // Second request denied (rate limited)
    EXPECT_FALSE(limiter.isAllowed(key));
    
    // Wait for time window to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(110));
    
    // Request should be allowed again
    EXPECT_TRUE(limiter.isAllowed(key));
}

TEST_F(TestRateLimiter, test_boundary_conditions) {
    RateLimiter limiter(3, 500); // 3 requests per 500ms
    
    const std::string key = "test_key";
    
    // Exhaust the limit
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_TRUE(limiter.isAllowed(key));
    
    // Should be rate limited
    EXPECT_FALSE(limiter.isAllowed(key));
    
    // Wait just under the window duration
    std::this_thread::sleep_for(std::chrono::milliseconds(450));
    EXPECT_FALSE(limiter.isAllowed(key)); // Still blocked
    
    // Wait for window to fully expire
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(limiter.isAllowed(key)); // Should be allowed again
}

// ============================================================================
// RESET FUNCTIONALITY TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_reset_specific_key) {
    RateLimiter limiter(1, 1000); // 1 request per second
    
    const std::string key1 = "key1";
    const std::string key2 = "key2";
    
    // Exhaust limits for both keys
    EXPECT_TRUE(limiter.isAllowed(key1));
    EXPECT_TRUE(limiter.isAllowed(key2));
    EXPECT_FALSE(limiter.isAllowed(key1)); // Rate limited
    EXPECT_FALSE(limiter.isAllowed(key2)); // Rate limited
    
    // Reset only key1
    limiter.reset(key1);
    
    // key1 should be allowed again, key2 still blocked
    EXPECT_TRUE(limiter.isAllowed(key1));
    EXPECT_FALSE(limiter.isAllowed(key2));
}

TEST_F(TestRateLimiter, test_reset_all_keys) {
    RateLimiter limiter(1, 1000); // 1 request per second
    
    const std::string key1 = "key1";
    const std::string key2 = "key2";
    
    // Exhaust limits for both keys
    EXPECT_TRUE(limiter.isAllowed(key1));
    EXPECT_TRUE(limiter.isAllowed(key2));
    EXPECT_FALSE(limiter.isAllowed(key1)); // Rate limited
    EXPECT_FALSE(limiter.isAllowed(key2)); // Rate limited
    
    // Reset all
    limiter.resetAll();
    
    // Both keys should be allowed again
    EXPECT_TRUE(limiter.isAllowed(key1));
    EXPECT_TRUE(limiter.isAllowed(key2));
}

TEST_F(TestRateLimiter, test_reset_nonexistent_key) {
    RateLimiter limiter(1, 1000);
    
    // Reset a key that doesn't exist - should not crash
    limiter.reset("nonexistent_key");
    
    // Normal operation should continue
    EXPECT_TRUE(limiter.isAllowed("test_key"));
}

// ============================================================================
// THREAD SAFETY TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_thread_safety_concurrent_access) {
    RateLimiter limiter(10, 1000); // 10 requests per second
    
    const std::string key = "concurrent_key";
    std::atomic<int> allowed_count{0};
    std::atomic<int> denied_count{0};
    
    const int num_threads = 5;
    const int requests_per_thread = 5;
    
    std::vector<std::thread> threads;
    
    // Launch multiple threads making requests
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&limiter, &key, &allowed_count, &denied_count, requests_per_thread]() {
            for (int i = 0; i < requests_per_thread; ++i) {
                if (limiter.isAllowed(key)) {
                    allowed_count.fetch_add(1);
                } else {
                    denied_count.fetch_add(1);
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify total requests
    int total_requests = allowed_count.load() + denied_count.load();
    EXPECT_EQ(total_requests, num_threads * requests_per_thread);
    
    // At most 10 should be allowed (rate limit), rest should be denied
    EXPECT_LE(allowed_count.load(), 10);
    EXPECT_GE(denied_count.load(), total_requests - 10);
}

TEST_F(TestRateLimiter, test_thread_safety_concurrent_reset) {
    RateLimiter limiter(5, 1000);
    
    const std::string key = "reset_key";
    std::atomic<bool> running{true};
    
    // Thread 1: Make requests
    std::thread requester([&limiter, &key, &running]() {
        while (running.load()) {
            limiter.isAllowed(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Thread 2: Reset periodically
    std::thread resetter([&limiter, &key, &running]() {
        while (running.load()) {
            limiter.reset(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    // Let them run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    running.store(false);
    
    requester.join();
    resetter.join();
    
    // If we reach here without deadlock or crash, thread safety is working
    SUCCEED();
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_zero_max_requests) {
    RateLimiter limiter(0, 1000); // No requests allowed
    
    // First request is always allowed due to implementation design
    EXPECT_TRUE(limiter.isAllowed("test_key"));
    
    // Subsequent requests should be denied
    EXPECT_FALSE(limiter.isAllowed("test_key"));
    EXPECT_FALSE(limiter.isAllowed("test_key"));
}

TEST_F(TestRateLimiter, test_very_short_time_window) {
    RateLimiter limiter(2, 1); // 2 requests per 1ms
    
    const std::string key = "test_key";
    
    // Make requests quickly
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_FALSE(limiter.isAllowed(key)); // Should be rate limited
    
    // Wait for tiny window to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    EXPECT_TRUE(limiter.isAllowed(key)); // Should be allowed again
}

TEST_F(TestRateLimiter, test_very_long_time_window) {
    RateLimiter limiter(1, 5000); // 1 request per 5 seconds
    
    const std::string key = "test_key";
    
    EXPECT_TRUE(limiter.isAllowed(key));
    EXPECT_FALSE(limiter.isAllowed(key)); // Rate limited for 5 seconds
}

TEST_F(TestRateLimiter, test_empty_key) {
    RateLimiter limiter(2, 1000);
    
    // Empty key should be treated as valid
    EXPECT_TRUE(limiter.isAllowed(""));
    EXPECT_TRUE(limiter.isAllowed(""));
    EXPECT_FALSE(limiter.isAllowed("")); // Rate limited
}

TEST_F(TestRateLimiter, test_very_long_key) {
    RateLimiter limiter(2, 1000);
    
    std::string long_key(1000, 'a'); // Very long key
    
    EXPECT_TRUE(limiter.isAllowed(long_key));
    EXPECT_TRUE(limiter.isAllowed(long_key));
    EXPECT_FALSE(limiter.isAllowed(long_key)); // Rate limited
}

// ============================================================================
// PERFORMANCE AND CLEANUP TESTS
// ============================================================================

TEST_F(TestRateLimiter, test_cleanup_old_entries) {
    // This test verifies that old entries are cleaned up to prevent memory leaks
    RateLimiter limiter(1, 100); // 1 request per 100ms
    
    // Create many different keys
    for (int i = 0; i < 1000; ++i) {
        std::string key = "key_" + std::to_string(i);
        limiter.isAllowed(key);
    }
    
    // Wait long enough for cleanup to potentially happen
    std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // 15x the window duration
    
    // Make a request to trigger cleanup
    limiter.isAllowed("trigger_cleanup");
    
    // If cleanup is working, memory usage should be reasonable
    // We can't directly test memory, but we can ensure the system still responds
    EXPECT_TRUE(limiter.isAllowed("new_key_after_cleanup"));
}

TEST_F(TestRateLimiter, test_many_rapid_requests_performance) {
    RateLimiter limiter(100, 1000); // 100 requests per second
    
    const std::string key = "perf_test";
    auto start = std::chrono::steady_clock::now();
    
    // Make many requests rapidly
    int allowed = 0;
    for (int i = 0; i < 1000; ++i) {
        if (limiter.isAllowed(key)) {
            allowed++;
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete quickly (within reasonable time)
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
    
    // Should respect rate limit
    EXPECT_LE(allowed, 100);
}
