#pragma once

#include <string>
#include <filesystem>
#include <system_error>

/**
 * Security utility functions for file and path validation
 */
class SecurityUtils
{
public:
    /**
     * Validate that a configuration file has secure permissions
     * Checks that the file is not world-writable and owned by appropriate user
     * @param filePath path to the configuration file
     * @return true if file permissions are secure
     */
    static bool validateConfigFilePermissions(const std::string& filePath);

    /**
     * Validate that a path doesn't contain traversal sequences
     * Prevents access to files outside intended directories
     * @param path path to validate
     * @return true if path is safe (no traversal sequences)
     */
    static bool validatePathTraversal(const std::string& path);

    /**
     * Canonicalize a path to resolve any relative components
     * Converts relative paths to absolute paths safely
     * @param path input path (may be relative)
     * @return canonicalized absolute path, or empty string if invalid
     */
    static std::string canonicalizePath(const std::string& path);

    /**
     * Check if a path is within an allowed directory
     * Ensures file access is restricted to specific directories
     * @param path path to check
     * @param allowedDir allowed base directory
     * @return true if path is within allowed directory
     */
    static bool isPathWithinDirectory(const std::string& path, const std::string& allowedDir);

private:
    SecurityUtils() = default; // Static utility class
};
