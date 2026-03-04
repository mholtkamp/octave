# Node System

## Overview

Nodes are the fundamental building blocks of scenes. Every object in the game world is a `Node` arranged in a parent-child hierarchy within a `World`. The node system provides 3D scene objects, 2D widgets, and specialized types like lights, cameras, and mesh renderers.

## Class Hierarchy

```
Object
└── Node                         (Engine/Source/Engine/Nodes/Node.h)
    ├── Node3D                   (Engine/Source/Engine/Nodes/3D/)
    │   ├── Primitive3D          — Physics-enabled 3D shapes
    │   │   ├── Box3D
    │   │   ├── Sphere3D
    │   │   ├── Capsule3D
    │   │   └── ...
    │   ├── Mesh3D               — Base for mesh rendering
    │   │   ├── StaticMesh3D
    │   │   ├── SkeletalMesh3D
    │   │   ├── InstancedMesh3D
    │   │   ├── ShadowMesh3D
    │   │   └── TextMesh3D
    │   ├── Camera3D
    │   ├── Light3D
    │   │   ├── DirectionalLight3D
    │   │   ├── PointLight3D
    │   │   └── SpotLight3D
    │   ├── Particle3D
    │   ├── Audio3D
    │   ├── Spline3D
    │   ├── TimelinePlayer       (Engine/Source/Engine/Nodes/TimelinePlayer.h)
    │   └── NodeGraphPlayer      (Engine/Source/Engine/Nodes/NodeGraphPlayer.h)
    └── Widget                   (Engine/Source/Engine/Nodes/Widgets/)
        ├── Canvas
        ├── Quad
        ├── Text
        ├── Button
        ├── Poly
        └── PolyRect
```

## Node Base Class

**File:** `Engine/Source/Engine/Nodes/Node.h/.cpp`

Key methods:
- **Lifecycle:** `Create()`, `Destroy()`, `Start()`, `Stop()`, `Tick(float deltaTime)`
- **Hierarchy:** `GetParent()`, `Attach(Node* parent)`, `GetChild(index)`, `GetNumChildren()`, `FindChild(name)`
- **Identity:** `GetName()`, `SetName()`, `GetUuid()`, `SetTag()`, `HasTag()`
- **State:** `SetActive(bool)`, `IsActive()`, `SetVisible(bool)`, `IsVisible()`
- **Scripts:** Each node can own a `Script` component for Lua scripting
- **RTTI:** `DECLARE_NODE(Class, Parent)` / `DEFINE_NODE(Class, Parent)`
- **Factory:** `Node::Construct(typeId)` or `Node::Construct("TypeName")`

## Node3D

**File:** `Engine/Source/Engine/Nodes/3D/Node3D.h/.cpp`

Adds transform (position, rotation, scale) in 3D space:
- `GetWorldPosition()`, `SetWorldPosition(vec3)`
- `GetWorldRotation()`, `SetWorldRotation(quat)`
- `GetWorldScale()`, `SetWorldScale(vec3)`
- Local transform equivalents: `GetPosition()`, `SetPosition()`, etc.
- `GetWorldTransform()` returns `glm::mat4`

## Key Node Types

| Type | Directory | Purpose |
|------|-----------|---------|
| `Primitive3D` | `Nodes/3D/` | Physics-enabled shapes (Box, Sphere, Capsule) with collision |
| `StaticMesh3D` | `Nodes/3D/` | Renders a StaticMesh asset with optional baked lighting |
| `SkeletalMesh3D` | `Nodes/3D/` | Skinned mesh with bone animation |
| `InstancedMesh3D` | `Nodes/3D/` | GPU-instanced mesh rendering |
| `Camera3D` | `Nodes/3D/` | Perspective/orthographic camera |
| `Light3D` | `Nodes/3D/` | Directional, point, and spot lights |
| `Particle3D` | `Nodes/3D/` | Particle system emitter |
| `Audio3D` | `Nodes/3D/` | Spatialized audio source |
| `TimelinePlayer` | `Nodes/` | Drives a Timeline asset (see [Timeline.md](Timeline.md)) |
| `NodeGraphPlayer` | `Nodes/` | Executes a NodeGraphAsset (see [NodeGraph.md](NodeGraph.md)) |
| `Widget` | `Nodes/Widgets/` | 2D UI elements (Canvas, Quad, Text, Button, Poly) |

## World Management

**File:** `Engine/Source/Engine/World.h/.cpp`

A `World` holds the root node tree, physics simulation (Bullet), and camera:

```cpp
World* world = GetWorld(0);
Node* root = world->GetRootNode();
Node* node = world->SpawnNode("Box3D", glm::vec3(0, 5, 0));
Node* node = world->SpawnScene("Level1");  // Instantiate a Scene asset
Node* found = world->FindNode("Player");
Camera3D* cam = world->GetActiveCamera();
```

Physics: `world->GetDynamicsWorld()` returns the Bullet `btDynamicsWorld*`.

## Node Ownership

Nodes are reference-counted via `NodePtr` (shared pointer). The World owns the root node; parent-child relationships form the ownership tree. `Node::Attach(parent)` transfers a node into the parent's child list.

## Scene Serialization

Scenes are assets (`Scene.h`) that serialize a node tree. Loading a scene reconstructs the hierarchy. Nodes have persistent UUIDs (v14+) for stable references across save/load.

## Related Files

| File | Purpose |
|------|---------|
| `Nodes/Node.h/.cpp` | Base node class |
| `Nodes/3D/Node3D.h/.cpp` | 3D transform node |
| `Nodes/3D/*.h/.cpp` | All 3D node types |
| `Nodes/Widgets/*.h/.cpp` | All widget types |
| `Nodes/NodeGraphPlayer.h/.cpp` | Node graph execution |
| `Nodes/TimelinePlayer.h/.cpp` | Timeline playback |
| `World.h/.cpp` | Scene graph and physics |
| `Assets/Scene.h/.cpp` | Scene asset (serialized node tree) |
