#pragma once

#include <string>
#include <vector>

/**
 * Structure to hold parsed command line arguments
 */
struct ParsedArgs {
    bool showHelp = false;
    bool showVersion = false;
    bool hasUnknownOptions = false;
    std::string unknownOption;
    std::string configPath;
};

/**
 * Interface for platform-specific utility functions
 * Handles privilege checking, executable path detection, and command line parsing
 */
class IPlatformUtils {
public:
    virtual ~IPlatformUtils() = default;

    /**
     * Check if running with administrator/root privileges
     * @return true if running with required privileges
     */
    virtual bool hasRequiredPrivileges() const = 0;

    /**
     * Get the full path to the current executable
     * @return full path to executable, or empty string if failed
     */
    virtual std::string getExecutablePath() const = 0;

    /**
     * Parse command line arguments in a platform-appropriate way
     * @param argc argument count
     * @param argv argument vector
     * @return parsed arguments structure
     */
    virtual ParsedArgs parseCommandLine(int argc, char* argv[]) const = 0;

    /**
     * Get the default installation path for the executable
     * @return platform-specific installation path
     */
    virtual std::string getDefaultInstallPath() const = 0;

    /**
     * Get the default log file path
     * @return platform-specific log file path
     */
    virtual std::string getDefaultLogPath() const = 0;

    /**
     * Get the default PID file path
     * @return platform-specific PID file path
     */
    virtual std::string getDefaultPidPath() const = 0;

    /**
     * Get the default configuration file path
     * @return platform-specific configuration file path
     */
    virtual std::string getDefaultConfigPath() const = 0;

    /**
     * Check if platform utilities are available
     * @return true if platform utilities are functional
     */
    virtual bool isAvailable() const = 0;

    /**
     * Get the privilege escalation message for the current platform
     * @return message explaining how to run with required privileges
     */
    virtual std::string getPrivilegeEscalationMessage() const = 0;

    /**
     * Resolve a relative path to an absolute path in a platform-specific way
     * @param relativePath the relative path to resolve
     * @return absolute path, or original path if resolution fails
     */
    virtual std::string resolveAbsolutePath(const std::string& relativePath) const = 0;
};
