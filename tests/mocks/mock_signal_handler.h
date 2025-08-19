#ifndef DDOGREEN_MOCK_SIGNAL_HANDLER_H
#define DDOGREEN_MOCK_SIGNAL_HANDLER_H

#include <gmock/gmock.h>
#include "platform/isignal_handler.h"

class MockSignalHandler : public ISignalHandler {
public:
    MOCK_METHOD(void, setupSignalHandlers, (), (override));
    MOCK_METHOD(bool, shouldRun, (), (override));
    MOCK_METHOD(void, waitForSignal, (), (override));
};

#endif // DDOGREEN_MOCK_SIGNAL_HANDLER_H
