#pragma once

#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void init(const std::string& logFile = "/var/log/ddogreen.log", bool consoleOutput = false);
    static void log(LogLevel level, const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

private:
    static std::string levelToString(LogLevel level);
    static std::string m_logFile;
    static bool m_consoleOutput;
};
