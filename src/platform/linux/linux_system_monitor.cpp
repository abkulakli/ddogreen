#include "platform/isystem_monitor.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <memory>
#include <tuple>
#include <string>

/**
 * Linux-specific system monitor implementation
 * Uses /proc/loadavg and /proc/cpuinfo for system monitoring
 */
class LinuxSystemMonitor : public ISystemMonitor {
public:
    LinuxSystemMonitor() : m_coreCount(0), m_available(false) {
        // Initialize and cache core count
        m_coreCount = readCpuCoreCount();
        m_available = (m_coreCount > 0) && checkProcLoadavgAccess();
    }

    virtual ~LinuxSystemMonitor() = default;

    /**
     * Get system load averages from /proc/loadavg
     * @return tuple of (1-minute, 5-minute, 15-minute) load averages
     */
    std::tuple<double, double, double> getLoadAverages() override {
        std::ifstream file("/proc/loadavg");
        if (!file.is_open()) {
            Logger::error("Failed to open /proc/loadavg");
            return std::make_tuple(0.0, 0.0, 0.0);
        }

        std::string line;
        if (!std::getline(file, line)) {
            Logger::error("Failed to read from /proc/loadavg");
            return std::make_tuple(0.0, 0.0, 0.0);
        }

        // Parse the load averages
        // Format: "0.15 0.12 0.08 1/123 1234"
        std::istringstream iss(line);
        double load1min, load5min, load15min;
        
        if (!(iss >> load1min >> load5min >> load15min)) {
            Logger::error("Failed to parse load averages from /proc/loadavg");
            return std::make_tuple(0.0, 0.0, 0.0);
        }

        return std::make_tuple(load1min, load5min, load15min);
    }

    /**
     * Get CPU core count from /proc/cpuinfo
     * @return number of CPU cores
     */
    int getCpuCoreCount() override {
        return m_coreCount;
    }

    /**
     * Check if Linux system monitoring is available
     * @return true if /proc/loadavg and /proc/cpuinfo are accessible
     */
    bool isAvailable() override {
        return m_available;
    }

private:
    /**
     * Read CPU core count from /proc/cpuinfo
     * @return number of CPU cores
     */
    int readCpuCoreCount() {
        std::ifstream file("/proc/cpuinfo");
        if (!file.is_open()) {
            Logger::error("Failed to open /proc/cpuinfo");
            return 1; // Default to 1 core if we can't read
        }

        int coreCount = 0;
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.find("processor") == 0) {
                coreCount++;
            }
        }

        if (coreCount == 0) {
            Logger::warning("Could not determine CPU core count, defaulting to 1");
            coreCount = 1;
        }

        Logger::debug("Detected " + std::to_string(coreCount) + " CPU cores");
        return coreCount;
    }

    /**
     * Check if /proc/loadavg is accessible
     * @return true if we can read /proc/loadavg
     */
    bool checkProcLoadavgAccess() {
        std::ifstream file("/proc/loadavg");
        bool accessible = file.is_open();
        
        if (!accessible) {
            Logger::error("Cannot access /proc/loadavg");
        }
        
        return accessible;
    }

    int m_coreCount;
    bool m_available;
};

// Factory function for creating Linux system monitor
std::unique_ptr<ISystemMonitor> createLinuxSystemMonitor() {
    return std::make_unique<LinuxSystemMonitor>();
}
