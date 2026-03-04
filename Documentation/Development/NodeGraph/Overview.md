# Node Graph System Overview

## Architecture

The Octave node graph system provides a visual programming interface for connecting data-producing nodes to data-consuming nodes across multiple domains. It is designed to be domain-agnostic at its core, with specialized behavior provided by domain classes.

```
┌─────────────────────────────────────────────────────────┐
│                   NodeGraphAsset                        │
│  ┌───────────────────────────────────────────────────┐  │
│  │                   NodeGraph                       │  │
│  │  ┌──────────┐    ┌──────────┐    ┌────────────┐  │  │
│  │  │ GraphNode│───>│ GraphNode│───>│ GraphNode  │  │  │
│  │  │ (Source) │    │ (Process)│    │ (Output)   │  │  │
│  │  └──────────┘    └──────────┘    └────────────┘  │  │
│  │       │               │               │          │  │
│  │  ┌────┴────┐    ┌────┴────┐    ┌─────┴─────┐    │  │
│  │  │  Pins   │    │  Pins   │    │   Pins    │    │  │
│  │  │Out: val │────│In: A    │    │In: result │    │  │
│  │  └─────────┘    │Out: res │────│           │    │  │
│  │                 └─────────┘    └───────────┘    │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  Domain: "Material" | "Shader" | "Procedural" | ...     │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### GraphNode
Base class for all nodes. Each node has:
- **Input pins** - Receive data from other nodes
- **Output pins** - Send data to other nodes
- **SetupPins()** - Called after construction to define the node's interface
- **Evaluate()** - Called during graph evaluation to compute output values

### GraphPin
A connection point on a node. Each pin has:
- A unique ID
- A name and data type (`DatumType`)
- A direction (Input or Output)
- A default value and a current value

### GraphLink
Connects an output pin to an input pin. Links are validated for type compatibility (Float↔Integer and Vector↔Color conversions are supported).

### NodeGraph
Container for nodes and links. Manages:
- Node CRUD operations (Add, Remove, Find)
- Link creation with cycle detection
- Serialization (save/load via streams)
- Domain name assignment

### GraphDomain
Abstract base class defining a graph context. Each domain:
- Declares available node types via `RegisterNodeTypes()`
- Specifies a default output node via `GetDefaultOutputNodeType()`
- Receives callbacks after evaluation via `OnGraphEvaluated()`

### GraphDomainManager
Singleton registry for all domains. Manages:
- Domain registration at engine startup
- External node registration via `REGISTER_GRAPH_NODE` macros
- Domain lookup by name

### GraphProcessor
Evaluates a node graph by:
1. Running topological sort to determine evaluation order
2. Detecting cycles (evaluation aborts if cycles exist)
3. Propagating values through links after each node evaluates

## Evaluation Flow

```
1. GraphProcessor::Evaluate(graph)
2. TopologicalSort() - Kahn's algorithm
3. For each node in sorted order:
   a. Reset input pins to default values
   b. Propagate output values from connected nodes
   c. Call node->Evaluate()
4. Domain->OnGraphEvaluated(graph)
```

## Serialization

Node graphs are serialized as part of `NodeGraphAsset`. The format includes:
- Domain name string
- All nodes (type ID, position, pin data)
- All links (output pin → input pin)
- ID counters for nodes, links, and pins

## Supported Data Types

| DatumType | Description | Compatible With |
|---|---|---|
| Float | Single float value | Integer |
| Integer | 32-bit signed integer | Float |
| Bool | Boolean true/false | - |
| Vector | 3D vector (glm::vec3) | Color |
| Vector2D | 2D vector (glm::vec2) | - |
| Color | RGBA color (glm::vec4) | Vector |

## Available Graph Types

See [GraphTypes.md](GraphTypes.md) for a complete listing of all 6 supported graph domains.
