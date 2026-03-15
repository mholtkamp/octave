# Domain Callbacks Example

## Overview

This example demonstrates how to use the `OnGraphEvaluated()` callback to read computed values from a node graph's output node and integrate them with game systems. This is the primary mechanism for bridging the visual graph editor to runtime behavior.

## Files

### MaterialDomain OnGraphEvaluated (Built-in Example)

The `MaterialDomain` reads material parameters after evaluation:

```cpp
void MaterialDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // Find the MaterialOutputNode
    for (uint32_t i = 0; i < graph->GetNumNodes(); ++i)
    {
        GraphNode* node = graph->GetNodes()[i];
        if (node->GetType() == MaterialOutputNode::GetStaticType())
        {
            // Read computed material parameters from input pins
            glm::vec4 baseColor = node->GetInputValue(0).GetColor();
            float metallic = node->GetInputValue(1).GetFloat();
            float roughness = node->GetInputValue(2).GetFloat();
            glm::vec3 normal = node->GetInputValue(3).GetVector();
            glm::vec4 emissive = node->GetInputValue(4).GetColor();
            float opacity = node->GetInputValue(5).GetFloat();

            // Apply to the material system
            // myMaterial->SetBaseColor(baseColor);
            // myMaterial->SetMetallic(metallic);
            // etc.
            break;
        }
    }
}
```

### Procedural Domain Callback Example

Reading procedural generation parameters to generate terrain:

```cpp
void ProceduralDomain::OnGraphEvaluated(NodeGraph* graph)
{
    for (uint32_t i = 0; i < graph->GetNumNodes(); ++i)
    {
        GraphNode* node = graph->GetNodes()[i];
        if (node->GetType() == ProceduralOutputNode::GetStaticType())
        {
            float height = node->GetInputValue(0).GetFloat();
            float density = node->GetInputValue(1).GetFloat();
            glm::vec4 color = node->GetInputValue(2).GetColor();
            float mask = node->GetInputValue(3).GetFloat();

            // Use these values to drive terrain generation
            // TerrainGenerator::SetHeight(height);
            // TerrainGenerator::SetDensity(density);
            break;
        }
    }
}
```

### FSM Domain Callback Example

Reading FSM state to control game logic:

```cpp
void FSMDomain::OnGraphEvaluated(NodeGraph* graph)
{
    for (uint32_t i = 0; i < graph->GetNumNodes(); ++i)
    {
        GraphNode* node = graph->GetNodes()[i];
        if (node->GetType() == FSMOutputNode::GetStaticType())
        {
            int32_t stateId = node->GetInputValue(0).GetInteger();
            bool shouldTransition = node->GetInputValue(1).GetBool();

            // Drive game state machine
            // if (shouldTransition)
            //     gameStateMachine->TransitionTo(stateId);
            break;
        }
    }
}
```

## How Graph Evaluation Works

The evaluation pipeline:

```
1. GraphProcessor::Evaluate(graph) is called
2. Nodes are topologically sorted (Kahn's algorithm)
3. For each node in evaluation order:
   a. Input pin values are reset to defaults
   b. Connected output values are propagated to input pins
   c. node->Evaluate() is called
4. domain->OnGraphEvaluated(graph) is called
```

After step 4, all output node input pins contain their final computed values.

## API Reference

### GraphNode Input Access

| Method | Description |
|---|---|
| `GetInputValue(uint32_t index)` | Returns the Datum value at the given input pin index |
| `GetInputPin(uint32_t index)` | Returns a pointer to the input GraphPin |
| `GetNumInputPins()` | Returns the number of input pins |

### Datum Value Accessors

| Method | Return Type | Description |
|---|---|---|
| `GetFloat()` | `float` | Get float value |
| `GetInteger()` | `int32_t` | Get integer value |
| `GetBool()` | `bool` | Get boolean value |
| `GetVector()` | `glm::vec3` | Get 3D vector |
| `GetVector2D()` | `glm::vec2` | Get 2D vector |
| `GetColor()` | `glm::vec4` | Get RGBA color |

## Best Practices

- Always search for the output node by type rather than assuming its position in the node list.
- Use `break` after finding the output node to avoid unnecessary iteration.
- The `OnGraphEvaluated()` callback fires after every evaluation — keep it lightweight.
- Input pin indices correspond to the order pins were added in `SetupPins()`.
- If the output node has no connections, input pins will contain their default values.
