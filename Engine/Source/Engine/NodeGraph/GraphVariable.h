#pragma once

#include "Datum.h"

#include <string>

struct GraphVariable
{
    std::string mName;
    DatumType mType = DatumType::Float;
    Datum mDefaultValue;
    Datum mRuntimeValue;  // reset to mDefaultValue on play
};
