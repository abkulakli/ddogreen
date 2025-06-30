#pragma once

namespace Config {
    // Default idle timeout in seconds (5 minutes)
    constexpr int DEFAULT_IDLE_TIMEOUT = 300;

    // Log file path
    constexpr const char* LOG_FILE = "/var/log/ddotlp.log";

    // PID file path
    constexpr const char* PID_FILE = "/var/run/ddotlp.pid";

    // Config file path
    constexpr const char* CONFIG_FILE = "/etc/ddotlp/ddotlp.conf";

    // TLP commands
    constexpr const char* TLP_START_COMMAND = "tlp start";
    constexpr const char* TLP_BAT_COMMAND = "tlp bat";
    constexpr const char* TLP_STATUS_COMMAND = "tlp-stat -s";
}
