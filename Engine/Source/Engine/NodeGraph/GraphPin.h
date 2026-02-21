#pragma once

#include "NodeGraph/GraphTypes.h"

#include <string>

struct GraphPin
{
    GraphPinId mId = INVALID_GRAPH_PIN_ID;
    GraphNodeId mOwnerNodeId = INVALID_GRAPH_NODE_ID;
    std::string mName;
    DatumType mDataType = DatumType::Float;
    GraphPinDirection mDirection = GraphPinDirection::Input;
    Datum mDefaultValue;
    Datum mValue;
    bool mExecutionTriggered = false;
};
