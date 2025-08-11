#include "platform/iplatform_utils.h"
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <string>
#include <iostream>
#include <memory>
#include <mach-o/dyld.h>

/**
 * macOS-specific platform utilities implementation
 */
class MacOSPlatformUtils : public IPlatformUtils {
public:
    MacOSPlatformUtils() = default;
    virtual ~MacOSPlatformUtils() = default;

    /**
     * Check if running with root privileges
     * @return true if running as root
     */
    bool hasRequiredPrivileges() const override {
        return geteuid() == 0;
    }

    /**
     * Get the full path to the current executable on macOS
     * @return full path to executable, or empty string if failed
     */
    std::string getExecutablePath() const override {
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            return std::string(path);
        }
        return "";
    }

    /**
     * Parse command line arguments using getopt_long
     * @param argc argument count
     * @param argv argument vector
     * @return parsed arguments structure
     */
    ParsedArgs parseCommandLine(int argc, char* argv[]) const override {
        ParsedArgs args;
        
        static struct option long_options[] = {
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'v'},
            {"config",      required_argument, 0, 'c'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c;
        
        // Reset getopt state
        optind = 1;

        while ((c = getopt_long(argc, argv, "hvc:", long_options, &option_index)) != -1) {
            switch (c) {
                case 'h':
                    args.showHelp = true;
                    break;
                case 'v':
                    args.showVersion = true;
                    break;
                case 'c':
                    args.configPath = optarg;
                    break;
                case '?':
                    args.hasUnknownOptions = true;
                    break;
                default:
                    break;
            }
        }
        
        return args;
    }

    /**
     * Get the default installation path for macOS
     * @return /usr/local/bin/ddogreen
     */
    std::string getDefaultInstallPath() const override {
        return "/usr/local/bin/ddogreen";
    }

    /**
     * Get the default log file path for macOS
     * @return /var/log/ddogreen.log
     */
    std::string getDefaultLogPath() const override {
        return "/var/log/ddogreen.log";
    }

    /**
     * Get the default PID file path for macOS
     * @return /var/run/ddogreen.pid
     */
    std::string getDefaultPidPath() const override {
        return "/var/run/ddogreen.pid";
    }

    /**
     * Get the default configuration file path for macOS
     * @return /etc/ddogreen/ddogreen.conf
     */
    std::string getDefaultConfigPath() const override {
        return "/etc/ddogreen/ddogreen.conf";
    }

    /**
     * Check if platform utilities are available
     * @return true (always available on macOS)
     */
    bool isAvailable() const override {
        return true;
    }

    /**
     * Get the privilege escalation message for macOS
     * @return message explaining to use sudo
     */
    std::string getPrivilegeEscalationMessage() const override {
        return "This program requires root privileges. Please run with sudo.";
    }

    /**
     * Resolve a relative path to an absolute path using realpath
     * @param relativePath the relative path to resolve
     * @return absolute path, or original path if resolution fails
     */
    std::string resolveAbsolutePath(const std::string& relativePath) const override {
        // If already absolute, return as-is
        if (!relativePath.empty() && relativePath[0] == '/') {
            return relativePath;
        }

        // Get current working directory
        char* cwd = getcwd(nullptr, 0);
        if (!cwd) {
            return relativePath; // Fallback to original path
        }

        std::string tempPath = std::string(cwd) + "/" + relativePath;
        free(cwd);

        // Use realpath to resolve all . and .. components and get canonical path
        char* resolvedPath = realpath(tempPath.c_str(), nullptr);
        if (resolvedPath) {
            std::string result(resolvedPath);
            free(resolvedPath);
            return result;
        }

        // If realpath fails, return the simple concatenation as fallback
        return tempPath;
    }
};

// Factory function to create macOS platform utilities
std::unique_ptr<IPlatformUtils> createMacOSPlatformUtils() {
    return std::make_unique<MacOSPlatformUtils>();
}
