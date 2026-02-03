# UUID-Based Asset Identification

## Problem
Asset names must be globally unique. Having `Models/SM_Plane.oct` and `Engine/Assets/Meshes/SM_Plane.oct` causes assertion failures.

## Solution
Add 64-bit UUID as the primary asset identifier. Keep name-based lookup for convenience.

---

## Key Changes

### 1. Data Structures

**AssetHeader** (`Asset.h`):
```cpp
struct AssetHeader
{
    uint32_t mMagic = ASSET_MAGIC_NUMBER;
    uint32_t mVersion = ASSET_VERSION_CURRENT;  // Bump to 12
    TypeId mType = INVALID_TYPE_ID;
    uint8_t mEmbedded = false;
    uint64_t mUuid = 0;  // NEW
};
```

**AssetStub** (`Asset.h`):
```cpp
struct AssetStub
{
    // ... existing fields ...
    uint64_t mUuid = 0;  // NEW
};
```

**AssetManager** (`AssetManager.h`):
```cpp
class AssetManager
{
    std::unordered_map<std::string, AssetStub*> mAssetMap;      // Name lookup (first wins)
    std::unordered_map<std::string, AssetStub*> mAssetPathMap;  // NEW: Path lookup (e.g., "Models/SM_Plane")
    std::unordered_map<uint64_t, AssetStub*> mUuidMap;          // NEW: UUID lookup (primary)
};
```

### 2. Serialization

**Stream::ReadAsset/WriteAsset** - Add format byte:
- `format=0`: Legacy name string (backward compat)
- `format=1`: UUID (new default)

### 3. Migration (Auto-UUID for Older Projects)
- Legacy assets (version < 12): **Automatically generate UUID on discovery**
- When loading older project without UUIDs:
  1. `DiscoverDirectory()` reads each .oct header
  2. If `header.mUuid == 0` (legacy), generate new UUID
  3. Store UUID in AssetStub for runtime use
  4. On next save, UUID is written to file header
- UUID persists in header on save
- Both name and UUID lookups work during transition
- No user action required - migration is automatic and transparent

---

## Implementation Order

### Phase 1: Foundation
1. Add `ReadUint64()`/`WriteUint64()` to Stream class
2. Add `GenerateAssetUuid()` utility function
3. Update AssetHeader with UUID field, bump version to 12
4. Update `Asset::ReadHeader()`/`WriteHeader()`

### Phase 2: Infrastructure
5. Add `mUuid` to AssetStub and Asset class
6. Add `mUuidMap` and `mAssetPathMap` to AssetManager
7. Add UUID-based and path-based lookup methods to AssetManager
8. Update `RegisterAsset()` to populate all three maps
9. Update `LoadAsset(string)` to check for `"Assets/"` prefix and route to path or name lookup

### Phase 3: Serialization
10. Update `Stream::ReadAsset()` with format byte + UUID path
11. Update `Stream::WriteAsset()` to write UUID format
12. Update async loading to support UUID

### Phase 4: Discovery & Migration
13. Update `DiscoverDirectory()` to read/generate UUIDs
14. Remove name-conflict assertion (allow duplicates)
15. Add lazy migration on save

---

## Engine Asset UUIDs

Engine assets use reserved UUIDs in range `0x0001000000000000` - `0x0001FFFFFFFFFFFF`:

```cpp
// In EngineTypes.h or similar
constexpr uint64_t ENGINE_UUID_BASE = 0x0001000000000000ULL;

// Engine assets get sequential IDs from this base
// SM_Plane  = ENGINE_UUID_BASE + 1
// SM_Cube   = ENGINE_UUID_BASE + 2
// T_White   = ENGINE_UUID_BASE + 100
// etc.
```

This ensures engine assets have identical UUIDs across all projects, so packaged games reference the same engine assets consistently.

---

## Files to Modify

| File | Changes |
|------|---------|
| `Engine/Source/Engine/Asset.h` | Add UUID to AssetHeader, AssetStub, Asset class |
| `Engine/Source/Engine/Asset.cpp` | Update ReadHeader/WriteHeader |
| `Engine/Source/Engine/AssetManager.h` | Add mUuidMap, UUID lookup methods |
| `Engine/Source/Engine/AssetManager.cpp` | Update RegisterAsset, discovery, remove name conflict assert |
| `Engine/Source/Engine/Stream.h` | Add ReadUint64/WriteUint64 |
| `Engine/Source/Engine/Stream.cpp` | Update ReadAsset/WriteAsset for UUID format |
| `Engine/Source/Engine/Maths.h` | Add GenerateAssetUuid() |

---

## Verification

### Editor Tests (Manual)

**Import & Discovery:**
- [ ] Import asset `SM_TestMesh` into `Assets/` folder
- [ ] Import another asset named `SM_TestMesh` into `Assets/Models/` folder - should succeed (no conflict)
- [ ] Verify both appear in Asset Browser with different paths
- [ ] Verify each has a unique UUID in inspector

**Renaming:**
- [ ] Create Material that references `T_TestTexture`
- [ ] Rename `T_TestTexture` to `T_RenamedTexture`
- [ ] Verify Material still references the texture (UUID preserved)
- [ ] Save and reload project - reference still works

**Moving Assets:**
- [ ] Move asset from `Assets/` to `Assets/Subfolder/`
- [ ] Verify UUID remains the same
- [ ] Verify all references still work

**Duplicate Names:**
- [ ] Load `SM_Plane` by name → should return engine's SM_Plane (first registered)
- [ ] Load `Assets/Models/SM_Plane` by path → should return project's SM_Plane
- [ ] Both meshes can be used in same scene simultaneously

**Path Lookup:**
- [ ] `LoadAsset("SM_Cube")` works (name lookup)
- [ ] `LoadAsset("Assets/SM_Cube")` works (path lookup)
- [ ] `LoadAsset("Assets/Models/Props/SM_Chair")` works (nested path lookup)

**Legacy Project Migration:**
- [ ] Open project created before UUID support
- [ ] Verify all assets get UUIDs assigned automatically (check inspector)
- [ ] Save project
- [ ] Reopen project - verify same UUIDs persisted (not regenerated)
- [ ] Verify no data loss or broken references

### Regression Tests (Ensure Nothing Broke)

**Build & Run:**
- [ ] `make -f Makefile_Linux_Editor` compiles without errors
- [ ] `make -f Makefile_Windows_Editor` compiles without errors
- [ ] Editor launches and loads default project

**Asset Loading:**
- [ ] Existing projects load without errors
- [ ] All existing asset references resolve correctly
- [ ] Scenes spawn all nodes with correct assets

**Serialization:**
- [ ] Save scene, reload - all asset references intact
- [ ] Export/package game - assets included correctly
- [ ] Embedded assets work in packaged builds

**Docker Build:**
- [ ] `docker run ... build-3ds` succeeds (the original issue)
- [ ] `docker run ... build-linux` succeeds
- [ ] No asset name conflict assertions

### Code Tests

1. **UUID uniqueness**: Generate 100k UUIDs, verify no collisions
2. **Round-trip**: Save asset with UUID, reload, verify UUID matches
3. **Legacy migration**: Load old .oct file (version < 12), verify UUID assigned

---

## Design Decisions

- **64-bit UUID**: Sufficient uniqueness, simpler serialization than 128-bit
- **In-header storage**: No sidecar files, atomic with asset data
- **Format byte**: Allows mixed legacy/new references during migration
- **Keep name lookup**: Convenient for editor, debugging, scripts
- **Reserved engine UUIDs**: Engine assets get hardcoded stable UUIDs (e.g., `SM_Plane = 0x0001000000000001`). Consistent across all projects.
- **Dual lookup convention**:
  - `LoadAsset("SM_Plane")` → simple name lookup (first match, backward compatible)
  - `LoadAsset("Assets/SM_Plane")` → path lookup (exact path match)
  - `LoadAsset("Assets/Models/SM_Plane")` → path lookup for nested directories
  - Detection: If string starts with `"Assets/"`, strip prefix and use path lookup; otherwise use name lookup
