#include "security_utils.h"
#include "logger.h"
#include <filesystem>
#include <algorithm>
#include <vector>

namespace fs = std::filesystem;

bool SecurityUtils::validateConfigFilePermissions(const std::string& filePath)
{
    try
    {
        // Check if file exists
        if (!fs::exists(filePath))
        {
            Logger::warning("Configuration file does not exist: " + filePath);
            return false;
        }

        // Get file permissions
        auto perms = fs::status(filePath).permissions();

#ifndef _WIN32
        // On Unix systems, check that file is not world-writable
        bool isWorldWritable = (perms & fs::perms::others_write) != fs::perms::none;

        if (isWorldWritable)
        {
            Logger::error("Security: Configuration file is world-writable: " + filePath);
            Logger::error("Fix with: chmod o-w " + filePath);
            return false;
        }

        // Check that file is not group-writable unless we're in the group
        bool isGroupWritable = (perms & fs::perms::group_write) != fs::perms::none;
        if (isGroupWritable)
        {
            Logger::warning("Configuration file is group-writable: " + filePath);
            Logger::warning("Consider: chmod g-w " + filePath);
            // Don't fail on group-writable, just warn
        }
#else
        // On Windows, basic permission check
        // Windows permissions are more complex, but we can at least check basic access
        bool isReadOnly = (perms & fs::perms::owner_write) == fs::perms::none;
        if (isReadOnly)
        {
            Logger::warning("Configuration file is read-only: " + filePath);
            // Read-only is actually good for security, don't fail
        }
#endif

        Logger::debug("Configuration file permissions validated: " + filePath);
        return true;
    }
    catch (const fs::filesystem_error& e)
    {
        Logger::error("Failed to check file permissions: " + std::string(e.what()));
        return false;
    }
    catch (const std::exception& e)
    {
        Logger::error("Error validating file permissions: " + std::string(e.what()));
        return false;
    }
}

bool SecurityUtils::validatePathTraversal(const std::string& path)
{
    // Check for common path traversal sequences
    std::vector<std::string> dangerousSequences = {
        "../",      // Unix path traversal
        "..\\",     // Windows path traversal
        "/..",      // Unix path traversal at end
        "\\..",     // Windows path traversal at end
        "%2e%2e",   // URL encoded ..
        "%2E%2E",   // URL encoded .. (uppercase)
        "..%2f",    // URL encoded ../
        "..%5c"     // URL encoded ..\ (backslash)
    };

    // Use std::any_of for better performance and readability
    auto foundDangerous = std::any_of(dangerousSequences.begin(), dangerousSequences.end(),
        [&path](const std::string& sequence) {
            return path.find(sequence) != std::string::npos;
        });
    
    if (foundDangerous)
    {
        // Find which sequence triggered for logging
        auto it = std::find_if(dangerousSequences.begin(), dangerousSequences.end(),
            [&path](const std::string& sequence) {
                return path.find(sequence) != std::string::npos;
            });
        
        Logger::error("Security: Path traversal sequence detected in path: " + path);
        Logger::error("Dangerous sequence: " + *it);
        return false;
    }

    // Additional check for excessive parent directory references
    size_t dotDotCount = 0;
    size_t pos = 0;
    while ((pos = path.find("..", pos)) != std::string::npos)
    {
        dotDotCount++;
        pos += 2;

        // If we see too many ".." sequences, it's suspicious
        if (dotDotCount > 3)
        {
            Logger::error("Security: Excessive parent directory references in path: " + path);
            return false;
        }
    }

    return true;
}

std::string SecurityUtils::canonicalizePath(const std::string& path)
{
    try
    {
        // First validate that the path doesn't contain traversal sequences
        if (!validatePathTraversal(path))
        {
            Logger::error("Path validation failed for: " + path);
            return "";
        }

        // Use filesystem library to canonicalize the path
        fs::path fsPath(path);

        // If path is relative, make it absolute relative to current directory
        if (fsPath.is_relative())
        {
            fsPath = fs::absolute(fsPath);
        }

        // Canonicalize to resolve any . or .. components
        fs::path canonical = fs::weakly_canonical(fsPath);

        std::string result = canonical.string();
        Logger::debug("Canonicalized path: " + path + " -> " + result);

        return result;
    }
    catch (const fs::filesystem_error& e)
    {
        Logger::error("Failed to canonicalize path '" + path + "': " + std::string(e.what()));
        return "";
    }
    catch (const std::exception& e)
    {
        Logger::error("Error canonicalizing path '" + path + "': " + std::string(e.what()));
        return "";
    }
}

bool SecurityUtils::isPathWithinDirectory(const std::string& path, const std::string& allowedDir)
{
    try
    {
        // Canonicalize both paths to resolve any relative components
        std::string canonicalPath = canonicalizePath(path);
        std::string canonicalAllowedDir = canonicalizePath(allowedDir);

        if (canonicalPath.empty() || canonicalAllowedDir.empty())
        {
            Logger::error("Failed to canonicalize paths for security check");
            return false;
        }

        // Ensure allowed directory ends with separator for proper prefix matching
        if (canonicalAllowedDir.back() != '/' &&
            canonicalAllowedDir.back() != '\\')
        {
            canonicalAllowedDir += fs::path::preferred_separator;
        }

        // Check if the canonical path starts with the allowed directory
        bool isWithin = canonicalPath.find(canonicalAllowedDir) == 0;

        if (!isWithin)
        {
            Logger::error("Security: Path is outside allowed directory");
            Logger::error("Path: " + canonicalPath);
            Logger::error("Allowed directory: " + canonicalAllowedDir);
        }

        return isWithin;
    }
    catch (const std::exception& e)
    {
        Logger::error("Error checking path containment: " + std::string(e.what()));
        return false;
    }
}
