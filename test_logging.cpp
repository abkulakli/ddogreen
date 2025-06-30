#include "include/logger.h"
#include <thread>
#include <chrono>

int main() {
    Logger::init("/tmp/ddotlp_test.log");

    Logger::info("Testing improved timestamp format");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::info("This log entry shows milliseconds precision");
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    Logger::info("TLP output: TLP started in AC mode (auto).");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Logger::info("TLP output: TLP started in battery mode (manual).");

    return 0;
}
