#include "platform/iservice_manager.h"
#include "logger.h"
#include <cstdlib>
#include <fstream>
#include <memory>
#include <array>
#include <string>

/**
 * Linux-specific service manager implementation
 * Uses systemd for service management
 */
class LinuxServiceManager : public IServiceManager {
public:
    LinuxServiceManager() = default;
    virtual ~LinuxServiceManager() = default;

    /**
     * Install systemd service
     * @param serviceName name of the service (e.g., "ddogreen")
     * @param executablePath full path to executable
     * @param description service description
     * @return true if successful
     */
    bool installService(const std::string& serviceName, 
                       const std::string& executablePath,
                       const std::string& description) override {
        
        // Define the target installation path
        std::string targetExecutablePath = "/usr/local/bin/" + serviceName;
        
        Logger::info("Setting up directories and permissions...");
        
        // Copy executable to system location
        Logger::info("Installing executable to " + targetExecutablePath + "...");
        std::string copyCommand = "cp \"" + executablePath + "\" \"" + targetExecutablePath + "\"";
        if (!executeCommand(copyCommand)) {
            Logger::error("Failed to copy executable to " + targetExecutablePath);
            return false;
        }
        
        // Set proper permissions
        if (!executeCommand("chmod 755 \"" + targetExecutablePath + "\"")) {
            Logger::warning("Failed to set executable permissions");
        }
        
        // Create log directory and file
        if (!executeCommand("mkdir -p /var/log")) {
            Logger::warning("Failed to create /var/log directory");
        }
        std::string logFile = "/var/log/" + serviceName + ".log";
        if (!executeCommand("touch " + logFile)) {
            Logger::warning("Failed to create log file");
        }
        if (!executeCommand("chmod 644 " + logFile)) {
            Logger::warning("Failed to set log file permissions");
        }
        
        // Create config directory
        std::string configDir = "/etc/" + serviceName;
        if (!executeCommand("mkdir -p " + configDir)) {
            Logger::warning("Failed to create config directory");
        }
        
        std::string serviceFile = "/etc/systemd/system/" + serviceName + ".service";
        
        // Create systemd service file content with security settings
        std::string serviceContent = 
            "[Unit]\n"
            "Description=" + description + "\n"
            "Documentation=https://github.com/abkulakli/ddogreen\n"
            "Documentation=https://www.ddosoft.com\n"
            "After=multi-user.target\n"
            "\n"
            "[Service]\n"
            "Type=forking\n"
            "ExecStart=" + targetExecutablePath + " --daemon\n"
            "ExecReload=/bin/kill -HUP $MAINPID\n"
            "PIDFile=/run/" + serviceName + ".pid\n"
            "Restart=always\n"
            "RestartSec=10\n"
            "User=root\n"
            "Group=root\n"
            "TimeoutStartSec=30\n"
            "RemainAfterExit=no\n"
            "\n"
            "# Security settings\n"
            "NoNewPrivileges=yes\n"
            "ProtectSystem=strict\n"
            "ProtectHome=yes\n"
            "ReadWritePaths=/var/log /run /tmp /proc\n"
            "PrivateTmp=yes\n"
            "ProtectKernelTunables=yes\n"
            "ProtectKernelModules=yes\n"
            "ProtectControlGroups=yes\n"
            "\n"
            "[Install]\n"
            "WantedBy=multi-user.target\n";
        
        // Write service file
        std::ofstream file(serviceFile);
        if (!file.is_open()) {
            Logger::error("Failed to create service file: " + serviceFile);
            return false;
        }
        
        file << serviceContent;
        file.close();
        
        // Reload systemd daemon
        if (!executeCommand("systemctl daemon-reload")) {
            Logger::error("Failed to reload systemd daemon");
            return false;
        }

        // Enable the service for auto-start on boot
        if (!enableService(serviceName)) {
            Logger::warning("Service installed but failed to enable auto-start");
        }

        // Start the service immediately
        if (!startService(serviceName)) {
            Logger::warning("Service installed and enabled but failed to start");
        }
        
        Logger::info("Successfully installed, enabled, and started service: " + serviceName);
        return true;
    }

    /**
     * Uninstall systemd service
     * @param serviceName name of the service to uninstall
     * @return true if successful
     */
    bool uninstallService(const std::string& serviceName) override {
        // Stop and disable service first
        stopService(serviceName);
        disableService(serviceName);
        
        std::string serviceFile = "/etc/systemd/system/" + serviceName + ".service";
        
        // Remove service file
        if (std::remove(serviceFile.c_str()) != 0) {
            Logger::warning("Could not remove service file: " + serviceFile);
        }
        
        // Reload systemd daemon
        executeCommand("systemctl daemon-reload");
        
        Logger::info("Cleaning up files...");
        
        // Remove installed executable
        std::string executablePath = "/usr/local/bin/" + serviceName;
        if (!executeCommand("rm -f " + executablePath)) {
            Logger::warning("Failed to remove installed executable");
        }
        
        // Clean up log and config files
        std::string logFile = "/var/log/" + serviceName + ".log";
        if (!executeCommand("rm -f " + logFile)) {
            Logger::warning("Failed to remove log file");
        }
        
        std::string pidFile = "/run/" + serviceName + ".pid";
        if (!executeCommand("rm -f " + pidFile)) {
            Logger::warning("Failed to remove PID file");
        }
        
        std::string configDir = "/etc/" + serviceName;
        if (!executeCommand("rm -rf " + configDir)) {
            Logger::warning("Failed to remove config directory");
        }
        
        Logger::info("Successfully uninstalled service: " + serviceName);
        return true;
    }

    /**
     * Start systemd service
     * @param serviceName name of the service to start
     * @return true if successful
     */
    bool startService(const std::string& serviceName) override {
        std::string command = "systemctl start " + serviceName;
        bool result = executeCommand(command);
        
        if (result) {
            Logger::info("Started service: " + serviceName);
        } else {
            Logger::error("Failed to start service: " + serviceName);
        }
        
        return result;
    }

    /**
     * Stop systemd service
     * @param serviceName name of the service to stop
     * @return true if successful
     */
    bool stopService(const std::string& serviceName) override {
        std::string command = "systemctl stop " + serviceName;
        bool result = executeCommand(command);
        
        if (result) {
            Logger::info("Stopped service: " + serviceName);
        } else {
            Logger::error("Failed to stop service: " + serviceName);
        }
        
        return result;
    }

    /**
     * Enable systemd service for automatic startup
     * @param serviceName name of the service to enable
     * @return true if successful
     */
    bool enableService(const std::string& serviceName) override {
        std::string command = "systemctl enable " + serviceName;
        bool result = executeCommand(command);
        
        if (result) {
            Logger::info("Enabled service: " + serviceName);
        } else {
            Logger::error("Failed to enable service: " + serviceName);
        }
        
        return result;
    }

    /**
     * Disable systemd service from starting automatically
     * @param serviceName name of the service to disable
     * @return true if successful
     */
    bool disableService(const std::string& serviceName) override {
        std::string command = "systemctl disable " + serviceName;
        bool result = executeCommand(command);
        
        if (result) {
            Logger::info("Disabled service: " + serviceName);
        } else {
            Logger::error("Failed to disable service: " + serviceName);
        }
        
        return result;
    }

    /**
     * Check if service management is available on this platform
     * @return true if service management is supported
     */
    bool isAvailable() override {
        return executeCommand("which systemctl > /dev/null 2>&1");
    }

    /**
     * Get service status
     * @param serviceName name of the service
     * @return status string ("running", "stopped", "unknown")
     */
    std::string getServiceStatus(const std::string& serviceName) override {
        std::string command = "systemctl is-active " + serviceName;
        std::string output = executeCommandWithOutput(command);
        
        // Remove trailing newline
        if (!output.empty() && output.back() == '\n') {
            output.pop_back();
        }
        
        if (output == "active") {
            return "running";
        } else if (output == "inactive" || output == "failed") {
            return "stopped";
        } else {
            return "unknown";
        }
    }

private:
    /**
     * Execute a command and return success status
     * @param command command to execute
     * @return true if command succeeded
     */
    bool executeCommand(const std::string& command) {
        Logger::debug("Executing command: " + command);
        int result = std::system(command.c_str());
        return (result == 0);
    }

    /**
     * Execute a command and return its output
     * @param command command to execute
     * @return command output as string
     */
    std::string executeCommandWithOutput(const std::string& command) {
        Logger::debug("Executing command with output: " + command);

        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            Logger::error("Failed to execute command: " + command);
            return "";
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

        pclose(pipe);
        return result;
    }
};

// Factory function for creating Linux service manager
std::unique_ptr<IServiceManager> createLinuxServiceManager() {
    return std::make_unique<LinuxServiceManager>();
}
