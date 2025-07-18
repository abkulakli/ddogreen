#include "platform/iplatform_utils.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <memory>

/**
 * Windows-specific platform utilities implementation
 */
class WindowsPlatformUtils : public IPlatformUtils {
public:
    WindowsPlatformUtils() = default;
    virtual ~WindowsPlatformUtils() = default;

    /**
     * Check if running with administrator privileges
     * @return true if running as administrator
     */
    bool hasRequiredPrivileges() const override {
        BOOL isAdmin = FALSE;
        PSID administratorsGroup = NULL;
        
        // Allocate and initialize a SID of the administrators group.
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup)) {
            // Determine whether the SID of administrators group is enabled in the primary access token of the process.
            if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin)) {
                isAdmin = FALSE;
            }
            FreeSid(administratorsGroup);
        }
        
        return isAdmin == TRUE;
    }

    /**
     * Get the full path to the current executable on Windows
     * @return full path to executable, or empty string if failed
     */
    std::string getExecutablePath() const override {
        char path[MAX_PATH];
        DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
        if (result == 0 || result == MAX_PATH) {
            return "";
        }
        return std::string(path);
    }

    /**
     * Parse command line arguments using simple Windows parsing
     * @param argc argument count
     * @param argv argument vector
     * @return parsed arguments structure
     */
    ParsedArgs parseCommandLine(int argc, char* argv[]) const override {
        ParsedArgs args;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-d" || arg == "--daemon") {
                args.runAsDaemon = true;
            } else if (arg == "-h" || arg == "--help") {
                args.showHelp = true;
            } else if (arg == "-v" || arg == "--version") {
                args.showVersion = true;
            } else if (arg == "-i" || arg == "--install") {
                args.install = true;
            } else if (arg == "-u" || arg == "--uninstall") {
                args.uninstall = true;
            } else {
                args.hasUnknownOptions = true;
                args.unknownOption = arg;
            }
        }
        
        return args;
    }

    /**
     * Get the default installation path for Windows
     * @return C:\Program Files\DDOSoft\ddogreen\ddogreen.exe
     */
    std::string getDefaultInstallPath() const override {
        return "C:\\Program Files\\DDOSoft\\ddogreen\\ddogreen.exe";
    }

    /**
     * Get the default log file path for Windows
     * @return C:\ProgramData\DDOSoft\ddogreen\ddogreen.log
     */
    std::string getDefaultLogPath() const override {
        return "C:\\ProgramData\\DDOSoft\\ddogreen\\ddogreen.log";
    }

    /**
     * Get the default PID file path for Windows
     * @return C:\ProgramData\DDOSoft\ddogreen\ddogreen.pid
     */
    std::string getDefaultPidPath() const override {
        return "C:\\ProgramData\\DDOSoft\\ddogreen\\ddogreen.pid";
    }

    /**
     * Check if platform utilities are available
     * @return true (always available on Windows)
     */
    bool isAvailable() const override {
        return true;
    }

    /**
     * Get the privilege escalation message for Windows
     * @return message explaining to run as administrator
     */
    std::string getPrivilegeEscalationMessage() const override {
        return "This program requires administrator privileges. Please run as administrator.";
    }
};

// Factory function to create Windows platform utilities
std::unique_ptr<IPlatformUtils> createWindowsPlatformUtils() {
    return std::make_unique<WindowsPlatformUtils>();
}
