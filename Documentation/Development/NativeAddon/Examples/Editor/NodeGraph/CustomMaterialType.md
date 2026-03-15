# Custom Material Type with Node Graph Integration

This example shows how to create a custom "Toon Material" type that embeds a node graph for visual parameter authoring. Double-clicking the material in the editor automatically opens the graph editor.

## 1. Define the Toon Output Node

```cpp
// ToonOutputNode.h
#pragma once
#include "NodeGraph/GraphNode.h"

class ToonOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ToonOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Toon Output"; }
    virtual const char* GetNodeCategory() const override { return "Toon"; }
    virtual glm::vec4 GetNodeColor() const override { return glm::vec4(0.8f, 0.3f, 0.6f, 1.0f); }
};
```

```cpp
// ToonOutputNode.cpp
#include "ToonOutputNode.h"
#include "Utilities.h"

DEFINE_GRAPH_NODE(ToonOutputNode);

void ToonOutputNode::SetupPins()
{
    AddInputPin("Base Color",    DatumType::Color, Datum(glm::vec4(1.0f)));
    AddInputPin("Shadow Color",  DatumType::Color, Datum(glm::vec4(0.2f, 0.1f, 0.3f, 1.0f)));
    AddInputPin("Outline Width", DatumType::Float, Datum(2.0f));
    AddInputPin("Rim Power",     DatumType::Float, Datum(3.0f));
    AddInputPin("Steps",         DatumType::Float, Datum(3.0f));
}

void ToonOutputNode::Evaluate()
{
    // Sink node - values are read after evaluation
}
```

## 2. Define the Toon Domain

```cpp
// ToonDomain.h
#pragma once
#include "NodeGraph/GraphDomain.h"

class ToonDomain : public GraphDomain
{
public:
    virtual const char* GetDomainName() const override { return "Toon"; }
    virtual void RegisterNodeTypes() override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
```

```cpp
// ToonDomain.cpp
#include "ToonDomain.h"
#include "ToonOutputNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/InputNodes.h"

void ToonDomain::RegisterNodeTypes()
{
    // Shared nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(ColorConstantNode::GetStaticType(), "Color", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpNode::GetStaticType(), "Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Toon output
    AddNodeType(ToonOutputNode::GetStaticType(), "Toon Output", "Toon", glm::vec4(0.8f, 0.3f, 0.6f, 1.0f));
}

TypeId ToonDomain::GetDefaultOutputNodeType() const
{
    return ToonOutputNode::GetStaticType();
}
```

## 3. Define the Toon Material

```cpp
// ToonMaterial.h
#pragma once
#include "Assets/Material.h"
#include "NodeGraph/NodeGraph.h"

struct ToonMaterialParams
{
    glm::vec4 mBaseColor = glm::vec4(1.0f);
    glm::vec4 mShadowColor = glm::vec4(0.2f, 0.1f, 0.3f, 1.0f);
    float mOutlineWidth = 2.0f;
    float mRimPower = 3.0f;
    float mSteps = 3.0f;
};

class ToonMaterial : public Material
{
public:
    DECLARE_ASSET(ToonMaterial, Material);

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual const char* GetTypeName() override { return "ToonMaterial"; }

    // Node graph integration
    virtual bool HasNodeGraph() const override { return mUseNodeGraph; }
    virtual NodeGraph* GetNodeGraph() override { return &mGraph; }
    virtual void ApplyGraphValues(NodeGraph* graph) override;

    void SetUseNodeGraph(bool use);

protected:
    ToonMaterialParams mParams;
    NodeGraph mGraph;
    bool mUseNodeGraph = false;
};
```

```cpp
// ToonMaterial.cpp
#include "ToonMaterial.h"
#include "ToonOutputNode.h"

FORCE_LINK_DEF(ToonMaterial);
DEFINE_ASSET(ToonMaterial);

void ToonMaterial::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);
    // Load params...
    mUseNodeGraph = stream.ReadBool();
    if (mUseNodeGraph)
    {
        mGraph.LoadStream(stream, mVersion);
    }
}

void ToonMaterial::SaveStream(Stream& stream, Platform platform)
{
    Material::SaveStream(stream, platform);
    // Save params...
    stream.WriteBool(mUseNodeGraph);
    if (mUseNodeGraph)
    {
        mGraph.SaveStream(stream);
    }
}

void ToonMaterial::Create()
{
    Material::Create();
}

void ToonMaterial::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Bool, "Use Node Graph", this, &mUseNodeGraph));
    outProps.push_back(Property(DatumType::Color, "Base Color", this, &mParams.mBaseColor));
    outProps.push_back(Property(DatumType::Color, "Shadow Color", this, &mParams.mShadowColor));
    outProps.push_back(Property(DatumType::Float, "Outline Width", this, &mParams.mOutlineWidth));
    outProps.push_back(Property(DatumType::Float, "Rim Power", this, &mParams.mRimPower));
    outProps.push_back(Property(DatumType::Float, "Steps", this, &mParams.mSteps));
}

void ToonMaterial::SetUseNodeGraph(bool use)
{
    mUseNodeGraph = use;
    if (mUseNodeGraph && mGraph.GetNumNodes() == 0)
    {
        mGraph.SetDomainName("Toon");
        mGraph.AddNode(ToonOutputNode::GetStaticType());
    }
}

void ToonMaterial::ApplyGraphValues(NodeGraph* graph)
{
    GraphNode* output = graph->FindOutputNode();
    if (output == nullptr) return;

    mParams.mBaseColor    = output->GetInputValue(0).GetColor();
    mParams.mShadowColor  = output->GetInputValue(1).GetColor();
    mParams.mOutlineWidth = output->GetInputValue(2).GetFloat();
    mParams.mRimPower     = output->GetInputValue(3).GetFloat();
    mParams.mSteps        = output->GetInputValue(4).GetFloat();
}
```

## 4. Register the Domain

In your Engine startup (e.g., `Engine.cpp`):

```cpp
#include "ToonDomain.h"

// In initialization code:
GraphDomainManager::Get()->RegisterDomain(new ToonDomain());
```

## 5. How It Works

- The `Material` base class provides virtual hooks: `HasNodeGraph()`, `GetNodeGraph()`, `ApplyGraphValues()`
- `ToonMaterial` overrides all three to wire up its embedded `NodeGraph`
- The editor's double-click handler in `EditorImgui.cpp` checks `material->HasNodeGraph()` on any `Material` subclass
- If true, it calls `OpenNodeGraphForEditing(mat->GetNodeGraph(), mat)` to open the graph editor
- When Preview is enabled, the editor evaluates the graph and calls `mat->ApplyGraphValues()` to write computed values back to the material parameters
- No additional editor code is needed for new material types
