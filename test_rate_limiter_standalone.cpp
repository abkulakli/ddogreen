#include <iostream>
#include <chrono>
#include <thread>
#include "rate_limiter.h"
#include "logger.h"

int main() {
    Logger::init("/tmp/test_rate_limiter.log", true);
    
    RateLimiter limiter(3, 1000);  // 3 requests per 1000ms
    
    std::cout << "Testing rate limiter (3 requests per 1000ms):" << std::endl;
    
    for (int i = 0; i < 10; ++i) {
        bool allowed = limiter.isAllowed("test_operation");
        std::cout << "Request " << (i+1) << ": " << (allowed ? "ALLOWED" : "BLOCKED") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Waiting 1.5 seconds for rate limit window to reset..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    std::cout << "Testing after reset:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        bool allowed = limiter.isAllowed("test_operation");
        std::cout << "Request " << (i+1) << ": " << (allowed ? "ALLOWED" : "BLOCKED") << std::endl;
    }
    
    return 0;
}
