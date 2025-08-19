#ifndef DDOGREEN_RATE_LIMITER_H
#define DDOGREEN_RATE_LIMITER_H

#include <chrono>
#include <unordered_map>
#include <string>
#include <mutex>

/**
 * @brief Rate limiter for preventing abuse through rapid operations
 * 
 * This class implements a token bucket algorithm to limit the rate
 * of operations and prevent denial of service attacks.
 */
class RateLimiter {
public:
    /**
     * @brief Construct a new Rate Limiter
     * 
     * @param maxRequests Maximum number of requests allowed in the time window
     * @param timeWindowMs Time window in milliseconds
     */
    explicit RateLimiter(int maxRequests = 5, int timeWindowMs = 1000);
    
    /**
     * @brief Check if an operation is allowed for the given key
     * 
     * @param key Identifier for the operation (e.g., "power_mode_change")
     * @return true if the operation is allowed, false if rate limited
     */
    bool isAllowed(const std::string& key);
    
    /**
     * @brief Reset the rate limiter for a specific key
     * 
     * @param key The key to reset
     */
    void reset(const std::string& key);
    
    /**
     * @brief Reset all rate limiting data
     */
    void resetAll();

private:
    struct RequestInfo {
        std::chrono::steady_clock::time_point lastRequest;
        int requestCount;
        std::chrono::steady_clock::time_point windowStart;
    };
    
    int maxRequests_;
    int timeWindowMs_;
    std::unordered_map<std::string, RequestInfo> requestMap_;
    std::mutex mutex_;
    
    void cleanupOldEntries();
};

#endif // DDOGREEN_RATE_LIMITER_H
