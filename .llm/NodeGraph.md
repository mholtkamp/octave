# Node Graph System

## Overview

The node graph system provides visual scripting through a directed acyclic graph (DAG) of nodes connected by typed pins. It supports multiple domains (material, shader, procedural, animation, FSM, scene graph) and a function/subgraph system for reusable logic.

## Architecture

```
NodeGraphAsset (Asset)           — Owns a main graph + function graphs
  └── NodeGraph                  — Container for nodes, links, pins
       ├── GraphNode instances   — Computation units
       ├── GraphLink instances   — Connections between pins
       └── GraphPin data         — Typed input/output ports

GraphProcessor                   — Evaluates a graph (topological sort + propagation)
GraphDomainManager               — Singleton managing all domains
  └── GraphDomain subclasses     — Define available node types per domain
```

## Key Files

| File | Purpose |
|------|---------|
| `Engine/NodeGraph/NodeGraph.h/.cpp` | Main graph container |
| `Engine/NodeGraph/GraphNode.h/.cpp` | Base node class with pins |
| `Engine/NodeGraph/GraphPin.h` | Pin structure (ID, type, value) |
| `Engine/NodeGraph/GraphLink.h` | Link structure (output pin → input pin) |
| `Engine/NodeGraph/GraphTypes.h` | ID typedefs, DatumType enum |
| `Engine/NodeGraph/GraphProcessor.h/.cpp` | Evaluation engine |
| `Engine/NodeGraph/GraphDomain.h/.cpp` | Domain base class |
| `Engine/NodeGraph/GraphDomainManager.h/.cpp` | Domain registry singleton |
| `Engine/NodeGraph/GraphVariable.h` | Variable struct (name, type, default/runtime value) |
| `Engine/NodeGraph/GraphClipboard.h/.cpp` | Copy/paste/export/import serialization |
| `Engine/Assets/NodeGraphAsset.h/.cpp` | Asset wrapper |
| `Engine/Nodes/NodeGraphPlayer.h/.cpp` | Runtime playback node |
| `Editor/NodeGraph/NodeGraphPanel.h/.cpp` | Editor UI panel |

All paths relative to `Engine/Source/`.

## GraphNode

Base class for all visual scripting nodes. Key virtual methods:

- `SetupPins()` — Create input/output pins (called once)
- `Evaluate()` — Read inputs, write outputs (called during graph evaluation)
- `TriggerExecutionPin(index)` — For flow control nodes
- `SaveStream()` / `LoadStream()` — Serialization
- `CopyCustomData(src)` — Deep copy for dynamic-pin nodes
- `GetNodeTypeName()`, `GetNodeCategory()`, `GetNodeColor()` — Metadata

Pin management: `AddInputPin(name, type, default)`, `AddOutputPin(name, type)`, `GetInputValue(index)`, `SetOutputValue(index, value)`.

Registration macros:
```cpp
DECLARE_GRAPH_NODE(MyNode, GraphNode)
DEFINE_GRAPH_NODE(MyNode)
REGISTER_GRAPH_NODE(MyNode, "My Node", "Category", "DomainName", color)
REGISTER_GRAPH_NODE_MULTI(MyNode, "My Node", "Category", color, "Domain1", "Domain2")
```

## Pin System

```cpp
struct GraphPin {
    GraphPinId mId;
    GraphNodeId mOwnerNodeId;
    std::string mName;
    DatumType mDataType;           // Float, Int, Bool, String, Vector, Color, Asset, Execution, ...
    GraphPinDirection mDirection;   // Input or Output
    Datum mDefaultValue;
    Datum mValue;
    bool mExecutionTriggered;      // For execution flow pins
};
```

**DatumType** enum includes: Integer, Float, Bool, String, Vector2D, Vector, Color, Asset, Byte, Table, Node, Node3D, Widget, Text, Quad, Audio3D, Scene, Execution, Function, Short.

## Domains

Located in `Engine/Source/Engine/NodeGraph/Domains/`:

| Domain | File | Default Output Node | Purpose |
|--------|------|---------------------|---------|
| Material | `MaterialDomain.h/.cpp` | MaterialOutputNode | Material shader graphs |
| Shader | `ShaderDomain.h/.cpp` | ShaderOutputNode | Vertex/fragment generation |
| Procedural | `ProceduralDomain.h/.cpp` | ProceduralOutputNode | Noise, gradients, patterns |
| Animation | `AnimationDomain.h/.cpp` | AnimationOutputNode | Animation blending |
| FSM | `FSMDomain.h/.cpp` | FSMOutputNode | Finite state machines |
| SceneGraph | `SceneGraphDomain.h/.cpp` | (none) | Gameplay event graphs |

Each domain registers its node types and provides an optional `OnGraphEvaluated()` callback.

## Node Types

Located in `Engine/Source/Engine/NodeGraph/Nodes/`:

| File | Count | Examples |
|------|-------|---------|
| `InputNodes.h/.cpp` | ~13 | FloatInputNode, IntInputNode, BoolInputNode, VectorInputNode |
| `ValueNodes.h/.cpp` | ~40+ | Constants, conversions, string ops, dictionary ops, datetime |
| `MathNodes.h/.cpp` | ~20+ | Add, Subtract, Multiply, Divide, Lerp, Clamp, Dot |
| `MaterialNodes.h/.cpp` | 5 | MaterialOutputNode, TextureSampleNode, FresnelNode, PannerNode |
| `ShaderNodes.h/.cpp` | 4 | ShaderOutputNode, VertexPositionNode, WorldPositionNode |
| `ProceduralNodes.h/.cpp` | 4 | ProceduralOutputNode, NoiseNode, VoronoiNode, GradientNode |
| `AnimationNodes.h/.cpp` | 4 | AnimationOutputNode, AnimClipNode, BlendNode, AnimSpeedNode |
| `FSMNodes.h/.cpp` | 4 | FSMOutputNode, StateNode, TransitionNode, ConditionNode |
| `SceneGraphNodes.h/.cpp` | 7 | StartEventNode, TickEventNode, StopEventNode, collision/overlap events |
| `FunctionNodes.h/.cpp` | 2 | FunctionOutputNode, FunctionCallNode |
| `VariableNodes.h/.cpp` | 2 | GetVariableNode, SetVariableNode |

Total: 100+ node types.

## Graph Processor (Evaluation)

**File:** `Engine/Source/Engine/NodeGraph/GraphProcessor.h/.cpp`

`Evaluate(NodeGraph* graph, const char* eventName = nullptr)`:
1. **Topological sort** — Kahn's algorithm, detects cycles
2. **Reset** — Clear execution flags, reset input pins to defaults
3. **Event triggering** — Find and trigger matching event nodes (if eventName given)
4. **Node evaluation** — Iterate in topological order, evaluate each node
5. **Value propagation** — Copy output values to connected input pins
6. **Execution propagation** — Flow execution pins to downstream nodes

**Execution modes:**
- **Data-flow**: Nodes with no execution pins always evaluate
- **Event-driven**: Event nodes trigger execution flow through execution pins
- **Hybrid**: Both coexist in the same graph

## Function/Subgraph System (v16)

**File:** `Engine/Source/Engine/NodeGraph/Nodes/FunctionNodes.h/.cpp`

NodeGraphAsset stores `mFunctionGraphs` (vector of `NodeGraph*`). Each has a `mGraphName` (empty = main graph).

**FunctionOutputNode**: Sink node in a function graph. Dynamic input pins define the function's return values. Overrides `LoadStream()` to pre-create pins before reading IDs.

**FunctionCallNode**: Calls a named function graph. Dynamic pins match the function's signature. Evaluation: copies inputs → evaluates temp graph copy → reads outputs.

Asset methods: `AddFunctionGraph(name)`, `RemoveFunctionGraph(index)`, `FindFunctionGraph(name)`, `ResolveFunctionCallNodes()`.

## Local Named Variables (v17)

**Files:** `Engine/Source/Engine/NodeGraph/GraphVariable.h`, `Engine/Source/Engine/NodeGraph/Nodes/VariableNodes.h/.cpp`

Variables are per-asset named state shared across main graph + all function subgraphs. Blueprint-style get/set nodes read/write at runtime.

```cpp
struct GraphVariable {
    std::string mName;
    DatumType mType;
    Datum mDefaultValue;
    Datum mRuntimeValue;  // reset to mDefaultValue on play
};
```

Supported types: Float, Integer, Bool, String, Vector2D, Vector, Color.

**GetVariableNode**: Pure data node with one output pin. Reads `mRuntimeValue` from the variable.

**SetVariableNode**: Flow node (Exec In/Out) with a value input and pass-through output. Writes to `mRuntimeValue`.

Both use `DEFINE_GRAPH_NODE` only (not registered in domain menus). Created dynamically from sidebar drag-drop or context menu.

Asset methods: `AddVariable(name, type)`, `RemoveVariable(index)`, `RenameVariable(index, newName)`, `FindVariableIndex(name)`, `GetVariable(index)`, `ResetVariablesToDefaults()`, `ResolveVariableNodes()`.

Runtime: `ResetVariablesToDefaults()` called when NodeGraphPlayer starts playing. Variable nodes in function subgraph temp copies are wired to the same owner asset in `FunctionCallNode::Evaluate()`.

Editor: Variables section in sidebar with type color indicators, inline default value editing, drag-drop (Get node) / Alt+drag-drop (Set node), right-click context menu (Rename/Change Type/Delete). Variables also appear in context menu "Variables" submenu and in search results.

## Copy/Paste + Import/Export (v17)

**Files:** `Engine/Source/Engine/NodeGraph/GraphClipboard.h/.cpp`

Binary clipboard format with magic `0x4F435447` ("OCTG"), base64-encoded for system clipboard.

**Namespace `GraphClipboard`:**
- `SerializeSelection(graph, nodeIds, stream)` — Writes selected nodes + internal links
- `DeserializeIntoGraph(graph, stream, position, asset)` — Creates nodes with remapped IDs, offsets positions, wires special nodes (FunctionCall/Variable) to owner asset
- `CopyToClipboard()` / `PasteFromClipboard()` — System clipboard via `SYS_SetClipboardText/GetClipboardText`
- `ExportToFile()` / `ImportFromFile()` — File-based sharing via `.octgraph` files

Helper methods on NodeGraph: `AllocNodeId()`, `AllocPinId()`, `AllocLinkId()`, `InsertNode(node)`.

Editor shortcuts: Ctrl+C (copy), Ctrl+V (paste), Ctrl+D (duplicate with +50,+50 offset). Toolbar: Export/Import buttons. Context menu: Copy on nodes, Paste/Import on background.

## NodeGraphPlayer (Runtime)

**File:** `Engine/Source/Engine/Nodes/NodeGraphPlayer.h/.cpp`

Attaches to a game node to run a NodeGraphAsset at runtime:

- `SetNodeGraphAsset(asset)`, `Play()`, `Pause()`, `StopGraph()`, `Reset()`
- Input: `SetInputFloat(name, value)`, `SetInputInt()`, `SetInputBool()`, etc.
- Output: `GetOutputFloat(index)`, `GetOutputVector(index)`, etc.
- Events: `FireEvent(eventName)` — triggers Start, Tick, Stop, physics events
- Physics: `BeginOverlap()`, `EndOverlap()`, `OnCollision()` fire corresponding event nodes

Tick flow: advance time → fire "Start" on first tick → process physics events → fire "Tick" → domain callback.

## Editor Panel

**File:** `Engine/Source/Editor/NodeGraph/NodeGraphPanel.h/.cpp`

Uses `imgui-node-editor` (ax::NodeEditor). Features: node creation/deletion/connection, function sidebar, variable sidebar with inline default editing, drag-drop for functions and variables, execution visualization (pin flashing), context menu with search, copy/paste/duplicate (Ctrl+C/V/D), export/import to `.octgraph` files.

Public API: `DrawNodeGraphContent()`, `OpenNodeGraphForEditing(NodeGraphAsset*)`, `CloseNodeGraphPanel()`.
