#pragma once

#include <stdint.h>

struct EmbeddedFile
{
    const char* mName;
    const char* mData;
    uint32_t mSize;
    bool mEngine;
};
