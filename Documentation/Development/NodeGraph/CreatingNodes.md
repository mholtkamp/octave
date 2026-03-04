# Creating Custom Nodes

This guide walks through creating a custom node for the node graph system.

## Step 1: Define the Node Class

Create a header file for your node. Use `DECLARE_GRAPH_NODE` to set up the factory pattern.

```cpp
// MyCustomNodes.h
#pragma once

#include "NodeGraph/GraphNode.h"

class MyCustomNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MyCustomNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "My Custom Node"; }
    virtual const char* GetNodeCategory() const override { return "Custom"; }
    virtual glm::vec4 GetNodeColor() const override;
};
```

## Step 2: Implement the Node

In the .cpp file, use `FORCE_LINK_DEF`, `DEFINE_GRAPH_NODE`, and implement the virtual methods.

```cpp
// MyCustomNodes.cpp
#include "MyCustomNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(MyCustomNodes);
DEFINE_GRAPH_NODE(MyCustomNode);

void MyCustomNode::SetupPins()
{
    // Add input pins with default values
    AddInputPin("Value A", DatumType::Float, Datum(0.0f));
    AddInputPin("Value B", DatumType::Float, Datum(1.0f));

    // Add output pins
    AddOutputPin("Result", DatumType::Float);
}

void MyCustomNode::Evaluate()
{
    // Read input values
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();

    // Compute and set output
    SetOutputValue(0, Datum(a * b + 0.5f));
}

glm::vec4 MyCustomNode::GetNodeColor() const
{
    return glm::vec4(0.8f, 0.4f, 0.1f, 1.0f);
}
```

## Step 3: Register the Node with a Domain

### Option A: Register with a single domain

Use `REGISTER_GRAPH_NODE` after `DEFINE_GRAPH_NODE`:

```cpp
DEFINE_GRAPH_NODE(MyCustomNode);
REGISTER_GRAPH_NODE(MyCustomNode, "My Custom Node", "Custom", "Material", glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
```

### Option B: Register with multiple domains

Use `REGISTER_GRAPH_NODE_MULTI` to register a node across multiple domains:

```cpp
DEFINE_GRAPH_NODE(MyCustomNode);
REGISTER_GRAPH_NODE_MULTI(MyCustomNode, "My Custom Node", "Custom", glm::vec4(0.8f, 0.4f, 0.1f, 1.0f), "Material", "Shader");
```

### Option C: Register inside a Domain class

Add the node directly in the domain's `RegisterNodeTypes()`:

```cpp
void MyDomain::RegisterNodeTypes()
{
    AddNodeType(MyCustomNode::GetStaticType(), "My Custom Node", "Custom", glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
}
```

## Step 4: Force Linkage

Add `FORCE_LINK_CALL(MyCustomNodes)` to `Engine.cpp`'s `ForceLinkage()` function to ensure the linker includes your node file.

## Step 5: Add to Project

Add the `.h` and `.cpp` files to `Engine.vcxproj` and `Engine.vcxproj.filters`.

## Available Data Types

| DatumType | C++ Type | Access Method |
|---|---|---|
| `DatumType::Float` | `float` | `GetFloat()` |
| `DatumType::Integer` | `int32_t` | `GetInteger()` |
| `DatumType::Bool` | `bool` | `GetBool()` |
| `DatumType::Vector` | `glm::vec3` | `GetVector()` |
| `DatumType::Vector2D` | `glm::vec2` | `GetVector2D()` |
| `DatumType::Color` | `glm::vec4` | `GetColor()` |

## Pin Best Practices

- Always provide default values for input pins so the node works without connections.
- Use meaningful pin names that describe the data.
- Output-only nodes (sources) like `TimeNode` have no input pins.
- Input-only nodes (sinks) like output nodes have no output pins.
- Each input pin can only have one incoming link (the most recent link replaces any existing one).
- Output pins can connect to multiple input pins.
