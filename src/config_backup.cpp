#include "config.h"
#include "logger.h"
#include "platform/platform_factory.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <span>

Config::Config() : m_monitoringFrequency{0}, m_highPerformanceThreshold{0.0}, m_powerSaveThreshold{0.0}
{
}

std::string Config::getDefaultConfigPath()
{
    auto platformUtils = PlatformFactory::createPlatformUtils();
    if (platformUtils && platformUtils->isAvailable())
    {
        return platformUtils->getDefaultConfigPath();
    }
    return "/etc/ddogreen/ddogreen.conf";
}

bool Config::loadFromFile(const std::string& configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        Logger::error("Configuration file not found: " + configPath);
        return false;
    }

    Logger::info("Loading configuration from: " + configPath);

    std::string line;
    int lineNumber = 0;
    bool hasErrors = false;

    while (std::getline(file, line))
    {
        lineNumber++;

        line = trim(std::span<const char>{line.data(), line.size()});
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        if (!parseLineString(line))
        {
            Logger::warning("Invalid configuration line " + std::to_string(lineNumber) + ": " + line);
            hasErrors = true;
        }
    }

    file.close();

    if (m_monitoringFrequency == 0)
    {
        Logger::error("Missing required configuration: monitoring_frequency");
        hasErrors = true;
    }
    if (m_highPerformanceThreshold == 0.0)
    {
        Logger::error("Missing required configuration: high_performance_threshold");
        hasErrors = true;
    }
    if (m_powerSaveThreshold == 0.0)
    {
        Logger::error("Missing required configuration: power_save_threshold");
        hasErrors = true;
    }

    if (hasErrors)
    {
        Logger::error("Configuration contains errors in: " + configPath + " - please fix the configuration file and restart");
        return false;
    }

    // Cross-validation: ensure power_save_threshold < high_performance_threshold
    if (m_powerSaveThreshold >= m_highPerformanceThreshold)
    {
        Logger::error("Configuration error: power_save_threshold (" + std::to_string(m_powerSaveThreshold) +
                     ") must be less than high_performance_threshold (" + std::to_string(m_highPerformanceThreshold) + ")");
        Logger::error("This ensures proper hysteresis behavior and prevents rapid mode switching");
        return false;
    }

    if (!validateConfiguration())
    {
        return false;
    }

    Logger::info("Configuration loaded successfully from: " + configPath);

    Logger::info("Monitoring frequency: " + std::to_string(m_monitoringFrequency) + " seconds");
    Logger::info("High performance threshold: " + std::to_string(m_highPerformanceThreshold) + " (" + std::to_string(m_highPerformanceThreshold * 100) + "%)");
    Logger::info("Power save threshold: " + std::to_string(m_powerSaveThreshold) + " (" + std::to_string(m_powerSaveThreshold * 100) + "%)");

    return true;
}

}

bool Config::validateConfiguration() const
{
    // Validate reasonable ranges and relationships
    double thresholdGap = m_highPerformanceThreshold - m_powerSaveThreshold;

    if (thresholdGap < 0.1)
    {
        Logger::warning("Small threshold gap (" + std::to_string(thresholdGap * 100) +
                       "%) may cause frequent mode switching. Recommended minimum: 10%");
    }

    if (m_monitoringFrequency < 10)
    {
        Logger::warning("Very frequent monitoring (" + std::to_string(m_monitoringFrequency) +
                       "s) may impact system performance. Consider 10+ seconds for production");
    }

    if (m_highPerformanceThreshold > 0.8)
    {
        Logger::warning("Very high performance threshold (" + std::to_string(m_highPerformanceThreshold * 100) +
                       "%) may rarely trigger performance mode");
    }

    if (m_powerSaveThreshold < 0.1)
    {
        Logger::warning("Very low power save threshold (" + std::to_string(m_powerSaveThreshold * 100) +
                       "%) may rarely trigger power save mode");
    }

    return true;
}

bool Config::validateConfiguration() const {
    // Validate reasonable ranges and relationships
    double thresholdGap = m_highPerformanceThreshold - m_powerSaveThreshold;

    if (thresholdGap < 0.1) {
        Logger::warning("Small threshold gap (" + std::to_string(thresholdGap * 100) +
                       "%) may cause frequent mode switching. Recommended minimum: 10%");
    }

    if (m_monitoringFrequency < 10) {
        Logger::warning("Very frequent monitoring (" + std::to_string(m_monitoringFrequency) +
                       "s) may impact system performance. Consider 10+ seconds for production");
    }

    if (m_highPerformanceThreshold > 0.9) {
        Logger::warning("Very high performance threshold (" + std::to_string(m_highPerformanceThreshold * 100) +
                       "%) may rarely trigger performance mode");
    }

    if (m_powerSaveThreshold < 0.1)
    {
        Logger::warning("Very low power save threshold (" + std::to_string(m_powerSaveThreshold * 100) +
                       "%) may rarely trigger power save mode");
    }

    return true;
}

bool Config::loadFromBuffer(std::span<const char> configData)
{
    if (configData.empty())
    {
        Logger::error("Configuration buffer is empty");
        return false;
    }

    Logger::info("Loading configuration from buffer");

    // Split buffer into lines and process each
    std::string_view dataView{configData.data(), configData.size()};
    size_t pos = 0;
    int lineNumber = 0;
    bool hasErrors = false;

    while (pos < dataView.size())
    {
        size_t lineEnd = dataView.find('\n', pos);
        if (lineEnd == std::string_view::npos)
        {
            lineEnd = dataView.size();
        }

        std::span<const char> lineSpan = configData.subspan(pos, lineEnd - pos);
        lineNumber++;

        // Remove carriage return if present
        if (!lineSpan.empty() && lineSpan.back() == '\r')
        {
            lineSpan = lineSpan.first(lineSpan.size() - 1);
        }

        std::string trimmedLine = trim(lineSpan);
        if (!trimmedLine.empty() && trimmedLine[0] != '#')
        {
            if (!parseLineString(trimmedLine))
            {
                Logger::warning("Invalid configuration line " + std::to_string(lineNumber) + ": " + trimmedLine);
                hasErrors = true;
            }
        }

        pos = lineEnd + 1;
    }

    // Validation same as file loading
    if (m_monitoringFrequency == 0)
    {
        Logger::error("Missing required configuration: monitoring_frequency");
        hasErrors = true;
    }
    if (m_highPerformanceThreshold == 0.0)
    {
        Logger::error("Missing required configuration: high_performance_threshold");
        hasErrors = true;
    }
    if (m_powerSaveThreshold == 0.0)
    {
        Logger::error("Missing required configuration: power_save_threshold");
        hasErrors = true;
    }

    if (hasErrors)
    {
        Logger::error("Configuration buffer contains errors - please fix the configuration and reload");
        return false;
    }

    // Cross-validation
    if (m_powerSaveThreshold >= m_highPerformanceThreshold)
    {
        Logger::error("Configuration error: power_save_threshold (" + std::to_string(m_powerSaveThreshold) +
                     ") must be less than high_performance_threshold (" + std::to_string(m_highPerformanceThreshold) + ")");
        return false;
    }

    if (!validateConfiguration())
    {
        return false;
    }

    Logger::info("Configuration loaded successfully from buffer");
    return true;
}

std::string Config::trim(std::span<const char> str) const
{
    if (str.empty())
    {
        return "";
    }

    // Find first non-whitespace character
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
    {
        ++start;
    }

    if (start == str.size())
    {
        return "";  // All whitespace
    }

    // Find last non-whitespace character
    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end])))
    {
        --end;
    }

    return std::string{str.data() + start, end - start + 1};
}

bool Config::parseLine(std::span<const char> line)
{
    if (line.empty())
    {
        return false;
    }

    // Find the equals sign
    auto equalPos = std::find(line.begin(), line.end(), '=');
    if (equalPos == line.end())
    {
        return false;
    }

    size_t equalIndex = std::distance(line.begin(), equalPos);
    
    std::span<const char> keySpan = line.first(equalIndex);
    std::span<const char> valueSpan = line.subspan(equalIndex + 1);

    std::string key = trim(keySpan);
    std::string value = trim(valueSpan);

    if (key.empty() || value.empty())
    {
        return false;
    }

    return parseKeyValue(key, value);
}

bool Config::parseLineString(const std::string& line)
{
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos)
    {
        return false;
    }

    std::string key = trim(std::span<const char>{line.data(), equalPos});
    std::string value = trim(std::span<const char>{line.data() + equalPos + 1, line.size() - equalPos - 1});

    if (key.empty() || value.empty())
    {
        return false;
    }

    return parseKeyValue(key, value);
}

bool Config::parseKeyValue(const std::string& key, const std::string& value)
{
    try
    {
        if (key == "monitoring_frequency")
        {
            int freq = std::stoi(value);
            if (freq >= 1 && freq <= 300)
            {
                m_monitoringFrequency = freq;
                return true;
            }
            else
            {
                Logger::warning("monitoring_frequency value " + value + " out of range (1-300 seconds)");
            }
        }
        else if (key == "high_performance_threshold")
        {
            double threshold = std::stod(value);
            if (threshold >= 0.1 && threshold <= 1.0)
            {
                m_highPerformanceThreshold = threshold;
                return true;
            }
            else
            {
                Logger::warning("high_performance_threshold value " + value + " out of range (0.1-1.0)");
            }
        }
        else if (key == "power_save_threshold")
        {
            double threshold = std::stod(value);
            if (threshold >= 0.05 && threshold <= 0.9)
            {
                m_powerSaveThreshold = threshold;
                return true;
            }
            else
            {
                Logger::warning("power_save_threshold value " + value + " out of range (0.05-0.9)");
            }
        }
        else
        {
            Logger::warning("Unknown configuration key: " + key);
        }
    }
    catch (const std::invalid_argument& e)
    {
        Logger::warning("Invalid numeric value for " + key + ": " + value);
    }
    catch (const std::out_of_range& e)
    {
        Logger::warning("Numeric value out of range for " + key + ": " + value);
    }
    catch (const std::exception& e)
    {
        Logger::warning("Error parsing " + key + ": " + e.what());
    }

    return false;
}
