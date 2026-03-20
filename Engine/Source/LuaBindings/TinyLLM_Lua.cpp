/**
 * @file TinyLLM_Lua.cpp
 * @brief Lua bindings for TinyLLM.
 */

#include "LuaBindings/LuaUtils.h"
#include "Engine/TinyLLMManager.h"
#include "Engine/Assets/TinyLLMAsset.h"
#include "AssetManager.h"
#include "Log.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

// TinyLLM.LoadAsset(assetName) -> bool
static int TinyLLM_LoadAsset(lua_State* L)
{
    const char* assetName = luaL_checkstring(L, 1);

    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (!mgr) {
        lua_pushboolean(L, 0);
        return 1;
    }

    // Unload previous
    mgr->UnloadModel();

    // Load asset
    Asset* asset = LoadAsset(assetName);
    if (!asset) {
        LogError("TinyLLM: Failed to load asset '%s'", assetName);
        lua_pushboolean(L, 0);
        return 1;
    }

    TinyLLMAsset* llmAsset = asset->As<TinyLLMAsset>();
    if (!llmAsset) {
        LogError("TinyLLM: Asset '%s' is not a TinyLLMAsset", assetName);
        lua_pushboolean(L, 0);
        return 1;
    }

    const TinyLLMConfig& cfg = llmAsset->GetConfig();
    LogDebug("TinyLLM: Loaded asset - dim=%d, layers=%d, vocab=%d",
        cfg.dim, cfg.n_layers, cfg.vocab_size);

    bool success = mgr->LoadModel(llmAsset);
    if (!success) {
        LogError("TinyLLM: Failed to initialize inference state");
    }

    lua_pushboolean(L, success ? 1 : 0);
    return 1;
}

// TinyLLM.UnloadModel()
static int TinyLLM_UnloadModel(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (mgr) {
        mgr->UnloadModel();
        LogDebug("TinyLLM: Model unloaded");
    }
    return 0;
}

// TinyLLM.IsLoaded() -> bool
static int TinyLLM_IsLoaded(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    lua_pushboolean(L, mgr && mgr->IsModelLoaded() ? 1 : 0);
    return 1;
}

// Helper to read options table
static float GetOptionFloat(lua_State* L, int idx, const char* key, float def)
{
    lua_getfield(L, idx, key);
    float result = def;
    if (!lua_isnil(L, -1)) {
        result = (float)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    return result;
}

static int GetOptionInt(lua_State* L, int idx, const char* key, int def)
{
    lua_getfield(L, idx, key);
    int result = def;
    if (!lua_isnil(L, -1)) {
        result = (int)lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
    return result;
}

// TinyLLM.Generate(prompt, [options]) -> string
static int TinyLLM_Generate(lua_State* L)
{
    const char* prompt = luaL_checkstring(L, 1);

    int maxTokens = 64;
    float temperature = 1.0f;
    float topP = 0.9f;

    if (lua_istable(L, 2)) {
        maxTokens = GetOptionInt(L, 2, "maxTokens", maxTokens);
        temperature = GetOptionFloat(L, 2, "temperature", temperature);
        topP = GetOptionFloat(L, 2, "topP", topP);
    }

    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (!mgr || !mgr->IsModelLoaded()) {
        lua_pushstring(L, "");
        return 1;
    }

    std::string result = mgr->Generate(prompt, maxTokens, temperature, topP);

    LogDebug("TinyLLM: Generated %d chars at %.2f tok/s",
        (int)result.size(), mgr->GetLastTokPerSec());

    lua_pushstring(L, result.c_str());
    return 1;
}

// TinyLLM.BeginGenerate(prompt, [options]) -> bool
static int TinyLLM_BeginGenerate(lua_State* L)
{
    const char* prompt = luaL_checkstring(L, 1);

    int maxTokens = 64;
    float temperature = 1.0f;
    float topP = 0.9f;

    if (lua_istable(L, 2)) {
        maxTokens = GetOptionInt(L, 2, "maxTokens", maxTokens);
        temperature = GetOptionFloat(L, 2, "temperature", temperature);
        topP = GetOptionFloat(L, 2, "topP", topP);
    }

    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (!mgr || !mgr->IsModelLoaded()) {
        lua_pushboolean(L, 0);
        return 1;
    }

    bool success = mgr->BeginGenerate(prompt, maxTokens, temperature, topP);
    lua_pushboolean(L, success ? 1 : 0);
    return 1;
}

// TinyLLM.IsGenerating() -> bool
static int TinyLLM_IsGenerating(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    lua_pushboolean(L, mgr && mgr->IsGenerating() ? 1 : 0);
    return 1;
}

// TinyLLM.Continue() -> string or nil
static int TinyLLM_Continue(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (!mgr || !mgr->IsGenerating()) {
        lua_pushnil(L);
        return 1;
    }

    std::string token = mgr->ContinueGenerate();
    if (token.empty() && !mgr->IsGenerating()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, token.c_str());
    }
    return 1;
}

// TinyLLM.Abort()
static int TinyLLM_Abort(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (mgr) {
        mgr->Abort();
    }
    return 0;
}

// TinyLLM.GetTokPerSec() -> number
static int TinyLLM_GetTokPerSec(lua_State* L)
{
    float ret = 0.0f;

    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (mgr)
    {
        ret = mgr->GetLastTokPerSec();
    }

    lua_pushnumber(L, ret);
    return 1;
}

// TinyLLM.GetPosition() -> int
static int TinyLLM_GetPosition(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    lua_pushinteger(L, mgr ? mgr->GetPosition() : 0);
    return 1;
}

// TinyLLM.GetMaxSeqLen() -> int
static int TinyLLM_GetMaxSeqLen(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    lua_pushinteger(L, mgr ? mgr->GetMaxSeqLen() : 0);
    return 1;
}

// TinyLLM.Reset()
static int TinyLLM_Reset(lua_State* L)
{
    TinyLLMManager* mgr = TinyLLMManager::Get();
    if (mgr) {
        mgr->Reset();
    }
    return 0;
}

void TinyLLM_Lua_Register(lua_State* L)
{
    // Create global TinyLLM table
    lua_newtable(L);

    lua_pushcfunction(L, TinyLLM_LoadAsset);
    lua_setfield(L, -2, "LoadAsset");

    lua_pushcfunction(L, TinyLLM_UnloadModel);
    lua_setfield(L, -2, "UnloadModel");

    lua_pushcfunction(L, TinyLLM_IsLoaded);
    lua_setfield(L, -2, "IsLoaded");

    lua_pushcfunction(L, TinyLLM_Generate);
    lua_setfield(L, -2, "Generate");

    lua_pushcfunction(L, TinyLLM_BeginGenerate);
    lua_setfield(L, -2, "BeginGenerate");

    lua_pushcfunction(L, TinyLLM_IsGenerating);
    lua_setfield(L, -2, "IsGenerating");

    lua_pushcfunction(L, TinyLLM_Continue);
    lua_setfield(L, -2, "Continue");

    lua_pushcfunction(L, TinyLLM_Abort);
    lua_setfield(L, -2, "Abort");

    lua_pushcfunction(L, TinyLLM_GetTokPerSec);
    lua_setfield(L, -2, "GetTokPerSec");

    lua_pushcfunction(L, TinyLLM_GetPosition);
    lua_setfield(L, -2, "GetPosition");

    lua_pushcfunction(L, TinyLLM_GetMaxSeqLen);
    lua_setfield(L, -2, "GetMaxSeqLen");

    lua_pushcfunction(L, TinyLLM_Reset);
    lua_setfield(L, -2, "Reset");

    lua_setglobal(L, "TinyLLM");
}
