#include "logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string Logger::m_logFile = "/var/log/ddogreen.log";
bool Logger::m_consoleOutput = false;
LogLevel Logger::m_minLevel = LogLevel::INFO;  // Default to INFO level for release builds

void Logger::init(const std::string& logFile, bool consoleOutput)
{
    m_logFile = logFile;
    m_consoleOutput = consoleOutput;

#ifdef NDEBUG
    m_minLevel = LogLevel::INFO;
#else
    m_minLevel = LogLevel::DEBUG;
#endif

    log(LogLevel::INFO, "Logger initialized");
}

void Logger::setLevel(LogLevel level)
{
    m_minLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) {
        return;
    }
    auto now = std::chrono::system_clock::now();
    auto time_val = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    
    // Use thread-safe localtime_r on POSIX systems, localtime_s on Windows
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &time_val);
#else
    localtime_r(&time_val, &timeinfo);
#endif
    
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    std::string logEntry = "[" + ss.str() + "] [" + levelToString(level) + "] " + message;

    if (!m_consoleOutput) {
        std::ofstream logFileStream(m_logFile, std::ios::app);
        if (logFileStream.is_open()) {
            logFileStream << logEntry << std::endl;
            logFileStream.close();
        } else {
            // Fallback to console if file logging fails
            std::cerr << "[LOGGER ERROR] Cannot write to log file: " << m_logFile << std::endl;
            std::cerr << logEntry << std::endl;
        }
    }

    // Write to console when console output is enabled or for errors/warnings
    if (m_consoleOutput || level == LogLevel::ERROR || level == LogLevel::WARNING) {
        if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
            std::cerr << logEntry << std::endl;
        } else {
            std::cout << logEntry << std::endl;
        }
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}
