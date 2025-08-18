#include "rate_limiter.h"
#include "logger.h"

RateLimiter::RateLimiter(int maxRequests, int timeWindowMs) 
    : maxRequests_(maxRequests), timeWindowMs_(timeWindowMs) {
}

bool RateLimiter::isAllowed(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    
    // Clean up old entries periodically
    cleanupOldEntries();
    
    auto it = requestMap_.find(key);
    if (it == requestMap_.end()) {
        // First request for this key
        requestMap_[key] = {now, 1, now};
        return true;
    }
    
    RequestInfo& info = it->second;
    
    // Check if we're in a new time window
    auto windowDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - info.windowStart).count();
    
    if (windowDuration >= timeWindowMs_) {
        // New time window - reset counter
        info.windowStart = now;
        info.requestCount = 1;
        info.lastRequest = now;
        return true;
    }
    
    // We're in the same time window - check if we exceed the limit
    if (info.requestCount >= maxRequests_) {
        Logger::warning("Rate limiting triggered for operation: " + key + 
            " (max " + std::to_string(maxRequests_) + " requests per " + 
            std::to_string(timeWindowMs_) + "ms)");
        return false;
    }
    
    // Request is allowed
    info.requestCount++;
    info.lastRequest = now;
    return true;
}

void RateLimiter::reset(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    requestMap_.erase(key);
}

void RateLimiter::resetAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    requestMap_.clear();
}

void RateLimiter::cleanupOldEntries() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = requestMap_.begin(); it != requestMap_.end();) {
        auto timeSinceLastRequest = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.lastRequest).count();
        
        // Remove entries that haven't been used for more than 10 time windows
        if (timeSinceLastRequest > timeWindowMs_ * 10) {
            it = requestMap_.erase(it);
        } else {
            ++it;
        }
    }
}
