# Creating Custom Domains

This guide explains how to create a new graph domain to add a custom graph type to the node graph system.

## Step 1: Define the Domain Class

Create a header file in `Engine/Source/Engine/NodeGraph/Domains/`.

```cpp
// MyDomain.h
#pragma once

#include "NodeGraph/GraphDomain.h"

class MyDomain : public GraphDomain
{
public:
    virtual const char* GetDomainName() const override { return "MyDomain"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
```

## Step 2: Implement the Domain

Create the corresponding .cpp file.

```cpp
// MyDomain.cpp
#include "NodeGraph/Domains/MyDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/MyOutputNode.h"  // Your output node

void MyDomain::RegisterNodeTypes()
{
    // Register shared nodes that should be available in this domain
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Register domain-specific nodes
    AddNodeType(MyOutputNode::GetStaticType(), "My Output", "MyDomain", glm::vec4(0.5f, 0.3f, 0.8f, 1.0f));
}

TypeId MyDomain::GetDefaultOutputNodeType() const
{
    return MyOutputNode::GetStaticType();
}

void MyDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // Read output node's input pin values and integrate with your system.
    // This is called after every graph evaluation.
}
```

## Step 3: Create the Output Node

Every domain should have an output (sink) node that collects the final computed values.

```cpp
// MyOutputNode.h
class MyOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MyOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "My Output"; }
    virtual const char* GetNodeCategory() const override { return "MyDomain"; }
    virtual glm::vec4 GetNodeColor() const override;
};
```

## Step 4: Register the Domain

In `Engine.cpp`, add the include and register the domain:

```cpp
#include "NodeGraph/Domains/MyDomain.h"

// In engine initialization:
GraphDomainManager::Get()->RegisterDomain(new MyDomain());
```

The domain must be registered **before** `ProcessExternalRegistrations()` is called.

## Step 5: Update Project Files

Add all new `.h` and `.cpp` files to `Engine.vcxproj` and `Engine.vcxproj.filters`.

## Domain Design Guidelines

### Choosing Which Shared Nodes to Include
- **Math-heavy domains** (Material, Shader, Procedural): Include all Math nodes and all Value nodes.
- **Logic-heavy domains** (FSM): Include only Float, Integer, and Viewer — skip Math nodes if they don't apply.
- **Transform-heavy domains** (SceneGraph): Include Add, Subtract, Multiply, and Vector/Float values.

### OnGraphEvaluated Callback
Use this to read the output node's computed values after graph evaluation:

```cpp
void MyDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // Find the output node
    for (uint32_t i = 0; i < graph->GetNumNodes(); ++i)
    {
        GraphNode* node = graph->GetNodes()[i];
        if (node->GetType() == MyOutputNode::GetStaticType())
        {
            // Read the computed input values
            float value = node->GetInputValue(0).GetFloat();
            // Apply to your system...
            break;
        }
    }
}
```

### GetDefaultOutputNodeType
Return the TypeId of your output node. This is used by `NodeGraphAsset::Create()` to automatically place an output node when a new graph of this type is created.
