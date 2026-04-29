#pragma once
#include <cstdint>

enum class CommandType : uint8_t {
    CONFIG_AND_DATA = 1,
    START_COMPUTATION = 2,
    REQUEST_STATUS = 3,
    STATUS_RESPONSE = 4
};

enum class ClientStatus : int {
    INIT = 0,
    RUNNING = 1,
    DONE = 2,
    ERROR = 3
};
