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
            {"daemon",      no_argument, 0, 'd'},
            {"help",        no_argument, 0, 'h'},
            {"version",     no_argument, 0, 'v'},
            {"install",     no_argument, 0, 'i'},
            {"uninstall",   no_argument, 0, 'u'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c;
        
        // Reset getopt state
        optind = 1;

        while ((c = getopt_long(argc, argv, "dhviu", long_options, &option_index)) != -1) {
            switch (c) {
                case 'd':
                    args.runAsDaemon = true;
                    break;
                case 'h':
                    args.showHelp = true;
                    break;
                case 'v':
                    args.showVersion = true;
                    break;
                case 'i':
                    args.install = true;
                    break;
                case 'u':
                    args.uninstall = true;
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
};

// Factory function to create macOS platform utilities
std::unique_ptr<IPlatformUtils> createMacOSPlatformUtils() {
    return std::make_unique<MacOSPlatformUtils>();
}
