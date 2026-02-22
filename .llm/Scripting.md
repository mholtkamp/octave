# Lua Scripting System

## Overview

Octave uses Lua as its scripting language. C++ classes are exposed to Lua through a macro-based binding layer with automatic registration. A stub generator produces EmmyLua-annotated files for IDE IntelliSense.

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Source/Engine/Script.h/.cpp` | Script component attached to nodes |
| `Engine/Source/Engine/ScriptMacros.h` | Binding macros (OSF, TSF, GSF) |
| `Engine/Source/Engine/ScriptAutoReg.h` | Auto-registration system |
| `Engine/Source/LuaBindings/*.h/.cpp` | 60+ binding file pairs |
| `Engine/Source/LuaBindings/LuaTypeCheck.h/.cpp` | Type checking utilities |
| `Engine/Source/LuaBindings/LuaUtils.h/.cpp` | Lua utility functions |
| `Tools/generate_lua_stubs.py` | EmmyLua stub generator |
| `Engine/Generated/LuaMeta/` | Generated stub files (76 files) |

## Script Component

**File:** `Engine/Source/Engine/Script.h/.cpp`

Each node can own a `Script` that runs a Lua file:

```cpp
class Script : public Object {
    // Lifecycle
    void StartScript();
    void RestartScript();
    void StopScript();
    void Tick(float deltaTime);
    bool IsActive() const;

    // File
    void SetFile(const char* filename);
    bool ReloadScriptFile(const std::string& fileName, bool restart = true);

    // Function calls (up to 8 params)
    bool HasFunction(const char* name) const;
    void CallFunction(const char* name);
    void CallFunction(const char* name, const Datum& param0, ...);
    Datum CallFunctionR(const char* name);   // With return value

    // Properties (exposed from Lua to editor)
    void GatherScriptProperties();
    std::vector<Property>& GetScriptProperties();
    void UploadScriptProperties();

    // Auto Properties (dynamic, from Lua)
    void AddAutoProperty(varName, displayName, type, defaultValue);
    void ClearAutoProperties();

    // Physics events (forwarded from node)
    void BeginOverlap(Primitive3D* thisNode, Primitive3D* otherNode);
    void EndOverlap(...);
    void OnCollision(..., glm::vec3 impactPoint, glm::vec3 impactNormal, ...);

    // Field access
    Datum GetField(const char* key);
    void SetField(const char* key, const Datum& value);
};
```

## Binding Macros (ScriptMacros.h)

Macros wrap C++ methods for Lua consumption:

**Object Script Functions (OSF)** — methods on Objects:
- `OSF(Class, Func)` — no params, no return
- `OSF_1..8(Class, Func)` — 1-8 params
- `OSF_R(Class, Func)` — with return
- `OSF_R_1..8(Class, Func)` — params + return

**Table/Global Script Functions:**
- `TSF(Table, Func)` — table-level (e.g., `Math.Lerp`)
- `GSF(Func)` — global functions

**Parameter extraction:** `SpNode(idx)`, `SpFloat(idx)`, `SpInt(idx)`, `SpString(idx)`, `SpBool(idx)`, `SpVector(idx)`, `SpAsset(idx)`, `SpFunction(idx)`, `SpNodeOrNil(idx)`.

**Return conversion:** `SrReturn`, `SrNode`, `SrAsset`, `SrVector`, `SrInt`, `SrFloat`, `SrString`, `SrBool`.

## Auto-Registration (ScriptAutoReg.h)

Each `OSF(Class, Func)` macro expands to a `ScriptFuncAutoReg` temporary that pushes registration data into a per-class vector at static init time.

At runtime, `RegisterScriptFuncs(lua_State* L)` iterates the vector and registers each function into the Lua metatable. Pattern:

```cpp
DECLARE_SCRIPT_LINK(Type, ParentType, TopType)  // In header
DEFINE_SCRIPT_LINK(Type, ParentType, TopType)    // In source
```

## Binding File Pattern

Each `*_Lua.h` defines a struct with static methods:

```cpp
struct Node_Lua {
    NodePtr mNode;    // Lua userdata holds this

    static int Create(lua_State* L, Node* node);
    static int GetName(lua_State* L);
    static int SetName(lua_State* L);
    // ... 60+ methods
    static void BindCommon(lua_State* L, int mtIndex);
};
```

Type checking: `CheckLuaType<T>(L, arg, typeName)`, `CheckHierarchyLuaType<T>(L, arg, typeName, classFlag)`, `CheckAssetLuaType<T>(L, arg, className, classFlag)`.

## Binding Files (60+ pairs)

Categories:
- **Core**: Engine_Lua, Asset_Lua, AssetManager_Lua, Scene_Lua, Script_Lua, System_Lua
- **Nodes**: Node_Lua, Node3d_Lua, Mesh3d_Lua, Camera3d_Lua, Light3d_Lua, Box3d_Lua, etc.
- **Assets**: Material_Lua, Texture_Lua, StaticMesh_Lua, Font_Lua, SoundWave_Lua, etc.
- **Widgets**: Widget_Lua, Canvas_Lua, Quad_Lua, Text_Lua, Button_Lua
- **Systems**: Audio_Lua, Input_Lua, Network_Lua, Log_Lua, Maths_Lua
- **Other**: Signal_Lua, Property_Lua, Stream_Lua, TimerManager_Lua, NodeGraphPlayer_Lua, Timeline_Lua

## Stub Generator

**File:** `Tools/generate_lua_stubs.py`

Parses `*_Lua.h/cpp` files, extracts function signatures by mapping `CHECK_*` macros and `lua_push*` calls to LuaLS types. Outputs EmmyLua-annotated `.lua` stubs.

**Output:** 76 files in `Engine/Generated/LuaMeta/` (59 types + 17 enums)

**Usage:**
```bash
python Tools/generate_lua_stubs.py [--verbose]
```

**VS Code config:**
```json
{ "Lua.workspace.library": ["Engine/Generated/LuaMeta"] }
```

## XSD Schema Generator

**File:** `Tools/generate_ui_xsd.py`

Generates an XSD schema for UIDocument XML files, enabling autocomplete and validation in XML editors (VS Code + Red Hat XML, IntelliJ, etc.). The schema encodes all UI elements, attributes, and enum values as hardcoded Python data structures (no C++ parsing).

**Output:** `Engine/Generated/XML/OctaveUIDocument.xsd`

**Usage:**
```bash
python Tools/generate_ui_xsd.py [--verbose]
```

**VS Code config:**
```json
{ "xml.fileAssociations": [{ "pattern": "**/*.xml", "systemId": "Engine/Generated/XML/OctaveUIDocument.xsd" }] }
```

To update the schema (e.g., new elements or attributes), edit the Python data structures in `generate_ui_xsd.py` and re-run. See `Documentation/Development/UI/XMLUISystem.md` for full UI system docs.

## Lua API Documentation

User-facing Lua API docs are in `Documentation/Lua/` and served via MkDocs. Sections: Systems, Assets, Nodes, Widgets, Misc.
