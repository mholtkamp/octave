# Custom Domain Example

## Overview

This example demonstrates how to create a custom "Audio" graph domain with its own output node. The domain registers shared Math/Value nodes plus domain-specific audio nodes.

## Files

### AudioDomain.h

```cpp
#pragma once

#include "NodeGraph/GraphDomain.h"

class AudioDomain : public GraphDomain
{
public:
    virtual const char* GetDomainName() const override { return "Audio"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
```

### AudioDomain.cpp

```cpp
#include "NodeGraph/Domains/AudioDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/AudioNodes.h"

void AudioDomain::RegisterNodeTypes()
{
    // Shared math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpNode::GetStaticType(), "Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ClampNode::GetStaticType(), "Clamp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Shared value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Audio-specific nodes
    AddNodeType(AudioOutputNode::GetStaticType(), "Audio Output", "Audio", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f));
    AddNodeType(OscillatorNode::GetStaticType(), "Oscillator", "Audio", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f));
    AddNodeType(AudioMixerNode::GetStaticType(), "Mixer", "Audio", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f));
}

TypeId AudioDomain::GetDefaultOutputNodeType() const
{
    return AudioOutputNode::GetStaticType();
}

void AudioDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // Find AudioOutputNode and read its input values
    for (uint32_t i = 0; i < graph->GetNumNodes(); ++i)
    {
        GraphNode* node = graph->GetNodes()[i];
        if (node->GetType() == AudioOutputNode::GetStaticType())
        {
            float volume = node->GetInputValue(0).GetFloat();
            float pitch = node->GetInputValue(1).GetFloat();
            // Apply to the audio system...
            break;
        }
    }
}
```

### AudioNodes.h

```cpp
#pragma once

#include "NodeGraph/GraphNode.h"

class AudioOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AudioOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Audio Output"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class OscillatorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(OscillatorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Oscillator"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class AudioMixerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AudioMixerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Mixer"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};
```

### AudioNodes.cpp

```cpp
#include "NodeGraph/Nodes/AudioNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(AudioNodes);

static const glm::vec4 kAudioNodeColor = glm::vec4(0.9f, 0.6f, 0.1f, 1.0f);

// AudioOutputNode
DEFINE_GRAPH_NODE(AudioOutputNode);

void AudioOutputNode::SetupPins()
{
    AddInputPin("Volume", DatumType::Float, Datum(1.0f));
    AddInputPin("Pitch", DatumType::Float, Datum(1.0f));
    AddInputPin("Pan", DatumType::Float, Datum(0.0f));
}

void AudioOutputNode::Evaluate() {}
glm::vec4 AudioOutputNode::GetNodeColor() const { return kAudioNodeColor; }

// OscillatorNode
DEFINE_GRAPH_NODE(OscillatorNode);

void OscillatorNode::SetupPins()
{
    AddInputPin("Frequency", DatumType::Float, Datum(440.0f));
    AddInputPin("Amplitude", DatumType::Float, Datum(1.0f));
    AddInputPin("Time", DatumType::Float, Datum(0.0f));
    AddOutputPin("Signal", DatumType::Float);
}

void OscillatorNode::Evaluate()
{
    float freq = GetInputValue(0).GetFloat();
    float amp = GetInputValue(1).GetFloat();
    float time = GetInputValue(2).GetFloat();
    float signal = amp * glm::sin(freq * time * 6.28318f);
    SetOutputValue(0, Datum(signal));
}

glm::vec4 OscillatorNode::GetNodeColor() const { return kAudioNodeColor; }

// AudioMixerNode
DEFINE_GRAPH_NODE(AudioMixerNode);

void AudioMixerNode::SetupPins()
{
    AddInputPin("Signal A", DatumType::Float, Datum(0.0f));
    AddInputPin("Signal B", DatumType::Float, Datum(0.0f));
    AddInputPin("Mix", DatumType::Float, Datum(0.5f));
    AddOutputPin("Output", DatumType::Float);
}

void AudioMixerNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    float mix = glm::clamp(GetInputValue(2).GetFloat(), 0.0f, 1.0f);
    SetOutputValue(0, Datum(glm::mix(a, b, mix)));
}

glm::vec4 AudioMixerNode::GetNodeColor() const { return kAudioNodeColor; }
```

### Engine.cpp Integration

```cpp
#include "NodeGraph/Domains/AudioDomain.h"

// In ForceLinkage():
FORCE_LINK_CALL(AudioNodes);

// In engine initialization (before ProcessExternalRegistrations):
GraphDomainManager::Get()->RegisterDomain(new AudioDomain());
```

## API Reference

### GraphDomain Virtual Methods

| Method | Description |
|---|---|
| `GetDomainName()` | Returns the domain's unique name string |
| `RegisterNodeTypes()` | Registers all node types available in this domain |
| `GetDefaultOutputNodeType()` | Returns the TypeId of the default output node |
| `OnGraphEvaluated(NodeGraph*)` | Called after graph evaluation completes |

### GraphDomain::AddNodeType(typeId, typeName, category, color)

Registers a node type with the domain. Duplicates are automatically skipped.

## Best Practices

- Register the domain before calling `ProcessExternalRegistrations()`.
- Always override `GetDefaultOutputNodeType()` so new graphs auto-create an output node.
- Include shared Math/Value nodes that make sense for your domain.
- Use `OnGraphEvaluated()` to bridge node graph results to your engine systems.
