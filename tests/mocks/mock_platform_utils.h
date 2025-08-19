#ifndef DDOGREEN_MOCK_PLATFORM_UTILS_H
#define DDOGREEN_MOCK_PLATFORM_UTILS_H

#include <gmock/gmock.h>
#include "platform/iplatform_utils.h"

class MockPlatformUtils : public IPlatformUtils {
public:
    MOCK_METHOD(bool, hasRequiredPrivileges, (), (const, override));
    MOCK_METHOD(std::string, getExecutablePath, (), (const, override));
    MOCK_METHOD(ParsedArgs, parseCommandLine, (int argc, char* argv[]), (const, override));
    MOCK_METHOD(std::string, getDefaultInstallPath, (), (const, override));
    MOCK_METHOD(std::string, getDefaultLogPath, (), (const, override));
    MOCK_METHOD(std::string, getDefaultPidPath, (), (const, override));
    MOCK_METHOD(std::string, getDefaultConfigPath, (), (const, override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
    MOCK_METHOD(std::string, getPrivilegeEscalationMessage, (), (const, override));
    MOCK_METHOD(std::string, resolveAbsolutePath, (const std::string& relativePath), (const, override));
};

#endif // DDOGREEN_MOCK_PLATFORM_UTILS_H
