#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @file OctaveEngineAPI.h
 * @brief Engine API exposed to native addon plugins.
 *
 * This struct provides plugins with access to engine functionality
 * through stable function pointers.
 */

struct lua_State;
struct EditorUIHooks;
struct ImGuiPluginContext;

// Forward declarations for engine types
class Node;
class Node3D;
class World;
class Asset;
class SoundWave;

/**
 * @brief Engine API provided to plugins during OnLoad.
 *
 * All function pointers are guaranteed to be valid for the lifetime
 * of the plugin (from OnLoad to OnUnload).
 */
struct OctaveEngineAPI
{
    // ===== Logging =====

    /**
     * @brief Log a debug message.
     * @param fmt Printf-style format string
     */
    void (*LogDebug)(const char* fmt, ...);

    /**
     * @brief Log a warning message.
     * @param fmt Printf-style format string
     */
    void (*LogWarning)(const char* fmt, ...);

    /**
     * @brief Log an error message.
     * @param fmt Printf-style format string
     */
    void (*LogError)(const char* fmt, ...);

    // ===== Lua Access =====

    /**
     * @brief Get the main Lua state.
     * @return Pointer to Lua state, or nullptr if not available
     */
    lua_State* (*GetLua)();

    // ===== Lua Wrappers =====
    // These wrap Lua C API functions so plugins don't need to link against Lua.
    // Names use "Lua_" prefix to avoid conflicts with Lua macros.
    // Example: Use sEngineAPI->Lua_pushnumber() instead of lua_pushnumber()

    // Stack manipulation
    void (*Lua_settop)(lua_State* L, int idx);
    void (*Lua_pushvalue)(lua_State* L, int idx);
    void (*Lua_pop)(lua_State* L, int n);
    int (*Lua_gettop)(lua_State* L);

    // Type checking
    int (*Lua_type)(lua_State* L, int idx);
    int (*Lua_isfunction)(lua_State* L, int idx);
    int (*Lua_istable)(lua_State* L, int idx);
    int (*Lua_isuserdata)(lua_State* L, int idx);
    int (*Lua_isnil)(lua_State* L, int idx);

    // Get values
    int (*Lua_toboolean)(lua_State* L, int idx);
    double (*Lua_tonumber)(lua_State* L, int idx);
    const char* (*Lua_tostring)(lua_State* L, int idx);
    void* (*Lua_touserdata)(lua_State* L, int idx);

    // Push values
    void (*Lua_pushnil)(lua_State* L);
    void (*Lua_pushboolean)(lua_State* L, int b);
    void (*Lua_pushnumber)(lua_State* L, double n);
    void (*Lua_pushstring)(lua_State* L, const char* s);
    void (*Lua_pushinteger)(lua_State* L, long long n);

    // Userdata
    void* (*Lua_newuserdata)(lua_State* L, size_t sz);

    // Tables and fields
    void (*Lua_createtable)(lua_State* L, int narr, int nrec);
    void (*Lua_setfield)(lua_State* L, int idx, const char* k);
    void (*Lua_getfield)(lua_State* L, int idx, const char* k);
    void (*Lua_setglobal)(lua_State* L, const char* name);
    void (*Lua_getglobal)(lua_State* L, const char* name);
    void (*Lua_rawset)(lua_State* L, int idx);
    void (*Lua_rawget)(lua_State* L, int idx);
    void (*Lua_settable)(lua_State* L, int idx);
    void (*Lua_gettable)(lua_State* L, int idx);

    // Metatables
    int (*Lua_setmetatable)(lua_State* L, int objindex);
    int (*Lua_getmetatable)(lua_State* L, int objindex);

    // Auxiliary library functions (LuaL_ prefix to avoid macro conflicts)
    int (*LuaL_newmetatable)(lua_State* L, const char* tname);
    void (*LuaL_setmetatable)(lua_State* L, const char* tname);
    void* (*LuaL_checkudata)(lua_State* L, int ud, const char* tname);
    double (*LuaL_checknumber)(lua_State* L, int arg);
    long long (*LuaL_checkinteger)(lua_State* L, int arg);
    const char* (*LuaL_checkstring)(lua_State* L, int arg);
    void (*LuaL_setfuncs)(lua_State* L, const void* l, int nup);  // luaL_Reg*
    void (*LuaL_getmetatable)(lua_State* L, const char* tname);

    // ===== World Management =====

    /**
     * @brief Get a world by index.
     * @param index World index (usually 0 for main world)
     * @return Pointer to world, or nullptr if index is out of range
     */
    World* (*GetWorld)(int32_t index);

    /**
     * @brief Get the number of active worlds.
     * @return Number of worlds
     */
    int32_t (*GetNumWorlds)();

    // ===== Node Operations =====

    /**
     * @brief Spawn a node of the given type in a world.
     * @param world World to spawn in
     * @param typeName Type name of node to spawn (e.g., "Mesh3D", "Camera3D")
     * @return Newly spawned node, or nullptr on failure
     */
    Node* (*SpawnNode)(World* world, const char* typeName);

    /**
     * @brief Destroy a node.
     * @param node Node to destroy
     */
    void (*DestroyNode)(Node* node);

    /**
     * @brief Find a node by name in a world.
     * @param world World to search in
     * @param name Node name to find
     * @return Found node, or nullptr if not found
     */
    Node* (*FindNode)(World* world, const char* name);

    // ===== Node3D Operations =====

    /**
     * @brief Get the local rotation of a Node3D in euler angles (degrees).
     * @param node The Node3D to query
     * @param outX Output: X rotation in degrees
     * @param outY Output: Y rotation in degrees
     * @param outZ Output: Z rotation in degrees
     */
    void (*Node3D_GetRotation)(Node3D* node, float* outX, float* outY, float* outZ);

    /**
     * @brief Set the local rotation of a Node3D in euler angles (degrees).
     * @param node The Node3D to modify
     * @param x X rotation in degrees
     * @param y Y rotation in degrees
     * @param z Z rotation in degrees
     */
    void (*Node3D_SetRotation)(Node3D* node, float x, float y, float z);

    /**
     * @brief Add rotation to a Node3D in euler angles (degrees).
     * @param node The Node3D to modify
     * @param x X rotation delta in degrees
     * @param y Y rotation delta in degrees
     * @param z Z rotation delta in degrees
     */
    void (*Node3D_AddRotation)(Node3D* node, float x, float y, float z);

    /**
     * @brief Get the local position of a Node3D.
     * @param node The Node3D to query
     * @param outX Output: X position
     * @param outY Output: Y position
     * @param outZ Output: Z position
     */
    void (*Node3D_GetPosition)(Node3D* node, float* outX, float* outY, float* outZ);

    /**
     * @brief Set the local position of a Node3D.
     * @param node The Node3D to modify
     * @param x X position
     * @param y Y position
     * @param z Z position
     */
    void (*Node3D_SetPosition)(Node3D* node, float x, float y, float z);

    /**
     * @brief Get the local scale of a Node3D.
     * @param node The Node3D to query
     * @param outX Output: X scale
     * @param outY Output: Y scale
     * @param outZ Output: Z scale
     */
    void (*Node3D_GetScale)(Node3D* node, float* outX, float* outY, float* outZ);

    /**
     * @brief Set the local scale of a Node3D.
     * @param node The Node3D to modify
     * @param x X scale
     * @param y Y scale
     * @param z Z scale
     */
    void (*Node3D_SetScale)(Node3D* node, float x, float y, float z);

    // ===== Asset System =====

    /**
     * @brief Load an asset by name (loads from disk if not already loaded).
     * @param name Asset name
     * @return Loaded asset, or nullptr on failure
     */
    Asset* (*LoadAsset)(const char* name);

    /**
     * @brief Fetch an asset by name (returns nullptr if not already loaded).
     * @param name Asset name
     * @return Asset if already loaded, nullptr otherwise
     */
    Asset* (*FetchAsset)(const char* name);

    /**
     * @brief Unload an asset by name.
     * @param name Asset name to unload
     */
    void (*UnloadAsset)(const char* name);

    // ===== Audio =====

    /**
     * @brief Play a 2D sound (non-positional).
     * @param sound Sound wave asset to play
     * @param volume Volume multiplier (0.0 - 1.0+)
     * @param pitch Pitch multiplier (1.0 = normal)
     */
    void (*PlaySound2D)(SoundWave* sound, float volume, float pitch);

    /**
     * @brief Stop all currently playing sounds.
     */
    void (*StopAllSounds)();

    /**
     * @brief Set the master audio volume.
     * @param volume Volume (0.0 - 1.0)
     */
    void (*SetMasterVolume)(float volume);

    /**
     * @brief Get the master audio volume.
     * @return Current master volume
     */
    float (*GetMasterVolume)();

    // ===== Input =====

    /**
     * @brief Check if a key is currently held down.
     * @param key Key code (from InputConstants.h)
     * @return true if key is down
     */
    bool (*IsKeyDown)(int32_t key);

    /**
     * @brief Check if a key was just pressed this frame.
     * @param key Key code (from InputConstants.h)
     * @return true if key was just pressed
     */
    bool (*IsKeyJustPressed)(int32_t key);

    /**
     * @brief Check if a key was just released this frame.
     * @param key Key code (from InputConstants.h)
     * @return true if key was just released
     */
    bool (*IsKeyJustReleased)(int32_t key);

    /**
     * @brief Check if a mouse button is currently held down.
     * @param button Mouse button (0=left, 1=right, 2=middle)
     * @return true if button is down
     */
    bool (*IsMouseButtonDown)(int32_t button);

    /**
     * @brief Check if a mouse button was just pressed this frame.
     * @param button Mouse button (0=left, 1=right, 2=middle)
     * @return true if button was just pressed
     */
    bool (*IsMouseButtonJustPressed)(int32_t button);

    /**
     * @brief Get the current mouse position.
     * @param x Output: mouse X position
     * @param y Output: mouse Y position
     */
    void (*GetMousePosition)(int32_t* x, int32_t* y);

    /**
     * @brief Get the mouse movement since last frame.
     * @param deltaX Output: mouse X delta
     * @param deltaY Output: mouse Y delta
     */
    void (*GetMouseDelta)(int32_t* deltaX, int32_t* deltaY);

    /**
     * @brief Get the scroll wheel delta.
     * @return Scroll wheel delta (positive = up, negative = down)
     */
    int32_t (*GetScrollWheelDelta)();

    // ===== Time =====

    /**
     * @brief Get the time elapsed since last frame (in seconds).
     * @return Delta time in seconds
     */
    float (*GetDeltaTime)();

    /**
     * @brief Get the total time elapsed since engine start (in seconds).
     * @return Elapsed time in seconds
     */
    float (*GetElapsedTime)();

    // ===== Gizmos =====

    /**
     * @brief Set the gizmo drawing color.
     * @param r Red component (0.0 - 1.0)
     * @param g Green component (0.0 - 1.0)
     * @param b Blue component (0.0 - 1.0)
     * @param a Alpha component (0.0 - 1.0)
     */
    void (*Gizmos_SetColor)(float r, float g, float b, float a);

    /**
     * @brief Set the gizmo transformation matrix (column-major, 16 floats).
     * @param matrix16 Pointer to 16 floats (column-major 4x4 matrix)
     */
    void (*Gizmos_SetMatrix)(const float* matrix16);

    /**
     * @brief Reset gizmo state to default (white color, identity matrix).
     */
    void (*Gizmos_ResetState)();

    /**
     * @brief Draw a solid cube gizmo.
     * @param cx Center X
     * @param cy Center Y
     * @param cz Center Z
     * @param sx Size X
     * @param sy Size Y
     * @param sz Size Z
     */
    void (*Gizmos_DrawCube)(float cx, float cy, float cz, float sx, float sy, float sz);

    /**
     * @brief Draw a wireframe cube gizmo.
     */
    void (*Gizmos_DrawWireCube)(float cx, float cy, float cz, float sx, float sy, float sz);

    /**
     * @brief Draw a solid sphere gizmo.
     * @param cx Center X
     * @param cy Center Y
     * @param cz Center Z
     * @param radius Sphere radius
     */
    void (*Gizmos_DrawSphere)(float cx, float cy, float cz, float radius);

    /**
     * @brief Draw a wireframe sphere gizmo.
     */
    void (*Gizmos_DrawWireSphere)(float cx, float cy, float cz, float radius);

    /**
     * @brief Draw a line gizmo.
     * @param x0 Start X
     * @param y0 Start Y
     * @param z0 Start Z
     * @param x1 End X
     * @param y1 End Y
     * @param z1 End Z
     */
    void (*Gizmos_DrawLine)(float x0, float y0, float z0, float x1, float y1, float z1);

    /**
     * @brief Draw a ray gizmo (from origin along direction).
     * @param ox Origin X
     * @param oy Origin Y
     * @param oz Origin Z
     * @param dx Direction X
     * @param dy Direction Y
     * @param dz Direction Z
     */
    void (*Gizmos_DrawRay)(float ox, float oy, float oz, float dx, float dy, float dz);

    // ===== Editor UI Hooks (Editor builds only) =====

    /**
     * @brief Pointer to editor UI hooks struct.
     *
     * This is nullptr in non-editor (game) builds.
     * Check before using to ensure plugin works in both contexts.
     */
    EditorUIHooks* editorUI;

    /**
     * @brief Get ImGui context for plugin use (Editor builds only).
     *
     * Plugins MUST call this in OnLoad and set up ImGui context before
     * calling any ImGui functions. Returns nullptr in non-editor builds.
     *
     * @param outCtx Pointer to ImGuiPluginContext to fill
     */
    void (*GetImGuiContext)(ImGuiPluginContext* outCtx);
};
