#include "RuntimePluginManager.h"
#include "System/System.h"
#include "Engine.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Engine/AudioManager.h"
#include "Engine/Clock.h"
#include "Engine/Nodes/Node.h"
#include "Engine/Nodes/3D/Node3d.h"
#include "Input/Input.h"
#include "Log.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

RuntimePluginManager* RuntimePluginManager::sInstance = nullptr;

// Pending registrations (for plugins registered before Create())
struct PendingPluginRegistration
{
    int (*getDescFunc)(OctavePluginDesc*);
    std::string pluginId;
};
static std::vector<PendingPluginRegistration>* sPendingRegistrations = nullptr;

void QueuePluginRegistration(int (*getDescFunc)(OctavePluginDesc*), const char* pluginId)
{

    // If manager already exists, register directly
    if (RuntimePluginManager::Get())
    {
        OctavePluginDesc desc = {};
        if (getDescFunc(&desc) == 0)
        {
            RuntimePluginManager::Get()->RegisterPlugin(desc, pluginId);
        }
        return;
    }

    // Otherwise queue for later
    if (sPendingRegistrations == nullptr)
    {
        sPendingRegistrations = new std::vector<PendingPluginRegistration>();
    }

    PendingPluginRegistration pending;
    pending.getDescFunc = getDescFunc;
    pending.pluginId = pluginId;
    sPendingRegistrations->push_back(pending);
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

// ===== RuntimePluginManager Implementation =====

void RuntimePluginManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new RuntimePluginManager();

        // Process any pending registrations from static initialization
        int pendingCount = sPendingRegistrations ? (int)sPendingRegistrations->size() : 0;

        if (sPendingRegistrations != nullptr)
        {
            for (const PendingPluginRegistration& pending : *sPendingRegistrations)
            {
                OctavePluginDesc desc = {};
                if (pending.getDescFunc(&desc) == 0)
                {
                    sInstance->RegisterPlugin(desc, pending.pluginId);
                }
            }

            delete sPendingRegistrations;
            sPendingRegistrations = nullptr;
        }
    }
}

void RuntimePluginManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

RuntimePluginManager* RuntimePluginManager::Get()
{
    return sInstance;
}

RuntimePluginManager::RuntimePluginManager()
{
    InitializeEngineAPI();
}

RuntimePluginManager::~RuntimePluginManager()
{
    Shutdown();
}

void RuntimePluginManager::InitializeEngineAPI()
{
    // Logging
    mEngineAPI.LogDebug = PluginLogDebug;
    mEngineAPI.LogWarning = PluginLogWarning;
    mEngineAPI.LogError = PluginLogError;

    // Lua
    mEngineAPI.GetLua = PluginGetLua;

    // Lua Wrappers
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

    // Editor UI (not available in runtime)
    mEngineAPI.editorUI = nullptr;
}

void RuntimePluginManager::Initialize()
{
    if (mInitialized)
    {
        return;
    }


    for (RuntimePluginState& plugin : mPlugins)
    {

        if (plugin.mDesc.OnLoad != nullptr)
        {
            int result = plugin.mDesc.OnLoad(&mEngineAPI);
            if (result == 0)
            {
                plugin.mLoaded = true;

                // Register script functions
                if (plugin.mDesc.RegisterScriptFuncs != nullptr)
                {
                    plugin.mDesc.RegisterScriptFuncs(GetLua());
                }
            }
            else
            {
            }
        }
        else
        {
            // No OnLoad, just mark as loaded
            plugin.mLoaded = true;
        }
    }

    mInitialized = true;
}

void RuntimePluginManager::Shutdown()
{
    if (!mInitialized)
    {
        return;
    }

    LogDebug("RuntimePluginManager: Shutting down %zu plugins", mPlugins.size());

    for (RuntimePluginState& plugin : mPlugins)
    {
        if (plugin.mLoaded && plugin.mDesc.OnUnload != nullptr)
        {
            plugin.mDesc.OnUnload();
            LogDebug("RuntimePluginManager: Unloaded plugin '%s'", plugin.mPluginId.c_str());
        }
        plugin.mLoaded = false;
    }

    mPlugins.clear();
    mInitialized = false;
}

void RuntimePluginManager::RegisterPlugin(const OctavePluginDesc& desc, const std::string& pluginId)
{
    // Check for duplicate
    for (const RuntimePluginState& existing : mPlugins)
    {
        if (existing.mPluginId == pluginId)
        {
            LogWarning("RuntimePluginManager: Plugin '%s' already registered", pluginId.c_str());
            return;
        }
    }

    RuntimePluginState state;
    state.mPluginId = pluginId;
    state.mDesc = desc;
    state.mLoaded = false;

    mPlugins.push_back(state);

    LogDebug("RuntimePluginManager: Registered plugin '%s'", pluginId.c_str());
}

void RuntimePluginManager::TickAllPlugins(float deltaTime)
{
    for (RuntimePluginState& plugin : mPlugins)
    {
        if (plugin.mLoaded && plugin.mDesc.Tick != nullptr)
        {
            plugin.mDesc.Tick(deltaTime);
        }
    }
}

bool RuntimePluginManager::IsRegistered(const std::string& pluginId) const
{
    for (const RuntimePluginState& plugin : mPlugins)
    {
        if (plugin.mPluginId == pluginId)
        {
            return true;
        }
    }
    return false;
}
