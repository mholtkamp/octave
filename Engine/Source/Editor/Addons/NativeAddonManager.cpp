#if EDITOR

#include "NativeAddonManager.h"
#include "AddonManager.h"
#include "System/System.h"
#include "System/ModuleLoader.h"
#include "Engine.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Engine/AudioManager.h"
#include "Engine/Clock.h"
#include "Engine/Nodes/Node.h"
#include "Engine/Nodes/3D/Node3d.h"
#include "Engine/Gizmos.h"
#include "Engine/Assets/TinyLLMAsset.h"
#include "Input/Input.h"
#include "Log.h"
#include "Stream.h"
#include "Utilities.h"
#include "Script.h"
#include "Plugins/ImGuiPluginContext.h"

#include "document.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <cstdio>
#include <cctype>
#include <cstring>
#include <functional>
#include <algorithm>
#include <sstream>

NativeAddonManager* NativeAddonManager::sInstance = nullptr;

// Helper function to create directories recursively
static bool CreateDirectoryRecursive(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    // Normalize path separators
    std::string normalizedPath = path;
    for (char& c : normalizedPath)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }

    // Remove trailing slash for processing
    if (normalizedPath.back() == '/')
    {
        normalizedPath.pop_back();
    }

    // If directory already exists, we're done
    if (DoesDirExist(normalizedPath.c_str()))
    {
        return true;
    }

    // Find parent directory
    size_t lastSlash = normalizedPath.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash > 0)
    {
        std::string parentPath = normalizedPath.substr(0, lastSlash);

        // Skip drive letter on Windows (e.g., "M:")
        bool isDriveRoot = (parentPath.length() == 2 && parentPath[1] == ':');
        if (!isDriveRoot && !DoesDirExist(parentPath.c_str()))
        {
            if (!CreateDirectoryRecursive(parentPath))
            {
                return false;
            }
        }
    }

    // Create this directory
    return SYS_CreateDirectory(normalizedPath.c_str());
}

// Helper function to convert addon name to export macro name
// e.g., "inventory-system-runtime" -> "INVENTORY_SYSTEM_RUNTIME_EXPORTS"
static std::string GenerateExportMacroName(const std::string& addonName)
{
    std::string result;
    for (char c : addonName)
    {
        if (c == '-' || c == ' ')
        {
            result += '_';
        }
        else if (std::isalnum(static_cast<unsigned char>(c)))
        {
            result += std::toupper(static_cast<unsigned char>(c));
        }
    }
    result += "_EXPORTS";
    return result;
}

// Helper function to convert addon name to library name (for .lib files)
// e.g., "inventory-system-runtime" -> "inventory_system_runtime"
static std::string GenerateLibraryName(const std::string& addonName)
{
    std::string result;
    for (char c : addonName)
    {
        if (c == '-')
        {
            result += '_';
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// ===== Engine API Implementation =====

static void PluginLogDebug(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LogDebug("[Plugin] %s", buffer);
}

static void PluginLogWarning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LogWarning("[Plugin] %s", buffer);
}

static void PluginLogError(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LogError("[Plugin] %s", buffer);
}

static lua_State* PluginGetLua()
{
    return GetLua();
}

// ===== World Management =====

static World* PluginGetWorld(int32_t index)
{
    return GetWorld(index);
}

static int32_t PluginGetNumWorlds()
{
    return GetNumWorlds();
}

// ===== Node Operations =====

static Node* PluginSpawnNode(World* world, const char* typeName)
{
    if (world == nullptr || typeName == nullptr)
    {
        return nullptr;
    }
    return world->SpawnNode(typeName);
}

static void PluginDestroyNode(Node* node)
{
    if (node != nullptr)
    {
        node->Destroy();
    }
}

static Node* PluginFindNode(World* world, const char* name)
{
    if (world == nullptr || name == nullptr)
    {
        return nullptr;
    }
    return world->FindNode(name);
}

// ===== Node3D Operations =====

static void PluginNode3D_GetRotation(Node3D* node, float* outX, float* outY, float* outZ)
{
    if (node != nullptr && outX != nullptr && outY != nullptr && outZ != nullptr)
    {
        glm::vec3 rot = node->GetRotationEuler();
        *outX = rot.x;
        *outY = rot.y;
        *outZ = rot.z;
    }
}

static void PluginNode3D_SetRotation(Node3D* node, float x, float y, float z)
{
    if (node != nullptr)
    {
        node->SetRotation(glm::vec3(x, y, z));
    }
}

static void PluginNode3D_AddRotation(Node3D* node, float x, float y, float z)
{
    if (node != nullptr)
    {
        node->AddRotation(glm::vec3(x, y, z));
    }
}

static void PluginNode3D_GetPosition(Node3D* node, float* outX, float* outY, float* outZ)
{
    if (node != nullptr && outX != nullptr && outY != nullptr && outZ != nullptr)
    {
        glm::vec3 pos = node->GetPosition();
        *outX = pos.x;
        *outY = pos.y;
        *outZ = pos.z;
    }
}

static void PluginNode3D_SetPosition(Node3D* node, float x, float y, float z)
{
    if (node != nullptr)
    {
        node->SetPosition(glm::vec3(x, y, z));
    }
}

static void PluginNode3D_GetScale(Node3D* node, float* outX, float* outY, float* outZ)
{
    if (node != nullptr && outX != nullptr && outY != nullptr && outZ != nullptr)
    {
        glm::vec3 scale = node->GetScale();
        *outX = scale.x;
        *outY = scale.y;
        *outZ = scale.z;
    }
}

static void PluginNode3D_SetScale(Node3D* node, float x, float y, float z)
{
    if (node != nullptr)
    {
        node->SetScale(glm::vec3(x, y, z));
    }
}

// ===== Asset System =====

static Asset* PluginLoadAsset(const char* name)
{
    if (name == nullptr)
    {
        return nullptr;
    }
    return LoadAsset(name);
}

static Asset* PluginFetchAsset(const char* name)
{
    if (name == nullptr)
    {
        return nullptr;
    }
    return FetchAsset(name);
}

static void PluginUnloadAsset(const char* name)
{
    if (name != nullptr)
    {
        UnloadAsset(name);
    }
}

// ===== TinyLLM =====

static int32_t PluginTinyLLM_Encode(Asset* model, const char* text, bool addBos, bool addEos,
                                     int32_t* outTokens, int32_t maxTokens)
{
    if (!model || !text || !outTokens || maxTokens <= 0)
    {
        return -1;
    }

    TinyLLMAsset* llmAsset = model->As<TinyLLMAsset>();
    if (!llmAsset)
    {
        return -1;
    }

    std::vector<int32_t> tokens;
    llmAsset->Encode(text, addBos, addEos, tokens);

    int32_t count = (int32_t)tokens.size();
    if (count > maxTokens)
    {
        count = maxTokens;
    }

    for (int32_t i = 0; i < count; i++)
    {
        outTokens[i] = tokens[i];
    }

    return count;
}

static int32_t PluginTinyLLM_Decode(Asset* model, int32_t prevToken, int32_t token,
                                     char* outStr, int32_t maxLen)
{
    if (!model || !outStr || maxLen <= 0)
    {
        return -1;
    }

    TinyLLMAsset* llmAsset = model->As<TinyLLMAsset>();
    if (!llmAsset)
    {
        return -1;
    }

    std::string decoded = llmAsset->Decode(prevToken, token);
    int32_t len = (int32_t)decoded.size();

    if (len >= maxLen)
    {
        len = maxLen - 1;
    }

    memcpy(outStr, decoded.c_str(), len);
    outStr[len] = '\0';

    return len;
}

// ===== Audio =====

static void PluginPlaySound2D(SoundWave* sound, float volume, float pitch)
{
    if (sound != nullptr)
    {
        AudioManager::PlaySound2D(sound, volume, pitch, 0.0f, false);
    }
}

static void PluginStopAllSounds()
{
    AudioManager::StopAllSounds();
}

static void PluginSetMasterVolume(float volume)
{
    AudioManager::SetMasterVolume(volume);
}

static float PluginGetMasterVolume()
{
    return AudioManager::GetMasterVolume();
}

// ===== Input =====

static bool PluginIsKeyDown(int32_t key)
{
    return INP_IsKeyDown(key);
}

static bool PluginIsKeyJustPressed(int32_t key)
{
    return INP_IsKeyJustDown(key);
}

static bool PluginIsKeyJustReleased(int32_t key)
{
    return INP_IsKeyJustUp(key);
}

static bool PluginIsMouseButtonDown(int32_t button)
{
    return INP_IsMouseButtonDown(button);
}

static bool PluginIsMouseButtonJustPressed(int32_t button)
{
    return INP_IsMouseButtonJustDown(button);
}

static void PluginGetMousePosition(int32_t* x, int32_t* y)
{
    if (x != nullptr && y != nullptr)
    {
        INP_GetMousePosition(*x, *y);
    }
}

static void PluginGetMouseDelta(int32_t* deltaX, int32_t* deltaY)
{
    if (deltaX != nullptr && deltaY != nullptr)
    {
        INP_GetMouseDelta(*deltaX, *deltaY);
    }
}

static int32_t PluginGetScrollWheelDelta()
{
    return INP_GetScrollWheelDelta();
}

// ===== Time =====

static float PluginGetDeltaTime()
{
    const Clock* clock = GetAppClock();
    return clock ? clock->DeltaTime() : 0.0f;
}

static float PluginGetElapsedTime()
{
    const Clock* clock = GetAppClock();
    return clock ? clock->GetTime() : 0.0f;
}

// ===== Lua Wrappers =====
// These forward calls to the engine's Lua library so plugins don't need to link against Lua

static void Plugin_lua_settop(lua_State* L, int idx) { lua_settop(L, idx); }
static void Plugin_lua_pushvalue(lua_State* L, int idx) { lua_pushvalue(L, idx); }
static void Plugin_lua_pop(lua_State* L, int n) { lua_pop(L, n); }
static int Plugin_lua_gettop(lua_State* L) { return lua_gettop(L); }

static int Plugin_lua_type(lua_State* L, int idx) { return lua_type(L, idx); }
static int Plugin_lua_isfunction(lua_State* L, int idx) { return lua_isfunction(L, idx); }
static int Plugin_lua_istable(lua_State* L, int idx) { return lua_istable(L, idx); }
static int Plugin_lua_isuserdata(lua_State* L, int idx) { return lua_isuserdata(L, idx); }
static int Plugin_lua_isnil(lua_State* L, int idx) { return lua_isnil(L, idx); }

static int Plugin_lua_toboolean(lua_State* L, int idx) { return lua_toboolean(L, idx); }
static double Plugin_lua_tonumber(lua_State* L, int idx) { return (double)lua_tonumber(L, idx); }
static const char* Plugin_lua_tostring(lua_State* L, int idx) { return lua_tostring(L, idx); }
static void* Plugin_lua_touserdata(lua_State* L, int idx) { return lua_touserdata(L, idx); }

static void Plugin_lua_pushnil(lua_State* L) { lua_pushnil(L); }
static void Plugin_lua_pushboolean(lua_State* L, int b) { lua_pushboolean(L, b); }
static void Plugin_lua_pushnumber(lua_State* L, double n) { lua_pushnumber(L, (lua_Number)n); }
static void Plugin_lua_pushstring(lua_State* L, const char* s) { lua_pushstring(L, s); }
static void Plugin_lua_pushinteger(lua_State* L, long long n) { lua_pushinteger(L, (lua_Integer)n); }

static void* Plugin_lua_newuserdata(lua_State* L, size_t sz) { return lua_newuserdata(L, sz); }

static void Plugin_lua_createtable(lua_State* L, int narr, int nrec) { lua_createtable(L, narr, nrec); }
static void Plugin_lua_setfield(lua_State* L, int idx, const char* k) { lua_setfield(L, idx, k); }
static void Plugin_lua_getfield(lua_State* L, int idx, const char* k) { lua_getfield(L, idx, k); }
static void Plugin_lua_setglobal(lua_State* L, const char* name) { lua_setglobal(L, name); }
static void Plugin_lua_getglobal(lua_State* L, const char* name) { lua_getglobal(L, name); }
static void Plugin_lua_rawset(lua_State* L, int idx) { lua_rawset(L, idx); }
static void Plugin_lua_rawget(lua_State* L, int idx) { lua_rawget(L, idx); }
static void Plugin_lua_settable(lua_State* L, int idx) { lua_settable(L, idx); }
static void Plugin_lua_gettable(lua_State* L, int idx) { lua_gettable(L, idx); }

static int Plugin_lua_setmetatable(lua_State* L, int objindex) { return lua_setmetatable(L, objindex); }
static int Plugin_lua_getmetatable(lua_State* L, int objindex) { return lua_getmetatable(L, objindex); }

static int Plugin_luaL_newmetatable(lua_State* L, const char* tname) { return luaL_newmetatable(L, tname); }
static void Plugin_luaL_setmetatable(lua_State* L, const char* tname) { luaL_setmetatable(L, tname); }
static void* Plugin_luaL_checkudata(lua_State* L, int ud, const char* tname) { return luaL_checkudata(L, ud, tname); }
static double Plugin_luaL_checknumber(lua_State* L, int arg) { return (double)luaL_checknumber(L, arg); }
static long long Plugin_luaL_checkinteger(lua_State* L, int arg) { return (long long)luaL_checkinteger(L, arg); }
static const char* Plugin_luaL_checkstring(lua_State* L, int arg) { return luaL_checkstring(L, arg); }
static void Plugin_luaL_setfuncs(lua_State* L, const void* l, int nup) { luaL_setfuncs(L, (const luaL_Reg*)l, nup); }
static void Plugin_luaL_getmetatable(lua_State* L, const char* tname) { luaL_getmetatable(L, tname); }

// ===== Gizmos Wrappers =====
static void PluginGizmos_SetColor(float r, float g, float b, float a) { Gizmos::SetColor(glm::vec4(r, g, b, a)); }
static void PluginGizmos_SetMatrix(const float* m) { Gizmos::SetMatrix(glm::make_mat4(m)); }
static void PluginGizmos_ResetState() { Gizmos::ResetState(); }
static void PluginGizmos_DrawCube(float cx, float cy, float cz, float sx, float sy, float sz) { Gizmos::DrawCube({cx,cy,cz}, {sx,sy,sz}); }
static void PluginGizmos_DrawWireCube(float cx, float cy, float cz, float sx, float sy, float sz) { Gizmos::DrawWireCube({cx,cy,cz}, {sx,sy,sz}); }
static void PluginGizmos_DrawSphere(float cx, float cy, float cz, float radius) { Gizmos::DrawSphere({cx,cy,cz}, radius); }
static void PluginGizmos_DrawWireSphere(float cx, float cy, float cz, float radius) { Gizmos::DrawWireSphere({cx,cy,cz}, radius); }
static void PluginGizmos_DrawLine(float x0, float y0, float z0, float x1, float y1, float z1) { Gizmos::DrawLine({x0,y0,z0}, {x1,y1,z1}); }
static void PluginGizmos_DrawRay(float ox, float oy, float oz, float dx, float dy, float dz) { Gizmos::DrawRay({ox,oy,oz}, {dx,dy,dz}); }

// ===== NativeAddonManager Implementation =====

void NativeAddonManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new NativeAddonManager();
    }
}

void NativeAddonManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

NativeAddonManager* NativeAddonManager::Get()
{
    return sInstance;
}

NativeAddonManager::NativeAddonManager()
{
    InitializeEngineAPI();
}

NativeAddonManager::~NativeAddonManager()
{
    // Unload all native addons
    for (auto& pair : mStates)
    {
        if (pair.second.mModuleHandle != nullptr)
        {
            UnloadNativeAddon(pair.first);
        }
    }
}

void NativeAddonManager::InitializeEngineAPI()
{
    // Logging
    mEngineAPI.LogDebug = PluginLogDebug;
    mEngineAPI.LogWarning = PluginLogWarning;
    mEngineAPI.LogError = PluginLogError;

    // Lua
    mEngineAPI.GetLua = PluginGetLua;

    // Lua Wrappers (Lua_ prefix to avoid macro conflicts)
    mEngineAPI.Lua_settop = Plugin_lua_settop;
    mEngineAPI.Lua_pushvalue = Plugin_lua_pushvalue;
    mEngineAPI.Lua_pop = Plugin_lua_pop;
    mEngineAPI.Lua_gettop = Plugin_lua_gettop;

    mEngineAPI.Lua_type = Plugin_lua_type;
    mEngineAPI.Lua_isfunction = Plugin_lua_isfunction;
    mEngineAPI.Lua_istable = Plugin_lua_istable;
    mEngineAPI.Lua_isuserdata = Plugin_lua_isuserdata;
    mEngineAPI.Lua_isnil = Plugin_lua_isnil;

    mEngineAPI.Lua_toboolean = Plugin_lua_toboolean;
    mEngineAPI.Lua_tonumber = Plugin_lua_tonumber;
    mEngineAPI.Lua_tostring = Plugin_lua_tostring;
    mEngineAPI.Lua_touserdata = Plugin_lua_touserdata;

    mEngineAPI.Lua_pushnil = Plugin_lua_pushnil;
    mEngineAPI.Lua_pushboolean = Plugin_lua_pushboolean;
    mEngineAPI.Lua_pushnumber = Plugin_lua_pushnumber;
    mEngineAPI.Lua_pushstring = Plugin_lua_pushstring;
    mEngineAPI.Lua_pushinteger = Plugin_lua_pushinteger;

    mEngineAPI.Lua_newuserdata = Plugin_lua_newuserdata;

    mEngineAPI.Lua_createtable = Plugin_lua_createtable;
    mEngineAPI.Lua_setfield = Plugin_lua_setfield;
    mEngineAPI.Lua_getfield = Plugin_lua_getfield;
    mEngineAPI.Lua_setglobal = Plugin_lua_setglobal;
    mEngineAPI.Lua_getglobal = Plugin_lua_getglobal;
    mEngineAPI.Lua_rawset = Plugin_lua_rawset;
    mEngineAPI.Lua_rawget = Plugin_lua_rawget;
    mEngineAPI.Lua_settable = Plugin_lua_settable;
    mEngineAPI.Lua_gettable = Plugin_lua_gettable;

    mEngineAPI.Lua_setmetatable = Plugin_lua_setmetatable;
    mEngineAPI.Lua_getmetatable = Plugin_lua_getmetatable;

    mEngineAPI.LuaL_newmetatable = Plugin_luaL_newmetatable;
    mEngineAPI.LuaL_setmetatable = Plugin_luaL_setmetatable;
    mEngineAPI.LuaL_checkudata = Plugin_luaL_checkudata;
    mEngineAPI.LuaL_checknumber = Plugin_luaL_checknumber;
    mEngineAPI.LuaL_checkinteger = Plugin_luaL_checkinteger;
    mEngineAPI.LuaL_checkstring = Plugin_luaL_checkstring;
    mEngineAPI.LuaL_setfuncs = Plugin_luaL_setfuncs;
    mEngineAPI.LuaL_getmetatable = Plugin_luaL_getmetatable;

    // World Management
    mEngineAPI.GetWorld = PluginGetWorld;
    mEngineAPI.GetNumWorlds = PluginGetNumWorlds;

    // Node Operations
    mEngineAPI.SpawnNode = PluginSpawnNode;
    mEngineAPI.DestroyNode = PluginDestroyNode;
    mEngineAPI.FindNode = PluginFindNode;

    // Node3D Operations
    mEngineAPI.Node3D_GetRotation = PluginNode3D_GetRotation;
    mEngineAPI.Node3D_SetRotation = PluginNode3D_SetRotation;
    mEngineAPI.Node3D_AddRotation = PluginNode3D_AddRotation;
    mEngineAPI.Node3D_GetPosition = PluginNode3D_GetPosition;
    mEngineAPI.Node3D_SetPosition = PluginNode3D_SetPosition;
    mEngineAPI.Node3D_GetScale = PluginNode3D_GetScale;
    mEngineAPI.Node3D_SetScale = PluginNode3D_SetScale;

    // Asset System
    mEngineAPI.LoadAsset = PluginLoadAsset;
    mEngineAPI.FetchAsset = PluginFetchAsset;
    mEngineAPI.UnloadAsset = PluginUnloadAsset;

    // TinyLLM
    mEngineAPI.TinyLLM_Encode = PluginTinyLLM_Encode;
    mEngineAPI.TinyLLM_Decode = PluginTinyLLM_Decode;

    // Audio
    mEngineAPI.PlaySound2D = PluginPlaySound2D;
    mEngineAPI.StopAllSounds = PluginStopAllSounds;
    mEngineAPI.SetMasterVolume = PluginSetMasterVolume;
    mEngineAPI.GetMasterVolume = PluginGetMasterVolume;

    // Input
    mEngineAPI.IsKeyDown = PluginIsKeyDown;
    mEngineAPI.IsKeyJustPressed = PluginIsKeyJustPressed;
    mEngineAPI.IsKeyJustReleased = PluginIsKeyJustReleased;
    mEngineAPI.IsMouseButtonDown = PluginIsMouseButtonDown;
    mEngineAPI.IsMouseButtonJustPressed = PluginIsMouseButtonJustPressed;
    mEngineAPI.GetMousePosition = PluginGetMousePosition;
    mEngineAPI.GetMouseDelta = PluginGetMouseDelta;
    mEngineAPI.GetScrollWheelDelta = PluginGetScrollWheelDelta;

    // Time
    mEngineAPI.GetDeltaTime = PluginGetDeltaTime;
    mEngineAPI.GetElapsedTime = PluginGetElapsedTime;

    // Gizmos
    mEngineAPI.Gizmos_SetColor = PluginGizmos_SetColor;
    mEngineAPI.Gizmos_SetMatrix = PluginGizmos_SetMatrix;
    mEngineAPI.Gizmos_ResetState = PluginGizmos_ResetState;
    mEngineAPI.Gizmos_DrawCube = PluginGizmos_DrawCube;
    mEngineAPI.Gizmos_DrawWireCube = PluginGizmos_DrawWireCube;
    mEngineAPI.Gizmos_DrawSphere = PluginGizmos_DrawSphere;
    mEngineAPI.Gizmos_DrawWireSphere = PluginGizmos_DrawWireSphere;
    mEngineAPI.Gizmos_DrawLine = PluginGizmos_DrawLine;
    mEngineAPI.Gizmos_DrawRay = PluginGizmos_DrawRay;

    // Editor UI (will be set when EditorUIHookManager is initialized)
    mEngineAPI.editorUI = nullptr;

    // ImGui context for plugins
    mEngineAPI.GetImGuiContext = [](ImGuiPluginContext* outCtx) {
        GetImGuiPluginContext(outCtx);
    };
}

void NativeAddonManager::DiscoverNativeAddons()
{
    LogDebug("Discovering native addons...");

    // Clear existing states (but keep track of loaded modules)
    std::unordered_map<std::string, void*> loadedModules;
    for (auto& pair : mStates)
    {
        if (pair.second.mModuleHandle != nullptr)
        {
            loadedModules[pair.first] = pair.second.mModuleHandle;
        }
    }
    mStates.clear();

    // Scan both sources
    ScanLocalPackages();
    ScanInstalledAddons();

    // Restore loaded module handles
    for (auto& pair : loadedModules)
    {
        auto it = mStates.find(pair.first);
        if (it != mStates.end())
        {
            it->second.mModuleHandle = pair.second;
        }
    }

    LogDebug("Discovered %zu native addons", mStates.size());
}

void NativeAddonManager::ScanLocalPackages()
{
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    if (projectDir.empty())
    {
        return;
    }

    std::string packagesDir = projectDir + "Packages/";
    if (!DoesDirExist(packagesDir.c_str()))
    {
        return;
    }

    DirEntry dirEntry;
    SYS_OpenDirectory(packagesDir, dirEntry);

    while (dirEntry.mValid)
    {
        // Use strcmp for char[] comparison (not pointer comparison)
        if (dirEntry.mDirectory &&
            strcmp(dirEntry.mFilename, ".") != 0 &&
            strcmp(dirEntry.mFilename, "..") != 0)
        {
            std::string addonPath = packagesDir + dirEntry.mFilename + "/";
            std::string packageJsonPath = addonPath + "package.json";

            if (SYS_DoesFileExist(packageJsonPath.c_str(), false))
            {
                NativeModuleMetadata metadata;
                if (ParsePackageJson(packageJsonPath, metadata) && metadata.mHasNative)
                {
                    NativeAddonState state;
                    state.mAddonId = dirEntry.mFilename;
                    state.mSourcePath = addonPath;
                    state.mNativeMetadata = metadata;

                    mStates[state.mAddonId] = state;
                    LogDebug("Found local native addon: %s", state.mAddonId.c_str());

                    // Update IDE config (ensures paths are correct if engine moved)
                    GenerateIDEConfig(addonPath);
                }
            }
        }

        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);
}

void NativeAddonManager::ScanInstalledAddons()
{
    AddonManager* addonMgr = AddonManager::Get();
    if (addonMgr == nullptr)
    {
        return;
    }

    const std::vector<InstalledAddon>& installed = addonMgr->GetInstalledAddons();
    std::string cacheDir = addonMgr->GetAddonCacheDirectory();

    for (const InstalledAddon& inst : installed)
    {
        // Skip if already found in local packages
        if (mStates.find(inst.mId) != mStates.end())
        {
            continue;
        }

        // Check cache for this addon
        std::string addonCachePath = cacheDir + "/" + inst.mId + "/";
        std::string packageJsonPath = addonCachePath + "package.json";

        if (SYS_DoesFileExist(packageJsonPath.c_str(), false))
        {
            NativeModuleMetadata metadata;
            if (ParsePackageJson(packageJsonPath, metadata) && metadata.mHasNative)
            {
                NativeAddonState state;
                state.mAddonId = inst.mId;
                state.mSourcePath = addonCachePath;
                state.mNativeMetadata = metadata;

                mStates[state.mAddonId] = state;
                LogDebug("Found installed native addon: %s", state.mAddonId.c_str());
            }
        }
    }
}

bool NativeAddonManager::ParsePackageJson(const std::string& path, NativeModuleMetadata& outMetadata)
{
    Stream stream;
    if (!stream.ReadFile(path.c_str(), false))
    {
        return false;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        return false;
    }

    if (!doc.HasMember("native") || !doc["native"].IsObject())
    {
        return false;
    }

    const rapidjson::Value& native = doc["native"];
    outMetadata.mHasNative = true;

    if (native.HasMember("target") && native["target"].IsString())
    {
        std::string target = native["target"].GetString();
        outMetadata.mTarget = (target == "editor") ?
            NativeAddonTarget::EditorOnly : NativeAddonTarget::EngineAndEditor;
    }

    if (native.HasMember("sourceDir") && native["sourceDir"].IsString())
    {
        outMetadata.mSourceDir = native["sourceDir"].GetString();
    }

    if (native.HasMember("binaryName") && native["binaryName"].IsString())
    {
        outMetadata.mBinaryName = native["binaryName"].GetString();
    }

    if (native.HasMember("entrySymbol") && native["entrySymbol"].IsString())
    {
        outMetadata.mEntrySymbol = native["entrySymbol"].GetString();
    }

    if (native.HasMember("apiVersion") && native["apiVersion"].IsUint())
    {
        outMetadata.mPluginApiVersion = native["apiVersion"].GetUint();
    }

    if (native.HasMember("exportDefine") && native["exportDefine"].IsString())
    {
        outMetadata.mExportDefine = native["exportDefine"].GetString();
    }

    // Parse dependencies (other native addon IDs this addon depends on)
    if (native.HasMember("dependencies") && native["dependencies"].IsArray())
    {
        const rapidjson::Value& deps = native["dependencies"];
        for (rapidjson::SizeType i = 0; i < deps.Size(); ++i)
        {
            if (deps[i].IsString())
            {
                outMetadata.mDependencies.push_back(deps[i].GetString());
            }
        }
    }

    return true;
}

std::vector<std::string> NativeAddonManager::GetDiscoveredAddonIds() const
{
    std::vector<std::string> ids;
    ids.reserve(mStates.size());
    for (const auto& pair : mStates)
    {
        ids.push_back(pair.first);
    }
    return ids;
}

std::string NativeAddonManager::ComputeFingerprint(const std::string& addonId)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        return "";
    }

    const NativeAddonState& state = it->second;
    std::string sourceDir = state.mSourcePath + state.mNativeMetadata.mSourceDir + "/";

    if (!DoesDirExist(sourceDir.c_str()))
    {
        return "";
    }

    // Gather all source files and compute hash from their mtimes and sizes
    std::vector<std::string> sourceFiles = GatherSourceFiles(sourceDir);
    if (sourceFiles.empty())
    {
        return "";
    }

    std::sort(sourceFiles.begin(), sourceFiles.end());

    uint64_t hash = 0;
    for (const std::string& file : sourceFiles)
    {
        // Simple hash using file path and mtime
        // In a real implementation, you might want to use actual file content hashing
        for (char c : file)
        {
            hash = hash * 31 + c;
        }

        // Add file size as part of fingerprint
        Stream stream;
        if (stream.ReadFile(file.c_str(), false))
        {
            hash = hash * 31 + stream.GetSize();
        }
    }

    char fingerprint[32];
    snprintf(fingerprint, sizeof(fingerprint), "%016llx", (unsigned long long)hash);
    return fingerprint;
}

std::vector<std::string> NativeAddonManager::GatherSourceFiles(const std::string& sourceDir)
{
    std::vector<std::string> files;

    std::function<void(const std::string&)> scanDir;
    scanDir = [&](const std::string& dir)
    {
        DirEntry dirEntry;
        SYS_OpenDirectory(dir, dirEntry);

        if (!dirEntry.mValid)
        {
            // Directory doesn't exist or can't be opened
            return;
        }

        while (dirEntry.mValid)
        {
            // Use strcmp for char[] comparison (not pointer comparison)
            if (strcmp(dirEntry.mFilename, ".") != 0 && strcmp(dirEntry.mFilename, "..") != 0)
            {
                std::string path = dir + dirEntry.mFilename;

                if (dirEntry.mDirectory)
                {
                    scanDir(path + "/");
                }
                else
                {
                    // Check for C++ source files
                    std::string filename = dirEntry.mFilename;
                    size_t dotPos = filename.find_last_of('.');
                    if (dotPos != std::string::npos)
                    {
                        std::string ext = filename.substr(dotPos);
                        if (ext == ".cpp" || ext == ".c" || ext == ".h" || ext == ".hpp")
                        {
                            files.push_back(path);
                        }
                    }
                }
            }

            SYS_IterateDirectory(dirEntry);
        }
        SYS_CloseDirectory(dirEntry);
    };

    scanDir(sourceDir);
    return files;
}

bool NativeAddonManager::NeedsBuild(const std::string& addonId)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        return false;
    }

    std::string currentFingerprint = ComputeFingerprint(addonId);
    if (currentFingerprint.empty())
    {
        return false;
    }

    // Check if output exists with current fingerprint
    std::string outputPath = GetOutputPath(addonId, currentFingerprint);
    return !SYS_DoesFileExist(outputPath.c_str(), false);
}

std::string NativeAddonManager::GetIntermediateDir(const std::string& addonId)
{
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    return projectDir + "Intermediate/Plugins/" + addonId + "/";
}

std::string NativeAddonManager::GetOutputPath(const std::string& addonId, const std::string& fingerprint)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        return "";
    }

    std::string intermediateDir = GetIntermediateDir(addonId);
    std::string binaryName = it->second.mNativeMetadata.mBinaryName;
    if (binaryName.empty())
    {
        binaryName = addonId;
    }

#if PLATFORM_WINDOWS
    return intermediateDir + fingerprint + "/" + binaryName + ".dll";
#else
    return intermediateDir + fingerprint + "/lib" + binaryName + ".so";
#endif
}

bool NativeAddonManager::GenerateBuildScript(const std::string& addonId,
                                             const std::string& outputDir,
                                             const std::string& outputPath,
                                             std::string& outScriptPath)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        return false;
    }

    const NativeAddonState& state = it->second;
    std::string sourceDir = state.mSourcePath + state.mNativeMetadata.mSourceDir + "/";
    std::string octavePath = SYS_GetOctavePath();

    // Try to load from manifest, fall back to hardcoded paths
    std::vector<std::string> includePaths;
    std::vector<std::string> defines;

    if (!LoadAddonIncludesManifest(includePaths, defines))
    {
        // Fallback to hardcoded paths
        includePaths = {
            "Engine/Source",
            "Engine/Source/Engine",
            "Engine/Source/Plugins",
            "External/Lua",
            "External/glm",
            "External/Imgui",
            "External/ImGuizmo",
            "External/bullet3/src",
            "External/Assimp",
            "External"
        };
        defines = {
            "OCTAVE_PLUGIN_EXPORT",
            "EDITOR=1",
            "LUA_ENABLED=1",
            "GLM_FORCE_RADIANS",
#if PLATFORM_WINDOWS
            "PLATFORM_WINDOWS=1",
            "API_VULKAN=1",
            "NOMINMAX"
#elif PLATFORM_LINUX
            "PLATFORM_LINUX=1",
            "API_VULKAN=1"
#elif PLATFORM_ANDROID
            "PLATFORM_ANDROID=1",
            "API_VULKAN=1"
#elif PLATFORM_3DS
            "PLATFORM_3DS=1",
            "API_C3D=1"
#endif
        };
    }

    // Create output directory (recursively to handle missing parent dirs)
    if (!DoesDirExist(outputDir.c_str()))
    {
        CreateDirectoryRecursive(outputDir);
    }

    std::vector<std::string> sourceFiles = GatherSourceFiles(sourceDir);

    // Get parent Packages directory for resolving sibling addon dependencies
    std::string packagesDir;
    {
        std::string path = state.mSourcePath;
        while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
        {
            path.pop_back();
        }
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            packagesDir = path.substr(0, lastSlash + 1);
        }
    }

#if PLATFORM_WINDOWS
    // Generate a batch file for Windows
    outScriptPath = outputDir + "build.bat";

    std::stringstream ss;
    ss << "@echo off\n";
    ss << "setlocal\n";
    ss << "\n";
    ss << ":: Find Visual Studio\n";
    ss << "set \"VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\n";
    ss << "for /f \"usebackq tokens=*\" %%i in (`\"%VSWHERE%\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (\n";
    ss << "  set \"VS_PATH=%%i\"\n";
    ss << ")\n";
    ss << "\n";
    ss << "if not defined VS_PATH (\n";
    ss << "  echo ERROR: Visual Studio not found\n";
    ss << "  exit /b 1\n";
    ss << ")\n";
    ss << "\n";
    ss << "call \"%VS_PATH%\\VC\\Auxiliary\\Build\\vcvars64.bat\" >nul 2>&1\n";
    ss << "\n";
    ss << ":: Compile\n";
    ss << "cl.exe /nologo /EHsc /O2 /LD /MD ";

    // Add defines from manifest
    for (const std::string& define : defines)
    {
        ss << "/D" << define << " ";
    }

    // Add export macro for this plugin
    std::string exportMacro = state.mNativeMetadata.mExportDefine.empty()
        ? GenerateExportMacroName(state.mAddonId)
        : state.mNativeMetadata.mExportDefine;
    ss << "/D" << exportMacro << " ";

    // Add include paths from manifest
    for (const std::string& path : includePaths)
    {
        ss << "/I\"" << octavePath << path << "/\" ";
    }
    ss << "/I\"" << sourceDir << "\" ";

    // Add dependency addon Source directories
    for (const std::string& depId : state.mNativeMetadata.mDependencies)
    {
        ss << "/I\"" << packagesDir << depId << "/Source/\" ";
    }

    // Add Vulkan SDK include path
    ss << "/I\"%VULKAN_SDK%/Include\" ";

    // Add source files
    for (const std::string& src : sourceFiles)
    {
        if (src.find(".cpp") != std::string::npos || src.find(".c") != std::string::npos)
        {
            ss << "\"" << src << "\" ";
        }
    }

    ss << "/Fe\"" << outputPath << "\" ";
    ss << "/link /DLL ";

    // Get executable directory for installed editor lib paths
    std::string exePath = SYS_GetExecutablePath();
    std::string exeDir;
    {
        size_t lastSlash = exePath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            exeDir = exePath.substr(0, lastSlash + 1);
        }
    }

    // Link against Octave import library and Lua library
    // First check installed editor paths (alongside executable)
    ss << "/LIBPATH:\"" << exeDir << "\" ";
    ss << "/LIBPATH:\"" << exeDir << "lib/\" ";
    // Then check development build paths
    ss << "/LIBPATH:\"" << octavePath << "/Standalone/Build/Windows/x64/DebugEditor/\" ";
    ss << "/LIBPATH:\"" << octavePath << "/Standalone/Build/Windows/x64/ReleaseEditor/\" ";
    ss << "/LIBPATH:\"" << octavePath << "/External/Lua/Build/Windows/x64/DebugEditor/\" ";
    ss << "/LIBPATH:\"" << octavePath << "/External/Lua/Build/Windows/x64/ReleaseEditor/\" ";

    // Add dependency addon library paths and libraries
    for (const std::string& depId : state.mNativeMetadata.mDependencies)
    {
        ss << "/LIBPATH:\"" << packagesDir << depId << "/Build/Debug/\" ";
        ss << "/LIBPATH:\"" << packagesDir << depId << "/Build/Release/\" ";
    }
    ss << "Octave.lib Lua.lib ";
    for (const std::string& depId : state.mNativeMetadata.mDependencies)
    {
        ss << GenerateLibraryName(depId) << ".lib ";
    }
    ss << "\n";
    ss << "\n";
    ss << "if %ERRORLEVEL% neq 0 (\n";
    ss << "  echo Build failed\n";
    ss << "  exit /b 1\n";
    ss << ")\n";
    ss << "\n";
    ss << "echo Build succeeded\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    stream.WriteFile(outScriptPath.c_str());

#else
    // Generate a shell script for Linux
    outScriptPath = outputDir + "build.sh";

    std::stringstream ss;
    ss << "#!/bin/bash\n";
    ss << "set -e\n";
    ss << "\n";
    ss << "g++ -shared -fPIC -O2 \\\n";

    // Add defines from manifest
    for (const std::string& define : defines)
    {
        ss << "  -D" << define << " \\\n";
    }

    // Add export macro for this plugin
    std::string exportMacroLinux = state.mNativeMetadata.mExportDefine.empty()
        ? GenerateExportMacroName(state.mAddonId)
        : state.mNativeMetadata.mExportDefine;
    ss << "  -D" << exportMacroLinux << " \\\n";

    // Add include paths from manifest
    for (const std::string& path : includePaths)
    {
        ss << "  -I\"" << octavePath << path << "/\" \\\n";
    }
    ss << "  -I\"" << sourceDir << "\" \\\n";

    // Add dependency addon Source directories (packagesDir already computed above for Windows)
    for (const std::string& depId : state.mNativeMetadata.mDependencies)
    {
        ss << "  -I\"" << packagesDir << depId << "/Source/\" \\\n";
    }

    // Add Vulkan SDK include path
    ss << "  -I\"$VULKAN_SDK/include\" \\\n";

    // Add source files
    for (const std::string& src : sourceFiles)
    {
        if (src.find(".cpp") != std::string::npos || src.find(".c") != std::string::npos)
        {
            ss << "  \"" << src << "\" \\\n";
        }
    }

    // Link against Lua library
    std::string luaLibPathLinux;
    std::vector<std::string> luaConfigsLinux = {"DebugEditor", "ReleaseEditor", "Debug", "Release"};
    for (const std::string& config : luaConfigsLinux)
    {
        std::string testPath = octavePath + "External/Lua/Build/Linux/x64/" + config + "/libLua.a";
        if (SYS_DoesFileExist(testPath.c_str(), false))
        {
            luaLibPathLinux = testPath;
            break;
        }
    }

    if (!luaLibPathLinux.empty())
    {
        ss << "  \"" << luaLibPathLinux << "\" \\\n";
    }
    else
    {
        // Try system Lua as fallback
        ss << "  -llua \\\n";
    }

    // Link against dependency shared libraries
    for (const std::string& depId : state.mNativeMetadata.mDependencies)
    {
        std::string depLibName = GenerateLibraryName(depId);
        ss << "  -L\"" << packagesDir << depId << "/Build/\" \\\n";
        ss << "  -l" << depLibName << " \\\n";
    }

    // Allow unresolved symbols - ImGui symbols will be resolved at runtime from the editor executable
    ss << "  -Wl,--unresolved-symbols=ignore-in-shared-libs \\\n";
    ss << "  -o \"" << outputPath << "\"\n";
    ss << "\n";
    ss << "echo \"Build succeeded\"\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    stream.WriteFile(outScriptPath.c_str());

    // Make executable
    std::string chmodCmd = "chmod +x \"" + outScriptPath + "\"";
    SYS_Exec(chmodCmd.c_str(), nullptr);
#endif

    return true;
}

bool NativeAddonManager::BuildNativeAddon(const std::string& addonId, std::string& outError)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        outError = "Addon not found: " + addonId;
        return false;
    }

    NativeAddonState& state = it->second;
    state.mBuildInProgress = true;
    state.mBuildLog.clear();
    state.mBuildError.clear();

    LogDebug("Building native addon: %s", addonId.c_str());

    // Compute fingerprint
    std::string fingerprint = ComputeFingerprint(addonId);
    if (fingerprint.empty())
    {
        outError = "Failed to compute fingerprint";
        state.mBuildInProgress = false;
        state.mBuildSucceeded = false;
        state.mBuildError = outError;
        return false;
    }

    // Get output paths
    std::string intermediateDir = GetIntermediateDir(addonId);
    std::string outputDir = intermediateDir + fingerprint + "/";
    std::string outputPath = GetOutputPath(addonId, fingerprint);

    // Create directories (recursively to handle missing parent dirs)
    if (!CreateDirectoryRecursive(outputDir))
    {
        outError = "Failed to create output directory: " + outputDir;
        state.mBuildInProgress = false;
        state.mBuildSucceeded = false;
        state.mBuildError = outError;
        return false;
    }

    // Generate build script
    std::string scriptPath;
    if (!GenerateBuildScript(addonId, outputDir, outputPath, scriptPath))
    {
        outError = "Failed to generate build script";
        state.mBuildInProgress = false;
        state.mBuildSucceeded = false;
        state.mBuildError = outError;
        return false;
    }

    // Execute build
    std::string stdoutStr;
    int exitCode = 0;

#if PLATFORM_WINDOWS
    std::string cmd = "cmd /c \"" + scriptPath + "\"";
#else
    std::string cmd = "bash \"" + scriptPath + "\"";
#endif

    bool success = SYS_ExecFull(cmd.c_str(), &stdoutStr, nullptr, &exitCode);

    state.mBuildLog = stdoutStr;
    state.mBuildInProgress = false;

    if (!success || exitCode != 0)
    {
        outError = "Build failed with exit code " + std::to_string(exitCode);
        state.mBuildSucceeded = false;
        state.mBuildError = outError + "\n" + stdoutStr;
        LogError("Build failed for %s: %s", addonId.c_str(), stdoutStr.c_str());
        return false;
    }

    // Verify output exists
    if (!SYS_DoesFileExist(outputPath.c_str(), false))
    {
        outError = "Build completed but output file not found: " + outputPath;
        state.mBuildSucceeded = false;
        state.mBuildError = outError;
        return false;
    }

    state.mBuildSucceeded = true;
    state.mFingerprint = fingerprint;
    LogDebug("Build succeeded for %s", addonId.c_str());

    return true;
}

bool NativeAddonManager::LoadNativeAddon(const std::string& addonId, std::string& outError)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        outError = "Addon not found: " + addonId;
        return false;
    }

    NativeAddonState& state = it->second;

    // Already loaded?
    if (state.mModuleHandle != nullptr)
    {
        outError = "Addon already loaded";
        return false;
    }

    // Build if needed
    if (NeedsBuild(addonId))
    {
        if (!BuildNativeAddon(addonId, outError))
        {
            return false;
        }
    }

    // Get output path
    std::string fingerprint = state.mFingerprint;
    if (fingerprint.empty())
    {
        fingerprint = ComputeFingerprint(addonId);
    }

    std::string modulePath = GetOutputPath(addonId, fingerprint);
    if (!SYS_DoesFileExist(modulePath.c_str(), false))
    {
        outError = "Module file not found: " + modulePath;
        return false;
    }

    // Load the module
    LogDebug("Loading native addon: %s from %s", addonId.c_str(), modulePath.c_str());

    void* handle = MOD_Load(modulePath.c_str());
    if (handle == nullptr)
    {
        outError = "Failed to load module: " + std::string(MOD_GetError());
        return false;
    }

    // Get entry point
    OctavePlugin_GetDescFunc getDesc = (OctavePlugin_GetDescFunc)MOD_Symbol(handle, state.mNativeMetadata.mEntrySymbol.c_str());
    if (getDesc == nullptr)
    {
        MOD_Unload(handle);
        outError = "Entry symbol not found: " + state.mNativeMetadata.mEntrySymbol;
        return false;
    }

    // Get plugin descriptor
    OctavePluginDesc desc = {};
    if (getDesc(&desc) != 0)
    {
        MOD_Unload(handle);
        outError = "Failed to get plugin descriptor";
        return false;
    }

    // Verify API version (accept version 1 or 2 for backward compatibility)
    if (desc.apiVersion < 1 || desc.apiVersion > OCTAVE_PLUGIN_API_VERSION)
    {
        MOD_Unload(handle);
        outError = "API version mismatch: plugin=" + std::to_string(desc.apiVersion) +
                   ", max supported=" + std::to_string(OCTAVE_PLUGIN_API_VERSION);
        return false;
    }

    // For v1 plugins, zero out the v2 fields they don't know about
    if (desc.apiVersion < 2)
    {
        desc.OnEditorPreInit = nullptr;
        desc.OnEditorReady = nullptr;
    }

    // Call OnLoad
    if (desc.OnLoad != nullptr)
    {
        int result = desc.OnLoad(&mEngineAPI);
        if (result != 0)
        {
            MOD_Unload(handle);
            outError = "Plugin OnLoad failed with code " + std::to_string(result);
            return false;
        }
    }

    // Register types if provided
    if (desc.RegisterTypes != nullptr)
    {
        // TODO: Pass actual node factory
        desc.RegisterTypes(nullptr);
    }

    // Register script functions if provided
    if (desc.RegisterScriptFuncs != nullptr)
    {
        desc.RegisterScriptFuncs(GetLua());
    }

    // Register editor UI if provided and in editor
    if (desc.RegisterEditorUI != nullptr && mEngineAPI.editorUI != nullptr)
    {
        // Generate hook ID from addon ID
        uint64_t hookId = 0;
        for (char c : addonId)
        {
            hookId = hookId * 31 + c;
        }
        desc.RegisterEditorUI(mEngineAPI.editorUI, hookId);
    }

    // Store state
    state.mModuleHandle = handle;
    state.mLoadedPath = modulePath;
    state.mDesc = desc;
    state.mDescValid = true;
    state.mFingerprint = fingerprint;

    LogDebug("Successfully loaded native addon: %s (v%s)", desc.pluginName, desc.pluginVersion);

    return true;
}

bool NativeAddonManager::UnloadNativeAddon(const std::string& addonId)
{
    auto it = mStates.find(addonId);
    if (it == mStates.end())
    {
        return false;
    }

    NativeAddonState& state = it->second;

    if (state.mModuleHandle == nullptr)
    {
        return true;  // Already unloaded
    }

    LogDebug("Unloading native addon: %s", addonId.c_str());

    // Call OnUnload
    if (state.mDescValid && state.mDesc.OnUnload != nullptr)
    {
        state.mDesc.OnUnload();
    }

    // Unload module
    MOD_Unload(state.mModuleHandle);

    state.mModuleHandle = nullptr;
    state.mLoadedPath.clear();
    state.mDescValid = false;
    state.mDesc = {};

    return true;
}

bool NativeAddonManager::ReloadNativeAddon(const std::string& addonId, std::string& outError)
{
    LogDebug("Reloading native addon: %s", addonId.c_str());

    // Unload first
    UnloadNativeAddon(addonId);

    // Load again (will rebuild if needed)
    return LoadNativeAddon(addonId, outError);
}

void NativeAddonManager::ReloadAllNativeAddons()
{
    LogDebug("Reloading all native addons...");

    // Discover addons first
    DiscoverNativeAddons();

    // Get list of addons that should be loaded
    AddonManager* addonMgr = AddonManager::Get();
    const std::vector<InstalledAddon>& installed = addonMgr ? addonMgr->GetInstalledAddons() : std::vector<InstalledAddon>();

    // Build set of enabled native addons
    std::vector<std::string> toLoad;

    // Local packages are always loaded
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    std::string packagesDir = projectDir + "Packages/";

    for (const auto& pair : mStates)
    {
        const std::string& addonId = pair.first;
        const NativeAddonState& state = pair.second;

        // Check if this is a local package
        bool isLocal = state.mSourcePath.find(packagesDir) == 0;

        if (isLocal)
        {
            toLoad.push_back(addonId);
        }
        else
        {
            // Check if installed and native enabled
            for (const InstalledAddon& inst : installed)
            {
                if (inst.mId == addonId && inst.mEnabled && inst.mEnableNative)
                {
                    toLoad.push_back(addonId);
                    break;
                }
            }
        }
    }

    // Reload each addon
    for (const std::string& addonId : toLoad)
    {
        std::string error;
        if (!ReloadNativeAddon(addonId, error))
        {
            LogWarning("Failed to reload native addon %s: %s", addonId.c_str(), error.c_str());
        }
    }

    // Call OnEditorPreInit on newly loaded plugins
    CallOnEditorPreInit();

    LogDebug("Finished reloading native addons");
}

void NativeAddonManager::TickAllPlugins(float deltaTime)
{
    for (auto& pair : mStates)
    {
        NativeAddonState& state = pair.second;

        // Only tick loaded plugins with a valid Tick callback
        if (state.mModuleHandle != nullptr &&
            state.mDescValid &&
            state.mDesc.Tick != nullptr)
        {
            state.mDesc.Tick(deltaTime);
        }
    }
}

void NativeAddonManager::TickEditorAllPlugins(float deltaTime)
{
    for (auto& pair : mStates)
    {
        NativeAddonState& state = pair.second;

        // Only tick loaded plugins with a valid TickEditor callback
        if (state.mModuleHandle != nullptr &&
            state.mDescValid &&
            state.mDesc.TickEditor != nullptr)
        {
            state.mDesc.TickEditor(deltaTime);
        }
    }
}

void NativeAddonManager::CallOnEditorPreInit()
{
    for (auto& pair : mStates)
    {
        NativeAddonState& state = pair.second;

        if (state.mModuleHandle != nullptr &&
            state.mDescValid &&
            state.mDesc.OnEditorPreInit != nullptr)
        {
            state.mDesc.OnEditorPreInit();
        }
    }
}

void NativeAddonManager::CallOnEditorReady()
{
    for (auto& pair : mStates)
    {
        NativeAddonState& state = pair.second;

        if (state.mModuleHandle != nullptr &&
            state.mDescValid &&
            state.mDesc.OnEditorReady != nullptr)
        {
            state.mDesc.OnEditorReady();
        }
    }
}

const NativeAddonState* NativeAddonManager::GetState(const std::string& addonId) const
{
    auto it = mStates.find(addonId);
    return (it != mStates.end()) ? &it->second : nullptr;
}

bool NativeAddonManager::IsLoaded(const std::string& addonId) const
{
    auto it = mStates.find(addonId);
    return (it != mStates.end()) && (it->second.mModuleHandle != nullptr);
}

std::string NativeAddonManager::GetAddonSourcePath(const std::string& addonId) const
{
    auto it = mStates.find(addonId);
    return (it != mStates.end()) ? it->second.mSourcePath : "";
}

std::vector<NativeAddonState> NativeAddonManager::GetEngineAddons() const
{
    std::vector<NativeAddonState> result;

    for (const auto& pair : mStates)
    {
        if (pair.second.mNativeMetadata.mTarget == NativeAddonTarget::EngineAndEditor)
        {
            result.push_back(pair.second);
        }
    }

    return result;
}

std::vector<std::string> NativeAddonManager::GetLocalPackageIds() const
{
    std::vector<std::string> result;

    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    if (projectDir.empty())
    {
        return result;
    }

    std::string packagesDir = projectDir + "Packages/";

    for (const auto& pair : mStates)
    {
        // Check if this addon is in the local Packages/ folder
        if (pair.second.mSourcePath.find(packagesDir) == 0)
        {
            result.push_back(pair.first);
        }
    }

    return result;
}

// ===== Manifest Generation =====

bool NativeAddonManager::GenerateAddonIncludesManifest()
{
    std::string octavePath = SYS_GetOctavePath();
    std::string generatedDir = octavePath + "Engine/Generated/";
    std::string outputPath = generatedDir + "AddonIncludes.json";

    // Ensure Generated directory exists
    if (!DoesDirExist(generatedDir.c_str()))
    {
        SYS_CreateDirectory(generatedDir.c_str());
    }

    std::stringstream ss;
    ss << "{\n";
    ss << "    \"version\": 1,\n";
    ss << "    \"includePaths\": [\n";
    ss << "        \"Engine/Source\",\n";
    ss << "        \"Engine/Source/Engine\",\n";
    ss << "        \"Engine/Source/Plugins\",\n";
    ss << "        \"External/Lua\",\n";
    ss << "        \"External/glm\",\n";
    ss << "        \"External/Imgui\",\n";
    ss << "        \"External/ImGuizmo\",\n";
    ss << "        \"External/bullet3/src\",\n";
    ss << "        \"External/Assimp\",\n";
    ss << "        \"External\"\n";
    ss << "    ],\n";
    ss << "    \"defines\": [\n";
    ss << "        \"OCTAVE_PLUGIN_EXPORT\",\n";
    ss << "        \"EDITOR=1\",\n";
    ss << "        \"LUA_ENABLED=1\",\n";
    ss << "        \"GLM_FORCE_RADIANS\",\n";
#if PLATFORM_WINDOWS
    ss << "        \"PLATFORM_WINDOWS=1\",\n";
    ss << "        \"API_VULKAN=1\",\n";
    ss << "        \"NOMINMAX\"\n";
#elif PLATFORM_LINUX
    ss << "        \"PLATFORM_LINUX=1\",\n";
    ss << "        \"API_VULKAN=1\"\n";
#elif PLATFORM_ANDROID
    ss << "        \"PLATFORM_ANDROID=1\",\n";
    ss << "        \"API_VULKAN=1\"\n";
#elif PLATFORM_3DS
    ss << "        \"PLATFORM_3DS=1\",\n";
    ss << "        \"API_C3D=1\"\n";
#endif
    ss << "    ]\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    bool success = stream.WriteFile(outputPath.c_str());

    if (success)
    {
        LogDebug("Generated AddonIncludes.json at %s", outputPath.c_str());
    }
    else
    {
        LogError("Failed to generate AddonIncludes.json");
    }

    return success;
}

bool NativeAddonManager::LoadAddonIncludesManifest(std::vector<std::string>& outIncludePaths,
                                                    std::vector<std::string>& outDefines)
{
    std::string octavePath = SYS_GetOctavePath();
    std::string manifestPath = octavePath + "Engine/Generated/AddonIncludes.json";

    // Check if manifest exists
    if (!SYS_DoesFileExist(manifestPath.c_str(), false))
    {
        return false;
    }

    // Read file
    Stream stream;
    if (!stream.ReadFile(manifestPath.c_str(), false))
    {
        return false;
    }

    // Parse JSON
    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        LogWarning("Failed to parse AddonIncludes.json");
        return false;
    }

    // Read include paths
    if (doc.HasMember("includePaths") && doc["includePaths"].IsArray())
    {
        const rapidjson::Value& paths = doc["includePaths"];
        for (rapidjson::SizeType i = 0; i < paths.Size(); i++)
        {
            if (paths[i].IsString())
            {
                outIncludePaths.push_back(paths[i].GetString());
            }
        }
    }

    // Read defines
    if (doc.HasMember("defines") && doc["defines"].IsArray())
    {
        const rapidjson::Value& defines = doc["defines"];
        for (rapidjson::SizeType i = 0; i < defines.Size(); i++)
        {
            if (defines[i].IsString())
            {
                outDefines.push_back(defines[i].GetString());
            }
        }
    }

    return true;
}

// ===== Creation and Packaging Implementation =====

std::string NativeAddonManager::GenerateIdFromName(const std::string& name)
{
    std::string id;
    id.reserve(name.size());

    bool lastWasHyphen = false;
    for (char c : name)
    {
        if (std::isalnum(c))
        {
            id += static_cast<char>(std::tolower(c));
            lastWasHyphen = false;
        }
        else if (c == ' ' || c == '_' || c == '-')
        {
            if (!lastWasHyphen && !id.empty())
            {
                id += '-';
                lastWasHyphen = true;
            }
        }
    }

    // Remove trailing hyphen
    while (!id.empty() && id.back() == '-')
    {
        id.pop_back();
    }

    return id;
}

bool NativeAddonManager::WriteTemplateSourceFile(const std::string& path,
                                                  const std::string& addonName,
                                                  const std::string& binaryName)
{
    // Generate a clean C++ identifier from addon name
    std::string className;
    bool capitalizeNext = true;
    for (char c : addonName)
    {
        if (std::isalnum(c))
        {
            if (capitalizeNext)
            {
                className += static_cast<char>(std::toupper(c));
                capitalizeNext = false;
            }
            else
            {
                className += c;
            }
        }
        else
        {
            capitalizeNext = true;
        }
    }

    if (className.empty())
    {
        className = "MyAddon";
    }

    std::stringstream ss;
    ss << "/**\n";
    ss << " * @file " << className << ".cpp\n";
    ss << " * @brief Native addon: " << addonName << "\n";
    ss << " */\n";
    ss << "\n";
    ss << "#include \"Plugins/OctavePluginAPI.h\"\n";
    ss << "#include \"Plugins/OctaveEngineAPI.h\"\n";
    ss << "\n";
    ss << "static OctaveEngineAPI* sEngineAPI = nullptr;\n";
    ss << "\n";
    ss << "static int OnLoad(OctaveEngineAPI* api)\n";
    ss << "{\n";
    ss << "    sEngineAPI = api;\n";
    ss << "    api->LogDebug(\"" << addonName << " loaded!\");\n";
    ss << "    return 0;\n";
    ss << "}\n";
    ss << "\n";
    ss << "static void OnUnload()\n";
    ss << "{\n";
    ss << "    if (sEngineAPI)\n";
    ss << "    {\n";
    ss << "        sEngineAPI->LogDebug(\"" << addonName << " unloaded.\");\n";
    ss << "    }\n";
    ss << "    sEngineAPI = nullptr;\n";
    ss << "}\n";
    ss << "\n";
    ss << "static void RegisterTypes(void* nodeFactory)\n";
    ss << "{\n";
    ss << "    // Register custom node types here\n";
    ss << "    // Example: REGISTER_NODE(MyCustomNode);\n";
    ss << "}\n";
    ss << "\n";
    ss << "static void RegisterScriptFuncs(lua_State* L)\n";
    ss << "{\n";
    ss << "    // Register Lua functions here\n";
    ss << "    // Use L to interact with Lua state\n";
    ss << "    (void)L; // Suppress unused parameter warning\n";
    ss << "}\n";
    ss << "\n";
    ss << "#if EDITOR\n";
    ss << "static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)\n";
    ss << "{\n";
    ss << "    // Register editor UI extensions here\n";
    ss << "    // Example:\n";
    ss << "    // hooks->AddMenuItem(hookId, \"Developer\", \"" << addonName << " Tool\",\n";
    ss << "    //     [](void*) { /* do something */ }, nullptr, nullptr);\n";
    ss << "}\n";
    ss << "#endif\n";
    ss << "\n";
    ss << "extern \"C\" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)\n";
    ss << "{\n";
    ss << "    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;\n";
    ss << "    desc->pluginName = \"" << addonName << "\";\n";
    ss << "    desc->pluginVersion = \"1.0.0\";\n";
    ss << "    desc->OnLoad = OnLoad;\n";
    ss << "    desc->OnUnload = OnUnload;\n";
    ss << "    desc->RegisterTypes = RegisterTypes;\n";
    ss << "    desc->RegisterScriptFuncs = RegisterScriptFuncs;\n";
    ss << "#if EDITOR\n";
    ss << "    desc->RegisterEditorUI = RegisterEditorUI;\n";
    ss << "#else\n";
    ss << "    desc->RegisterEditorUI = nullptr;\n";
    ss << "#endif\n";
    ss << "    desc->OnEditorPreInit = nullptr;\n";
    ss << "    desc->OnEditorReady = nullptr;\n";
    ss << "    return 0;\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    return stream.WriteFile(path.c_str());
}

bool NativeAddonManager::WritePackageJson(const std::string& path, const NativeAddonCreateInfo& info)
{
    std::string targetStr = (info.mTarget == NativeAddonTarget::EditorOnly) ? "editor" : "engine";

    std::stringstream ss;
    ss << "{\n";
    ss << "    \"name\": \"" << info.mName << "\",\n";
    ss << "    \"author\": \"" << info.mAuthor << "\",\n";
    ss << "    \"description\": \"" << info.mDescription << "\",\n";
    ss << "    \"version\": \"" << info.mVersion << "\",\n";
    ss << "    \"native\": {\n";
    ss << "        \"target\": \"" << targetStr << "\",\n";
    ss << "        \"sourceDir\": \"Source\",\n";
    ss << "        \"binaryName\": \"" << info.mBinaryName << "\",\n";
    ss << "        \"entrySymbol\": \"OctavePlugin_GetDesc\",\n";
    ss << "        \"apiVersion\": 1\n";
    ss << "    }\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    return stream.WriteFile(path.c_str());
}

bool NativeAddonManager::WriteVSCodeConfig(const std::string& addonPath)
{
    std::string vscodeDir = addonPath + ".vscode/";
    if (!DoesDirExist(vscodeDir.c_str()))
    {
        SYS_CreateDirectory(vscodeDir.c_str());
    }

    std::string octavePath = SYS_GetOctavePath();

    // Helper lambda to normalize paths for JSON (use forward slashes)
    auto normalizePath = [](const std::string& path) -> std::string {
        std::string result;
        for (char c : path)
        {
            if (c == '\\')
                result += '/';
            else
                result += c;
        }
        return result;
    };

    std::string octavePathJson = normalizePath(octavePath);

    // Try to load from manifest, fall back to hardcoded paths
    std::vector<std::string> includePaths;
    std::vector<std::string> defines;

    if (!LoadAddonIncludesManifest(includePaths, defines))
    {
        // Fallback to hardcoded paths
        includePaths = {
            "Engine/Source",
            "Engine/Source/Engine",
            "Engine/Source/Plugins",
            "External/Lua",
            "External/glm",
            "External/Imgui",
            "External/ImGuizmo",
            "External/bullet3/src",
            "External/Assimp",
            "External"
        };
        defines = {
            "OCTAVE_PLUGIN_EXPORT",
            "EDITOR=1",
            "LUA_ENABLED=1",
            "GLM_FORCE_RADIANS",
#if PLATFORM_WINDOWS
            "PLATFORM_WINDOWS=1",
            "API_VULKAN=1",
            "NOMINMAX"
#elif PLATFORM_LINUX
            "PLATFORM_LINUX=1",
            "API_VULKAN=1"
#elif PLATFORM_ANDROID
            "PLATFORM_ANDROID=1",
            "API_VULKAN=1"
#elif PLATFORM_3DS
            "PLATFORM_3DS=1",
            "API_C3D=1"
#endif
        };
    }

    // Parse package.json for dependencies
    std::string packageJsonPath = addonPath + "package.json";
    NativeModuleMetadata metadata;
    ParsePackageJson(packageJsonPath, metadata);

    // Get parent Packages directory and addon ID for resolving sibling addon dependencies
    std::string packagesDir;
    std::string addonId;
    {
        std::string path = addonPath;
        while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
        {
            path.pop_back();
        }
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            packagesDir = path.substr(0, lastSlash + 1);
            addonId = path.substr(lastSlash + 1);
        }
    }
    std::string packagesDirJson = normalizePath(packagesDir);

    std::stringstream ss;
    ss << "{\n";
    ss << "    \"configurations\": [\n";
    ss << "        {\n";
    ss << "            \"name\": \"Octave Addon\",\n";
    ss << "            \"includePath\": [\n";
    ss << "                \"${workspaceFolder}/**\"";

    // Add include paths from manifest
    for (const std::string& path : includePaths)
    {
        ss << ",\n                \"" << octavePathJson << path << "\"";
    }
    // Add dependency addon Source directories
    for (const std::string& depId : metadata.mDependencies)
    {
        ss << ",\n                \"" << packagesDirJson << depId << "/Source\"";
    }
    // Add Vulkan SDK include path
#if PLATFORM_WINDOWS
    ss << ",\n                \"${env:VULKAN_SDK}/Include\"";
#else
    ss << ",\n                \"${env:VULKAN_SDK}/include\"";
#endif
    ss << "\n            ],\n";

    ss << "            \"defines\": [";
    bool firstDefine = true;
    for (const std::string& define : defines)
    {
        if (!firstDefine) ss << ",";
        ss << "\n                \"" << define << "\"";
        firstDefine = false;
    }
    // Add export macro for this plugin
    std::string exportMacroJson = metadata.mExportDefine.empty()
        ? GenerateExportMacroName(addonId)
        : metadata.mExportDefine;
    ss << ",\n                \"" << exportMacroJson << "\"";
    ss << "\n            ],\n";

    ss << "            \"cStandard\": \"c17\",\n";
    ss << "            \"cppStandard\": \"c++17\",\n";
#if PLATFORM_WINDOWS
    ss << "            \"intelliSenseMode\": \"windows-msvc-x64\"\n";
#else
    ss << "            \"intelliSenseMode\": \"linux-gcc-x64\"\n";
#endif
    ss << "        }\n";
    ss << "    ],\n";
    ss << "    \"version\": 4\n";
    ss << "}\n";

    std::string configPath = vscodeDir + "c_cpp_properties.json";
    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    return stream.WriteFile(configPath.c_str());
}

bool NativeAddonManager::WriteCMakeLists(const std::string& addonPath, const std::string& binaryName)
{
    std::string octavePath = SYS_GetOctavePath();

    // Helper lambda to normalize paths for CMake (use forward slashes)
    auto normalizePath = [](const std::string& path) -> std::string {
        std::string result;
        for (char c : path)
        {
            if (c == '\\')
                result += '/';
            else
                result += c;
        }
        return result;
    };

    std::string octavePathCMake = normalizePath(octavePath);

    // Try to load from manifest, fall back to hardcoded paths
    std::vector<std::string> includePaths;
    std::vector<std::string> defines;

    if (!LoadAddonIncludesManifest(includePaths, defines))
    {
        // Fallback to hardcoded paths
        includePaths = {
            "Engine/Source",
            "Engine/Source/Engine",
            "Engine/Source/Plugins",
            "External/Lua",
            "External/glm",
            "External/Imgui",
            "External/ImGuizmo",
            "External/bullet3/src",
            "External/Assimp",
            "External"
        };
        defines = {
            "OCTAVE_PLUGIN_EXPORT",
            "EDITOR=1",
            "LUA_ENABLED=1",
            "GLM_FORCE_RADIANS",
#if PLATFORM_WINDOWS
            "PLATFORM_WINDOWS=1",
            "API_VULKAN=1",
            "NOMINMAX"
#elif PLATFORM_LINUX
            "PLATFORM_LINUX=1",
            "API_VULKAN=1"
#elif PLATFORM_ANDROID
            "PLATFORM_ANDROID=1",
            "API_VULKAN=1"
#elif PLATFORM_3DS
            "PLATFORM_3DS=1",
            "API_C3D=1"
#endif
        };
    }

    std::stringstream ss;
    ss << "cmake_minimum_required(VERSION 3.15)\n";
    ss << "project(" << binaryName << ")\n";
    ss << "\n";
    ss << "set(CMAKE_CXX_STANDARD 17)\n";
    ss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    ss << "\n";
    ss << "# Octave Engine path\n";
    ss << "set(OCTAVE_PATH \"" << octavePathCMake << "\")\n";
    ss << "\n";
    ss << "# Gather source files\n";
    ss << "file(GLOB_RECURSE SOURCES \"Source/*.cpp\" \"Source/*.c\")\n";
    ss << "file(GLOB_RECURSE HEADERS \"Source/*.h\" \"Source/*.hpp\")\n";
    ss << "\n";
    ss << "# Create shared library\n";
    ss << "add_library(" << binaryName << " SHARED ${SOURCES} ${HEADERS})\n";
    ss << "\n";
    ss << "# Find Vulkan SDK\n";
    ss << "if(DEFINED ENV{VULKAN_SDK})\n";
    ss << "    set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})\n";
    ss << "else()\n";
    ss << "    find_package(Vulkan QUIET)\n";
    ss << "    if(Vulkan_FOUND)\n";
    ss << "        get_filename_component(VULKAN_SDK_PATH ${Vulkan_INCLUDE_DIRS} DIRECTORY)\n";
    ss << "    endif()\n";
    ss << "endif()\n";
    ss << "\n";
    // Parse package.json for dependencies
    std::string packageJsonPath = addonPath + "package.json";
    NativeModuleMetadata metadata;
    ParsePackageJson(packageJsonPath, metadata);

    // Get parent Packages directory and addon name for resolving sibling addon dependencies
    std::string packagesDir;
    std::string addonName;
    {
        std::string path = addonPath;
        while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
        {
            path.pop_back();
        }
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            packagesDir = path.substr(0, lastSlash + 1);
            addonName = path.substr(lastSlash + 1);
        }
    }

    ss << "# Include directories\n";
    ss << "target_include_directories(" << binaryName << " PRIVATE\n";
    ss << "    ${CMAKE_CURRENT_SOURCE_DIR}/Source\n";

    // Add include paths from manifest
    for (const std::string& path : includePaths)
    {
        ss << "    ${OCTAVE_PATH}/" << path << "\n";
    }

    // Add dependency addon Source directories
    for (const std::string& depId : metadata.mDependencies)
    {
        std::string depSourceDir = normalizePath(packagesDir + depId + "/Source");
        ss << "    " << depSourceDir << "\n";
    }

    // Add Vulkan SDK include path
    ss << ")\n";
    ss << "\n";
    ss << "# Add Vulkan SDK include path if found\n";
    ss << "if(VULKAN_SDK_PATH)\n";
    ss << "    target_include_directories(" << binaryName << " PRIVATE ${VULKAN_SDK_PATH}/Include)\n";
    ss << "endif()\n";
    ss << "\n";
    ss << "# Compile definitions\n";
    ss << "target_compile_definitions(" << binaryName << " PRIVATE\n";
    for (const std::string& define : defines)
    {
        ss << "    " << define << "\n";
    }
    // Add export macro for this plugin
    std::string exportMacroCMake = metadata.mExportDefine.empty()
        ? GenerateExportMacroName(addonName)
        : metadata.mExportDefine;
    ss << "    " << exportMacroCMake << "\n";
    ss << ")\n";
    ss << "\n";
    ss << "# Link against Octave import library and dependencies\n";
    ss << "if(WIN32)\n";
    ss << "    if(CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
    ss << "        set(OCTAVE_LIB_PATH \"${OCTAVE_PATH}/Standalone/Build/Windows/x64/DebugEditor\")\n";
    ss << "    else()\n";
    ss << "        set(OCTAVE_LIB_PATH \"${OCTAVE_PATH}/Standalone/Build/Windows/x64/ReleaseEditor\")\n";
    ss << "    endif()\n";
    ss << "    target_link_directories(" << binaryName << " PRIVATE ${OCTAVE_LIB_PATH})\n";
    ss << "    target_link_libraries(" << binaryName << " PRIVATE Octave)\n";

    // Add dependency link directories and libraries
    std::string packagesDirCMake = normalizePath(packagesDir);
    for (const std::string& depId : metadata.mDependencies)
    {
        std::string depLibName = GenerateLibraryName(depId);
        ss << "    target_link_directories(" << binaryName << " PRIVATE \"" << packagesDirCMake << depId << "/Build\")\n";
        ss << "    target_link_libraries(" << binaryName << " PRIVATE " << depLibName << ")\n";
    }
    ss << "endif()\n";

    std::string cmakePath = addonPath + "CMakeLists.txt";
    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    return stream.WriteFile(cmakePath.c_str());
}

bool NativeAddonManager::WriteVSProject(const std::string& addonPath, const std::string& addonName,
                                         const std::string& binaryName)
{
    std::string octavePath = SYS_GetOctavePath();

    // Helper lambda to normalize paths for XML (use backslashes on Windows)
    auto normalizePathVS = [](const std::string& path) -> std::string {
        std::string result;
        for (char c : path)
        {
            if (c == '/')
                result += '\\';
            else
                result += c;
        }
        return result;
    };

    std::string octavePathVS = normalizePathVS(octavePath);

    // Try to load from manifest, fall back to hardcoded paths
    std::vector<std::string> includePaths;
    std::vector<std::string> defines;

    if (!LoadAddonIncludesManifest(includePaths, defines))
    {
        // Fallback to hardcoded paths
        includePaths = {
            "Engine/Source",
            "Engine/Source/Engine",
            "Engine/Source/Plugins",
            "External/Lua",
            "External/glm",
            "External/Imgui",
            "External/ImGuizmo",
            "External/bullet3/src",
            "External/Assimp",
            "External"
        };
        defines = {
            "OCTAVE_PLUGIN_EXPORT",
            "EDITOR=1",
            "LUA_ENABLED=1",
            "GLM_FORCE_RADIANS",
#if PLATFORM_WINDOWS
            "PLATFORM_WINDOWS=1",
            "API_VULKAN=1",
            "NOMINMAX"
#elif PLATFORM_LINUX
            "PLATFORM_LINUX=1",
            "API_VULKAN=1"
#elif PLATFORM_ANDROID
            "PLATFORM_ANDROID=1",
            "API_VULKAN=1"
#elif PLATFORM_3DS
            "PLATFORM_3DS=1",
            "API_C3D=1"
#endif
        };
    }

    std::string sourceDir = addonPath + "Source";
    std::string sourceDirVS = normalizePathVS(sourceDir);

    // Gather source files for the project
    std::vector<std::string> sourceFiles = GatherSourceFiles(sourceDir + "/");

    // Generate a simple GUID (not truly unique but sufficient for local use)
    // Format: {8-4-4-4-12}
    std::string guid = "{12345678-1234-1234-1234-123456789ABC}";

    std::stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    ss << "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";
    ss << "  <ItemGroup Label=\"ProjectConfigurations\">\n";
    ss << "    <ProjectConfiguration Include=\"Debug|x64\">\n";
    ss << "      <Configuration>Debug</Configuration>\n";
    ss << "      <Platform>x64</Platform>\n";
    ss << "    </ProjectConfiguration>\n";
    ss << "    <ProjectConfiguration Include=\"Release|x64\">\n";
    ss << "      <Configuration>Release</Configuration>\n";
    ss << "      <Platform>x64</Platform>\n";
    ss << "    </ProjectConfiguration>\n";
    ss << "  </ItemGroup>\n";
    ss << "  <PropertyGroup Label=\"Globals\">\n";
    ss << "    <VCProjectVersion>16.0</VCProjectVersion>\n";
    ss << "    <ProjectGuid>" << guid << "</ProjectGuid>\n";
    ss << "    <RootNamespace>" << binaryName << "</RootNamespace>\n";
    ss << "    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n";
    ss << "  </PropertyGroup>\n";
    ss << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";
    ss << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n";
    ss << "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n";
    ss << "    <UseDebugLibraries>true</UseDebugLibraries>\n";
    ss << "    <PlatformToolset>v143</PlatformToolset>\n";
    ss << "    <CharacterSet>Unicode</CharacterSet>\n";
    ss << "  </PropertyGroup>\n";
    ss << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" Label=\"Configuration\">\n";
    ss << "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n";
    ss << "    <UseDebugLibraries>false</UseDebugLibraries>\n";
    ss << "    <PlatformToolset>v143</PlatformToolset>\n";
    ss << "    <WholeProgramOptimization>true</WholeProgramOptimization>\n";
    ss << "    <CharacterSet>Unicode</CharacterSet>\n";
    ss << "  </PropertyGroup>\n";
    ss << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n";
    ss << "  <ImportGroup Label=\"ExtensionSettings\">\n";
    ss << "  </ImportGroup>\n";
    ss << "  <ImportGroup Label=\"Shared\">\n";
    ss << "  </ImportGroup>\n";
    ss << "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n";
    ss << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    ss << "  </ImportGroup>\n";
    ss << "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n";
    ss << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    ss << "  </ImportGroup>\n";
    ss << "  <PropertyGroup Label=\"UserMacros\" />\n";
    ss << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n";
    ss << "    <OutDir>$(ProjectDir)Build\\Debug\\</OutDir>\n";
    ss << "    <IntDir>$(ProjectDir)Build\\Intermediate\\Debug\\</IntDir>\n";
    ss << "    <TargetName>" << binaryName << "</TargetName>\n";
    ss << "  </PropertyGroup>\n";
    ss << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n";
    ss << "    <OutDir>$(ProjectDir)Build\\Release\\</OutDir>\n";
    ss << "    <IntDir>$(ProjectDir)Build\\Intermediate\\Release\\</IntDir>\n";
    ss << "    <TargetName>" << binaryName << "</TargetName>\n";
    ss << "  </PropertyGroup>\n";

    // Parse package.json for dependencies
    std::string packageJsonPath = addonPath + "package.json";
    NativeModuleMetadata metadata;
    ParsePackageJson(packageJsonPath, metadata);

    // Get parent Packages directory for resolving sibling addon dependencies
    std::string packagesDir;
    {
        std::string path = addonPath;
        while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
        {
            path.pop_back();
        }
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            packagesDir = path.substr(0, lastSlash + 1);
        }
    }

    // Build include directories string from manifest paths
    std::string includesStr;
    for (const std::string& path : includePaths)
    {
        std::string fullPath = octavePathVS + normalizePathVS(path);
        includesStr += fullPath + ";";
    }
    // Add dependency addon Source directories
    for (const std::string& depId : metadata.mDependencies)
    {
        includesStr += normalizePathVS(packagesDir + depId + "/Source") + ";";
    }
    // Add Vulkan SDK include path
    includesStr += "$(VULKAN_SDK)\\Include;";
    includesStr += "$(ProjectDir)Source;%(AdditionalIncludeDirectories)";

    // Build preprocessor definitions string from manifest
    std::string definesStr;
    for (const std::string& define : defines)
    {
        definesStr += define + ";";
    }

    // Add export macro for this plugin (so it exports its symbols when building)
    std::string exportMacro = metadata.mExportDefine.empty()
        ? GenerateExportMacroName(addonName)
        : metadata.mExportDefine;
    definesStr += exportMacro + ";";

    // Get executable directory for installed editor lib paths
    std::string exePath = SYS_GetExecutablePath();
    std::string exeDirVS;
    {
        size_t lastSlash = exePath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            exeDirVS = normalizePathVS(exePath.substr(0, lastSlash + 1));
        }
    }

    // Path to Octave import library and Lua library
    // Include both installed editor paths and development build paths
    std::string octaveLibPathDebug = exeDirVS + ";" + exeDirVS + "lib\\;" + octavePathVS + "\\Standalone\\Build\\Windows\\x64\\DebugEditor\\";
    std::string octaveLibPathRelease = exeDirVS + ";" + exeDirVS + "lib\\;" + octavePathVS + "\\Standalone\\Build\\Windows\\x64\\ReleaseEditor\\";
    std::string luaLibPathDebug = octavePathVS + "\\External\\Lua\\Build\\Windows\\x64\\DebugEditor\\";
    std::string luaLibPathRelease = octavePathVS + "\\External\\Lua\\Build\\Windows\\x64\\ReleaseEditor\\";

    // Build library paths and dependencies for dependencies
    std::string depLibPaths;
    std::string depLibs;
    for (const std::string& depId : metadata.mDependencies)
    {
        // Add dependency's build output directory to library search path
        std::string depBuildPath = normalizePathVS(packagesDir + depId + "/Build");
        depLibPaths += depBuildPath + "\\Debug;";
        depLibPaths += depBuildPath + "\\Release;";

        // Add dependency's .lib file to linker dependencies
        std::string depLibName = GenerateLibraryName(depId) + ".lib;";
        depLibs += depLibName;
    }

    ss << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n";
    ss << "    <ClCompile>\n";
    ss << "      <WarningLevel>Level3</WarningLevel>\n";
    ss << "      <SDLCheck>true</SDLCheck>\n";
    ss << "      <PreprocessorDefinitions>" << definesStr << "_DEBUG;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    ss << "      <ConformanceMode>true</ConformanceMode>\n";
    ss << "      <LanguageStandard>stdcpp17</LanguageStandard>\n";
    ss << "      <AdditionalIncludeDirectories>" << includesStr << "</AdditionalIncludeDirectories>\n";
    ss << "    </ClCompile>\n";
    ss << "    <Link>\n";
    ss << "      <SubSystem>Windows</SubSystem>\n";
    ss << "      <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    ss << "      <AdditionalLibraryDirectories>" << octaveLibPathDebug << ";" << luaLibPathDebug << ";" << depLibPaths << "%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n";
    ss << "      <AdditionalDependencies>Octave.lib;Lua.lib;" << depLibs << "%(AdditionalDependencies)</AdditionalDependencies>\n";
    ss << "    </Link>\n";
    ss << "  </ItemDefinitionGroup>\n";
    ss << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n";
    ss << "    <ClCompile>\n";
    ss << "      <WarningLevel>Level3</WarningLevel>\n";
    ss << "      <FunctionLevelLinking>true</FunctionLevelLinking>\n";
    ss << "      <IntrinsicFunctions>true</IntrinsicFunctions>\n";
    ss << "      <SDLCheck>true</SDLCheck>\n";
    ss << "      <PreprocessorDefinitions>" << definesStr << "NDEBUG;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    ss << "      <ConformanceMode>true</ConformanceMode>\n";
    ss << "      <LanguageStandard>stdcpp17</LanguageStandard>\n";
    ss << "      <AdditionalIncludeDirectories>" << includesStr << "</AdditionalIncludeDirectories>\n";
    ss << "    </ClCompile>\n";
    ss << "    <Link>\n";
    ss << "      <SubSystem>Windows</SubSystem>\n";
    ss << "      <EnableCOMDATFolding>true</EnableCOMDATFolding>\n";
    ss << "      <OptimizeReferences>true</OptimizeReferences>\n";
    ss << "      <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    ss << "      <AdditionalLibraryDirectories>" << octaveLibPathRelease << ";" << luaLibPathRelease << ";" << depLibPaths << "%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n";
    ss << "      <AdditionalDependencies>Octave.lib;Lua.lib;" << depLibs << "%(AdditionalDependencies)</AdditionalDependencies>\n";
    ss << "    </Link>\n";
    ss << "  </ItemDefinitionGroup>\n";

    // Add source files
    ss << "  <ItemGroup>\n";
    for (const std::string& file : sourceFiles)
    {
        std::string ext = "";
        size_t dotPos = file.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            ext = file.substr(dotPos);
        }

        // Convert to relative path and backslashes
        std::string relPath = file;
        if (relPath.find(addonPath) == 0)
        {
            relPath = relPath.substr(addonPath.length());
        }
        std::string relPathVS;
        for (char c : relPath)
        {
            if (c == '/')
                relPathVS += '\\';
            else
                relPathVS += c;
        }

        if (ext == ".cpp" || ext == ".c")
        {
            ss << "    <ClCompile Include=\"" << relPathVS << "\" />\n";
        }
        else if (ext == ".h" || ext == ".hpp")
        {
            ss << "    <ClInclude Include=\"" << relPathVS << "\" />\n";
        }
    }
    ss << "  </ItemGroup>\n";

    ss << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n";
    ss << "  <ImportGroup Label=\"ExtensionTargets\">\n";
    ss << "  </ImportGroup>\n";
    ss << "</Project>\n";

    std::string vcxprojPath = addonPath + binaryName + ".vcxproj";
    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    return stream.WriteFile(vcxprojPath.c_str());
}

bool NativeAddonManager::GenerateIDEConfig(const std::string& addonPath)
{
    bool success = WriteVSCodeConfig(addonPath);

    // Also parse package.json to get binary name for CMakeLists
    std::string packageJsonPath = addonPath + "package.json";
    NativeModuleMetadata metadata;
    if (ParsePackageJson(packageJsonPath, metadata))
    {
        std::string binaryName = metadata.mBinaryName;
        if (binaryName.empty())
        {
            // Try to extract from addon folder name
            std::string path = addonPath;
            while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
            {
                path.pop_back();
            }
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != std::string::npos)
            {
                binaryName = path.substr(lastSlash + 1);
            }
        }

        if (!binaryName.empty())
        {
            WriteCMakeLists(addonPath, binaryName);

            // Get addon name for VS project (use folder name if not in metadata)
            std::string addonName = binaryName;
            std::string path = addonPath;
            while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
            {
                path.pop_back();
            }
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != std::string::npos)
            {
                addonName = path.substr(lastSlash + 1);
            }

            WriteVSProject(addonPath, addonName, binaryName);
        }
    }

    return success;
}

bool NativeAddonManager::CreateNativeAddon(const NativeAddonCreateInfo& info, std::string& outError, std::string* outPath)
{
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    if (projectDir.empty())
    {
        outError = "No project loaded";
        return false;
    }

    std::string packagesDir = projectDir + "Packages/";
    return CreateNativeAddonAtPath(info, packagesDir, outError, outPath);
}

bool NativeAddonManager::CreateNativeAddonAtPath(const NativeAddonCreateInfo& info, const std::string& targetDir,
                                                   std::string& outError, std::string* outPath)
{
    // Validate name
    if (info.mName.empty())
    {
        outError = "Addon name is required";
        return false;
    }

    // Generate ID from name if not provided
    std::string addonId = info.mId.empty() ? GenerateIdFromName(info.mName) : info.mId;
    if (addonId.empty())
    {
        outError = "Could not generate valid addon ID from name";
        return false;
    }

    // Generate binary name if not provided
    std::string binaryName = info.mBinaryName.empty() ? addonId : info.mBinaryName;
    // Remove hyphens for binary name (use underscores instead)
    std::string binaryNameClean;
    for (char c : binaryName)
    {
        if (c == '-')
            binaryNameClean += '_';
        else
            binaryNameClean += c;
    }

    // Create target directory if it doesn't exist
    if (!DoesDirExist(targetDir.c_str()))
    {
        SYS_CreateDirectory(targetDir.c_str());
    }

    // Create addon directory
    std::string normalizedTarget = targetDir;
    if (!normalizedTarget.empty() && normalizedTarget.back() != '/' && normalizedTarget.back() != '\\')
    {
        normalizedTarget += '/';
    }
    std::string addonPath = normalizedTarget + addonId + "/";
    if (DoesDirExist(addonPath.c_str()))
    {
        outError = "Addon folder already exists: " + addonPath;
        return false;
    }

    SYS_CreateDirectory(addonPath.c_str());

    // Create Source directory
    std::string sourceDir = addonPath + "Source/";
    SYS_CreateDirectory(sourceDir.c_str());

    // Create Assets directory (empty, for user to add assets)
    std::string assetsDir = addonPath + "Assets/";
    SYS_CreateDirectory(assetsDir.c_str());

    // Create Scripts directory (empty, for user to add scripts)
    std::string scriptsDir = addonPath + "Scripts/";
    SYS_CreateDirectory(scriptsDir.c_str());

    // Generate C++ identifier for class name
    std::string className;
    bool capitalizeNext = true;
    for (char c : info.mName)
    {
        if (std::isalnum(c))
        {
            if (capitalizeNext)
            {
                className += static_cast<char>(std::toupper(c));
                capitalizeNext = false;
            }
            else
            {
                className += c;
            }
        }
        else
        {
            capitalizeNext = true;
        }
    }
    if (className.empty())
    {
        className = "MyAddon";
    }

    // Write package.json
    NativeAddonCreateInfo finalInfo = info;
    finalInfo.mId = addonId;
    finalInfo.mBinaryName = binaryNameClean;
    if (!WritePackageJson(addonPath + "package.json", finalInfo))
    {
        outError = "Failed to write package.json";
        return false;
    }

    // Write template source file
    std::string sourceFile = sourceDir + className + ".cpp";
    if (!WriteTemplateSourceFile(sourceFile, info.mName, binaryNameClean))
    {
        outError = "Failed to write template source file";
        return false;
    }

    // Write IDE configurations
    WriteVSCodeConfig(addonPath);
    WriteCMakeLists(addonPath, binaryNameClean);
    WriteVSProject(addonPath, info.mName, binaryNameClean);

    // Discover the new addon
    DiscoverNativeAddons();

    LogDebug("Created native addon: %s at %s", addonId.c_str(), addonPath.c_str());

    // Return the created path
    if (outPath != nullptr)
    {
        *outPath = addonPath;
    }

    return true;
}

bool NativeAddonManager::PackageNativeAddon(const NativeAddonPackageOptions& options, std::string& outError)
{
    auto it = mStates.find(options.mAddonId);
    if (it == mStates.end())
    {
        outError = "Addon not found: " + options.mAddonId;
        return false;
    }

    const NativeAddonState& state = it->second;
    std::string addonPath = state.mSourcePath;

    // Verify addon exists
    if (!DoesDirExist(addonPath.c_str()))
    {
        outError = "Addon path not found: " + addonPath;
        return false;
    }

    // Determine output path
    std::string outputPath = options.mOutputPath;
    if (outputPath.empty())
    {
        const std::string& projectDir = GetEngineState()->mProjectDirectory;
        outputPath = projectDir + "Packaged/" + options.mAddonId + ".zip";
    }

    // Create output directory if needed
    size_t lastSlash = outputPath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        std::string outputDir = outputPath.substr(0, lastSlash + 1);
        if (!DoesDirExist(outputDir.c_str()))
        {
            SYS_CreateDirectory(outputDir.c_str());
        }
    }

    // Build the zip command
    // We'll use a simple approach - create a temp directory with selected contents, then zip
    std::string tempDir = GetEngineState()->mProjectDirectory + "Intermediate/Package_" + options.mAddonId + "/";
    std::string tempAddonDir = tempDir + options.mAddonId + "/";

    // Clean and create temp directory
    if (DoesDirExist(tempDir.c_str()))
    {
        SYS_RemoveDirectory(tempDir.c_str());
    }
    SYS_CreateDirectory(tempDir.c_str());
    SYS_CreateDirectory(tempAddonDir.c_str());

    // Copy package.json (always required)
    std::string srcPackageJson = addonPath + "package.json";
    std::string dstPackageJson = tempAddonDir + "package.json";
    SYS_CopyFile(srcPackageJson.c_str(), dstPackageJson.c_str());

    // Copy selected contents
    if (options.mIncludeSource)
    {
        std::string srcDir = addonPath + "Source/";
        std::string dstDir = tempAddonDir + "Source/";
        if (DoesDirExist(srcDir.c_str()))
        {
            SYS_CopyDirectory(srcDir.c_str(), dstDir.c_str());
        }
    }

    if (options.mIncludeAssets)
    {
        std::string srcDir = addonPath + "Assets/";
        std::string dstDir = tempAddonDir + "Assets/";
        if (DoesDirExist(srcDir.c_str()))
        {
            SYS_CopyDirectory(srcDir.c_str(), dstDir.c_str());
        }
    }

    if (options.mIncludeScripts)
    {
        std::string srcDir = addonPath + "Scripts/";
        std::string dstDir = tempAddonDir + "Scripts/";
        if (DoesDirExist(srcDir.c_str()))
        {
            SYS_CopyDirectory(srcDir.c_str(), dstDir.c_str());
        }
    }

    if (options.mIncludeThumbnail)
    {
        std::string srcFile = addonPath + "thumbnail.png";
        std::string dstFile = tempAddonDir + "thumbnail.png";
        if (SYS_DoesFileExist(srcFile.c_str(), false))
        {
            SYS_CopyFile(srcFile.c_str(), dstFile.c_str());
        }
    }

    // Create zip file
#if PLATFORM_WINDOWS
    // Use PowerShell to create zip on Windows
    std::string cmd = "powershell -Command \"Compress-Archive -Path '" + tempAddonDir + "*' -DestinationPath '" + outputPath + "' -Force\"";
#else
    // Use zip command on Linux
    std::string cmd = "cd \"" + tempDir + "\" && zip -r \"" + outputPath + "\" \"" + options.mAddonId + "\"";
#endif

    std::string cmdOutput;
    int exitCode = 0;
    bool success = SYS_ExecFull(cmd.c_str(), &cmdOutput, nullptr, &exitCode);

    // Clean up temp directory
    SYS_RemoveDirectory(tempDir.c_str());

    if (!success || exitCode != 0)
    {
        outError = "Failed to create zip file: " + cmdOutput;
        return false;
    }

    LogDebug("Packaged native addon %s to %s", options.mAddonId.c_str(), outputPath.c_str());
    return true;
}

#endif // EDITOR
