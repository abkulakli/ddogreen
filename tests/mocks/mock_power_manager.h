#pragma once

#include <gmock/gmock.h>
#include "platform/ipower_manager.h"

class MockPowerManager : public IPowerManager {
public:
    MOCK_METHOD(bool, setPerformanceMode, (), (override));
    MOCK_METHOD(bool, setPowerSavingMode, (), (override));
    MOCK_METHOD(std::string, getCurrentMode, (), (override));
    MOCK_METHOD(bool, isAvailable, (), (override));
};
