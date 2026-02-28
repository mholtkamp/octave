#pragma once

#include "Datum.h"

#include <cstdint>

typedef uint32_t GraphNodeId;
typedef uint32_t GraphPinId;
typedef uint32_t GraphLinkId;

constexpr GraphNodeId INVALID_GRAPH_NODE_ID = 0;
constexpr GraphPinId INVALID_GRAPH_PIN_ID = 0;
constexpr GraphLinkId INVALID_GRAPH_LINK_ID = 0;

enum class GraphPinDirection : uint8_t
{
    Input,
    Output
};

bool AreGraphPinTypesCompatible(DatumType outputType, DatumType inputType);

const char* GetDatumTypeName(DatumType type);
glm::vec4 GetDatumTypeColor(DatumType type);
