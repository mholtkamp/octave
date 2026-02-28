#pragma once

#include <cstdint>

enum class InterpMode : uint8_t
{
    Linear,
    Step,
    Cubic,

    Count
};
