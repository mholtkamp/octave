# Asset System

## Overview

Assets are serializable game data (textures, meshes, materials, scenes, etc.). The system provides versioned binary serialization, UUID-based identification, async loading, reference counting, and a factory-based type registry.

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Source/Engine/Asset.h/.cpp` | Base asset class |
| `Engine/Source/Engine/AssetManager.h/.cpp` | Registry, loading, discovery |
| `Engine/Source/Engine/AssetRef.h` | Reference wrapper (typedef aliases) |
| `Engine/Source/Engine/Stream.h/.cpp` | Binary serialization |
| `Engine/Source/Engine/Assets/*.h/.cpp` | Asset type implementations |

## Asset Base Class

```cpp
class Asset : public Object {
    DECLARE_ASSET(Asset, Object);

    // Serialization
    virtual void LoadStream(Stream& stream, Platform platform);
    virtual void SaveStream(Stream& stream, Platform platform);
    virtual bool Import(const std::string& path, ImportOptions* options = nullptr);

    // Lifecycle
    virtual void Create();
    virtual void Destroy();
    virtual void Copy(Asset* srcAsset);

    // File I/O
    void LoadFile(const char* path, AsyncLoadRequest* request = nullptr);
    void SaveFile(const char* path, Platform platform);

    // Identity
    const std::string& GetName() const;
    uint64_t GetUuid() const;
    void EnsureUuid();       // Generate if not assigned
    bool IsLoaded() const;

    // Editor
    void SetDirtyFlag();     // Mark as modified (EDITOR only)
    bool GetDirtyFlag();

    // Properties
    virtual void GatherProperties(std::vector<Property>& outProps);
};
```

Macros: `DECLARE_ASSET(Class, Parent)` / `DEFINE_ASSET(Class)`.

## Asset Types (15)

All in `Engine/Source/Engine/Assets/`:

| Type | File | Description |
|------|------|-------------|
| `Texture` | `Texture.h/.cpp` | Images with mipmaps, formats, filter/wrap modes |
| `StaticMesh` | `StaticMesh.h/.cpp` | Static geometry with baked lighting support |
| `SkeletalMesh` | `SkeletalMesh.h/.cpp` | Skinned mesh with skeleton/bones |
| `Material` | `Material.h/.cpp` | Full material with shader parameters |
| `MaterialBase` | `MaterialBase.h/.cpp` | Abstract material base |
| `MaterialLite` | `MaterialLite.h/.cpp` | Lightweight runtime material |
| `MaterialInstance` | `MaterialInstance.h/.cpp` | Material with overrides from a base |
| `ParticleSystem` | `ParticleSystem.h/.cpp` | Particle emitter configuration |
| `Font` | `Font.h/.cpp` | TTF/OTF fonts with glyph atlases |
| `SoundWave` | `SoundWave.h/.cpp` | Audio data (Vorbis compressed) |
| `Scene` | `Scene.h/.cpp` | Serialized node hierarchy |
| `Timeline` | `Timeline.h/.cpp` | Keyframe animation sequences |
| `NodeGraphAsset` | `NodeGraphAsset.h/.cpp` | Visual scripting graphs |
| `DataAsset` | `DataAsset.h/.cpp` | Lua-defined data containers (ScriptableObject equivalent) |

## AssetManager

**Singleton:** `AssetManager::Get()`

Key operations:

```cpp
// Discovery
void Initialize();
void Discover(directoryName, directoryPath);
void DiscoverEmbeddedAssets(assets, count);

// Loading (UUID-based, preferred)
Asset* LoadAssetByUuid(uint64_t uuid);
void AsyncLoadAssetByUuid(uint64_t uuid, AssetRef* targetRef);
AssetStub* GetAssetStubByUuid(uint64_t uuid);

// Loading (name-based, backward compatible)
Asset* LoadAsset(const std::string& name);
Asset* GetAsset(const std::string& name);
void AsyncLoadAsset(const std::string& name, AssetRef* targetRef);

// Loading (path-based)
Asset* LoadAssetByPath(const std::string& path);  // "Assets/Models/SM_Plane"

// Utilities
bool DoesAssetExist(name);
void SaveAsset(name);
bool RenameAsset(asset, newName);
void Purge(purgeEngineAssets);  // Unload unreferenced
void RefSweep();                // Garbage collect
```

Global helpers: `FetchAsset(name)`, `LoadAsset(name)`, `FetchAssetByUuid(uuid)`, `LoadAssetByUuid(uuid)`.

Template helpers: `FetchAsset<Texture>(name)`, `LoadAsset<Material>(name)`.

## AssetRef (Reference Pattern)

**File:** `Engine/Source/Engine/AssetRef.h`

Lightweight wrapper around `Asset*` with async load support:

```cpp
AssetRef ref;
ref = LoadAsset("MyTexture");
Texture* tex = ref.Get<Texture>();
```

Typedefs: `TextureRef`, `StaticMeshRef`, `MaterialRef`, `SkeletalMeshRef`, `ParticleSystemRef`, `SoundWaveRef`, `FontRef`, `SceneRef`, `TimelineRef`, `DataAssetRef`.

## AssetStub

Metadata entry in the asset registry:
```cpp
struct AssetStub {
    Asset* mAsset;           // nullptr until loaded
    std::string mPath;       // File path
    TypeId mType;            // Asset class type
    uint64_t mUuid;          // Primary identifier (v12+)
    bool mEngineAsset;       // Built-in engine asset?
    // EDITOR: mName, mDirectory
};
```

## Stream (Serialization)

**File:** `Engine/Source/Engine/Stream.h/.cpp`

Binary stream with methods for all types:
- Primitives: `ReadInt32()`, `WriteFloat()`, `ReadBool()`, etc.
- Math: `ReadVec3()`, `WriteQuat()`, `ReadMatrix()`, etc.
- Strings: `ReadString()`, `WriteString()` (4-byte length prefix)
- Assets: `ReadAsset(AssetRef&)`, `WriteAsset(AssetRef&)` (UUID-based)
- File I/O: `ReadFile(path)`, `WriteFile(path)`
- Async: `SetAsyncRequest()`, `SetAssetVersion()`

## Asset Header

```cpp
struct AssetHeader {
    uint32_t mMagic = 0x4f435421;          // "OCT!"
    uint32_t mVersion = ASSET_VERSION_CURRENT;
    TypeId mType;
    uint8_t mEmbedded;
    uint64_t mUuid;                         // v12+
};
```

## Version History

| Version | Constant | Changes |
|---------|----------|---------|
| 1 | `ASSET_VERSION_BASE` | Initial format |
| 2 | `ASSET_VERSION_SCENE_EXTRA_DATA` | Scene extra data |
| 3 | `ASSET_VERSION_PARTICLE_RADIAL_SPAWN` | Particle radial spawning |
| 4 | `ASSET_VERSION_PROPERTY_EXTRA` | Extended property data |
| 5 | `ASSET_VERSION_SCENE_SUB_SCENE_OVERRIDE` | Sub-scene overrides |
| 6 | `ASSET_VERSION_FONT_TTF_FLAG` | TTF support flag |
| 7 | `ASSET_VERSION_TEXTURE_COOKED_PROPERTIES` | Texture cook properties |
| 8 | `ASSET_VERSION_TEXTURE_LOW_QUALITY` | Low-quality textures |
| 9 | `ASSET_VERSION_MATERIAL_LITE_TEXTURE_COUNT` | MaterialLite textures |
| 10 | `ASSET_VERSION_STATIC_MESH_3D_HAS_BAKED_LIGHTING` | Baked lighting |
| 11 | `ASSET_VERSION_SCENE_SUBSCENE_INSTANCE_COLORS` | Sub-scene colors |
| 12 | `ASSET_VERSION_UUID_SUPPORT` | UUID-based asset IDs |
| 13 | `ASSET_VERSION_UUID_WITH_NAME_FALLBACK` | UUID + name fallback |
| 14 | `ASSET_VERSION_NODE_PERSISTENT_UUID` | Persistent node UUIDs |
| 15 | `ASSET_VERSION_MATERIAL_LITE_NODE_GRAPH` | MaterialLite node graphs |
| 16 | `ASSET_VERSION_NODE_GRAPH_FUNCTIONS` | Named function subgraphs |
| 17 | `ASSET_VERSION_NODE_GRAPH_VARIABLES` | Local named variables, copy/paste/export |

**Current:** `ASSET_VERSION_CURRENT = 17`

## Async Loading

Assets can be loaded asynchronously via `AsyncLoadAsset()`. The `AsyncLoadRequest` tracks the load state. `AssetRef::GetLoadRequest()` checks pending loads. Load states: `Unloaded → AwaitBegin → AwaitEnd → Loaded`.
