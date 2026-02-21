#pragma once

#include "NodeGraph/GraphTypes.h"

struct GraphLink
{
    GraphLinkId mId = INVALID_GRAPH_LINK_ID;
    GraphPinId mOutputPinId = INVALID_GRAPH_PIN_ID;
    GraphPinId mInputPinId = INVALID_GRAPH_PIN_ID;
    GraphNodeId mOutputNodeId = INVALID_GRAPH_NODE_ID;
    GraphNodeId mInputNodeId = INVALID_GRAPH_NODE_ID;
};
