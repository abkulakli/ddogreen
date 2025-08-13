#pragma once

#include <gmock/gmock.h>
#include "platform/isystem_monitor.h"

class MockSystemMonitor : public ISystemMonitor {
public:
    MOCK_METHOD(double, getLoadAverage, (), (override));
    MOCK_METHOD(int, getCpuCoreCount, (), (override));
    MOCK_METHOD(bool, isAvailable, (), (override));
    MOCK_METHOD(void, setMonitoringFrequency, (int frequencySeconds), (override));
};
