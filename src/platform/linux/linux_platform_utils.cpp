#include "platform/iplatform_utils.h"
#include "security_utils.h"
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <linux/limits.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <filesystem>

/**
 * Linux-specific platform utilities implementation
 */
class LinuxPlatformUtils : public IPlatformUtils
{
public:
    LinuxPlatformUtils() = default;
    virtual ~LinuxPlatformUtils() override = default;

    /**
     * Check if running with root privileges
     * @return true if running as root
     */
    bool hasRequiredPrivileges() const override
    {
        return geteuid() == 0;
    }

    /**
     * Get the full path to the current executable on Linux
     * @return full path to executable, or empty string if failed
     */
    std::string getExecutablePath() const override
    {
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count != -1)
        {
            path[count] = '\0';
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
    ParsedArgs parseCommandLine(int argc, char* argv[]) const override
    {
        ParsedArgs args;

        static struct option long_options[] =
        {
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'v'},
            {"config",      required_argument, 0, 'c'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c;

        optind = 1;

        while ((c = getopt_long(argc, argv, "hvc:", long_options, &option_index)) != -1)
        {
            switch (c)
            {
                case 'h':
                    args.showHelp = true;
                    break;
                case 'v':
                    args.showVersion = true;
                    break;
                case 'c':
                    // Security validation: Check config path for traversal
                    if (optarg && SecurityUtils::validatePathTraversal(optarg))
                    {
                        args.configPath = optarg;
                    }
                    else
                    {
                        args.hasUnknownOptions = true;
                        args.unknownOption = std::string("Invalid config path: ") + (optarg ? optarg : "null");
                    }
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
     * Get the default installation path for Linux
     * @return /usr/local/bin/ddogreen
     */
    std::string getDefaultInstallPath() const override
    {
        return "/usr/local/bin/ddogreen";
    }

    /**
     * Get the default log file path for Linux
     * @return /var/log/ddogreen.log
     */
    std::string getDefaultLogPath() const override
    {
        return "/var/log/ddogreen.log";
    }

    /**
     * Get the default PID file path for Linux
     * @return /run/ddogreen.pid
     */
    std::string getDefaultPidPath() const override
    {
        return "/run/ddogreen.pid";
    }

    /**
     * Get the default configuration file path for Linux
     * @return /etc/ddogreen/ddogreen.conf
     */
    std::string getDefaultConfigPath() const override
    {
        return "/etc/ddogreen/ddogreen.conf";
    }

    /**
     * Check if platform utilities are available
     * @return true (always available on Linux)
     */
    bool isAvailable() const override
    {
        return true;
    }

    /**
     * Get the privilege escalation message for Linux
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
    std::string resolveAbsolutePath(const std::string& relativePath) const override
    {
        if (!relativePath.empty() && relativePath[0] == '/')
        {
            return relativePath;
        }

        char* cwd = getcwd(nullptr, 0);
        if (!cwd)
        {
            return relativePath;
        }

        std::string tempPath = std::string(cwd) + "/" + relativePath;
        free(cwd);

        char* resolvedPath = realpath(tempPath.c_str(), nullptr);
        if (resolvedPath)
        {
            std::string result(resolvedPath);
            free(resolvedPath);
            return result;
        }

        return tempPath;
    }

    /**
     * Get the current power source by checking Linux power supply subsystem
     * @return PowerSource enumeration value
     */
    PowerSource getPowerSource() const override
    {
        try
        {
            // Check /sys/class/power_supply for AC adapters and batteries
            const std::string powerSupplyPath = "/sys/class/power_supply";
            
            if (!std::filesystem::exists(powerSupplyPath))
            {
                return PowerSource::UNKNOWN;
            }

            bool hasAC = false;
            bool hasBattery = false;
            bool acOnline = false;

            // Iterate through power supply devices
            for (const auto& entry : std::filesystem::directory_iterator(powerSupplyPath))
            {
                if (!entry.is_directory()) continue;

                std::string devicePath = entry.path().string();
                std::string typePath = devicePath + "/type";

                // Read device type
                std::ifstream typeFile(typePath);
                if (typeFile.is_open())
                {
                    std::string deviceType;
                    std::getline(typeFile, deviceType);
                    typeFile.close();

                    if (deviceType == "Mains" || deviceType == "ADP1")
                    {
                        hasAC = true;
                        // Check if AC is online
                        std::string onlinePath = devicePath + "/online";
                        std::ifstream onlineFile(onlinePath);
                        if (onlineFile.is_open())
                        {
                            std::string online;
                            std::getline(onlineFile, online);
                            onlineFile.close();
                            if (online == "1")
                            {
                                acOnline = true;
                            }
                        }
                    }
                    else if (deviceType == "Battery")
                    {
                        hasBattery = true;
                    }
                }
            }

            // Determine power source based on detected devices
            if (hasAC && acOnline)
            {
                return PowerSource::AC_POWER;
            }
            else if (hasBattery)
            {
                return PowerSource::BATTERY;
            }
            else
            {
                return PowerSource::UNKNOWN;
            }
        }
        catch (const std::exception&)
        {
            // If we can't determine power source, assume unknown
            return PowerSource::UNKNOWN;
        }
    }
};

std::unique_ptr<IPlatformUtils> createLinuxPlatformUtils()
{
    return std::make_unique<LinuxPlatformUtils>();
}
