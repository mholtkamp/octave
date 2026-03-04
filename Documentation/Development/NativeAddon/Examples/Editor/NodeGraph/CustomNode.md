# Custom Node Example

## Overview

This example demonstrates how to create a custom node (`RandomFloatNode`) and register it with multiple graph domains using `REGISTER_GRAPH_NODE_MULTI`. The node generates a pseudo-random float value based on a seed input.

## Files

### RandomFloatNode.h

```cpp
#pragma once

#include "NodeGraph/GraphNode.h"

class RandomFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomFloatNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random Float"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};
```

### RandomFloatNode.cpp

```cpp
#include "RandomFloatNode.h"
#include "Utilities.h"

FORCE_LINK_DEF(RandomFloatNode);

DEFINE_GRAPH_NODE(RandomFloatNode);

// Register with both Material and Shader domains
REGISTER_GRAPH_NODE_MULTI(RandomFloatNode, "Random Float", "Utility",
    glm::vec4(0.6f, 0.4f, 0.8f, 1.0f), "Material", "Shader");

void RandomFloatNode::SetupPins()
{
    AddInputPin("Seed", DatumType::Float, Datum(0.0f));
    AddInputPin("Min", DatumType::Float, Datum(0.0f));
    AddInputPin("Max", DatumType::Float, Datum(1.0f));
    AddOutputPin("Value", DatumType::Float);
}

void RandomFloatNode::Evaluate()
{
    float seed = GetInputValue(0).GetFloat();
    float minVal = GetInputValue(1).GetFloat();
    float maxVal = GetInputValue(2).GetFloat();

    // Simple hash-based pseudo-random
    float hash = glm::fract(glm::sin(seed * 12.9898f) * 43758.5453f);
    float result = glm::mix(minVal, maxVal, hash);

    SetOutputValue(0, Datum(result));
}

glm::vec4 RandomFloatNode::GetNodeColor() const
{
    return glm::vec4(0.6f, 0.4f, 0.8f, 1.0f);
}
```

### Engine.cpp Integration

Add the force linkage call to `ForceLinkage()`:

```cpp
FORCE_LINK_CALL(RandomFloatNode);
```

## API Reference

### REGISTER_GRAPH_NODE(Class, TypeName, Category, DomainName, Color)

Registers a node with a single domain. The node appears in that domain's context menu.

| Parameter | Type | Description |
|---|---|---|
| Class | identifier | Node class name (must match DEFINE_GRAPH_NODE) |
| TypeName | string | Display name in context menu |
| Category | string | Category grouping in context menu |
| DomainName | string | Target domain (e.g. "Material") |
| Color | glm::vec4 | Node header color |

### REGISTER_GRAPH_NODE_MULTI(Class, TypeName, Category, Color, ...)

Registers a node with multiple domains at once.

| Parameter | Type | Description |
|---|---|---|
| Class | identifier | Node class name |
| TypeName | string | Display name |
| Category | string | Category grouping |
| Color | glm::vec4 | Node header color |
| ... | strings | One or more domain names |

## Best Practices

- Use `REGISTER_GRAPH_NODE_MULTI` when a utility node is useful across multiple domains.
- Choose a node color that matches its category for visual consistency.
- Always provide sensible default values for input pins.
- The duplicate-check in `AddNodeType()` prevents the same node from appearing twice if it's also registered by the domain's `RegisterNodeTypes()`.
