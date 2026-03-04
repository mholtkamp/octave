# Core Architecture

## Overview

The engine core lives in `Engine/Source/Engine/`. It provides the object model, RTTI, property reflection, factory system, serialization, logging, and the main initialization/update loop.

## Engine Lifecycle

**File:** `Engine/Source/Engine/Engine.h` / `Engine.cpp`

```
Initialize()
  ├── InitializeLog()
  ├── CreateProfiler()
  ├── Renderer::Create()
  ├── AssetManager::Create()
  ├── NetworkManager::Create()
  ├── EditorImguiInit()          [#if EDITOR]
  ├── SYS_Initialize()           [platform-specific]
  ├── AssetManager::Get()->Initialize()
  ├── LoadProject(path)
  └── AssetManager::Get()->Discover()

Update()  [called each frame]
  ├── OctPreUpdate()
  ├── World tick (physics, scripts, nodes)
  ├── OctPostUpdate()
  └── EditorImguiDraw()          [#if EDITOR]

Shutdown()
  ├── EditorImguiPreShutdown()   [#if EDITOR]
  ├── World cleanup
  └── Subsystem Destroy() calls
```

External hooks (defined by the game, not the engine):
- `OctPreInitialize(EngineConfig&)`, `OctPostInitialize()`
- `OctPreUpdate()`, `OctPostUpdate()`
- `OctPreShutdown()`, `OctPostShutdown()`

## Object System (RTTI)

**Files:** `Engine/Source/Engine/Object.h/.cpp`

`Object` is the base class providing runtime type identification and property reflection.

```cpp
class Object {
    virtual TypeId GetType() const;
    virtual const char* GetTypeName() const;
    bool Is(TypeId typeId) const;
    template<typename T> T* As();                    // Safe downcast
    Object* QueryInterface(TypeId typeId);
    virtual void GatherProperties(std::vector<Property>& outProps);
};
```

## RTTI Macros

Every node and asset class uses declaration/definition macros:

```cpp
// In header:
class Box3D : public Primitive3D {
    DECLARE_NODE(Box3D, Primitive3D);   // Adds RTTI, factory, script link
    // ...
};

// In source:
DEFINE_NODE(Box3D, Primitive3D);
```

`DECLARE_NODE` expands to: `DECLARE_FACTORY(Base, Node)` + `DECLARE_OBJECT(Base, Parent)` + `DECLARE_SCRIPT_LINK(Base, Parent, Node)` + `typedef Parent Super;`

Similarly: `DECLARE_ASSET(Class, Parent)` / `DEFINE_ASSET(Class)` for assets.

## Factory Pattern

**Files:** `Engine/Source/Engine/Factory.h/.cpp`

Types self-register at static init. Create instances by type ID or name:

```cpp
Node* node = Node::Construct(typeId);
Node* node = Node::Construct("Box3D");
Asset* asset = Asset::Factory("Material");
```

## Property System

**Files:** `Engine/Source/Engine/Property.h/.cpp`

Properties enable editor inspection, serialization, and Lua access. Nodes override `GatherProperties()` to expose their fields:

```cpp
void Box3D::GatherProperties(std::vector<Property>& outProps) {
    Primitive3D::GatherProperties(outProps);
    outProps.push_back(Property("Extents", DatumType::Vector, &mExtents));
}
```

## Singleton Pattern

Most managers use static Create/Destroy/Get:

```cpp
class Renderer {
    static void Create();
    static void Destroy();
    static Renderer* Get();
protected:
    static Renderer* sInstance;
};
```

## Logging

**Files:** `Engine/Source/Engine/Log.h/.cpp`

```cpp
LogDebug("message %d", value);
LogWarning("message");
LogError("message");
```

Thread-safe via `LockLog()` / `UnlockLog()` using `sMutex`. Callbacks: `RegisterLogCallback(LogCallbackFP)` / `UnregisterLogCallback(LogCallbackFP)`.

`LogSeverity` enum: `Debug`, `Warning`, `Error`, `Count` (in `System/SystemTypes.h`).

## Serialization

**Files:** `Engine/Source/Engine/Stream.h/.cpp`

Binary stream with read/write for all primitive and math types. Supports file I/O, async loading, and asset version tracking. See [AssetSystem.md](AssetSystem.md).

## World

**Files:** `Engine/Source/Engine/World.h/.cpp`

Contains the node tree, physics world (Bullet), and per-frame update logic:

```cpp
World* world = GetWorld(0);
Node* node = world->SpawnNode("Box3D");
Node* found = world->FindNode("PlayerNode");
Camera3D* cam = world->GetActiveCamera();
```

## Key Constants

**File:** `Engine/Source/Engine/Constants.h`

```cpp
LOGGING_ENABLED  1
CONSOLE_ENABLED  1
DEBUG_DRAW_ENABLED  1
LUA_ENABLED  1
EMBED_ALL_ASSETS  1  // Embed assets in executable for console builds
```

## Related Files

| File | Purpose |
|------|---------|
| `Engine.h/.cpp` | Main engine interface and lifecycle |
| `Object.h/.cpp` | RTTI base class |
| `Factory.h/.cpp` | Type factory and registration |
| `Property.h/.cpp` | Property reflection system |
| `Stream.h/.cpp` | Binary serialization |
| `Log.h/.cpp` | Logging with callbacks |
| `World.h/.cpp` | Scene graph and physics world |
| `Constants.h` | Compile-time feature toggles |
| `EngineTypes.h` | EngineConfig, EngineState structs |
| `Script.h/.cpp` | Lua script component |
| `ScriptMacros.h` | Script binding macros |
| `ScriptAutoReg.h` | Auto-registration for Lua functions |
