#pragma once

#include <gmock/gmock.h>
#include "platform/isignal_handler.h"

class MockSignalHandler : public ISignalHandler {
public:
    MOCK_METHOD(void, setupSignalHandlers, (), (override));
    MOCK_METHOD(bool, shouldRun, (), (override));
    MOCK_METHOD(void, waitForSignal, (), (override));
};
