#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/NodeGraphPlayer.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define NODE_GRAPH_PLAYER_LUA_NAME "NodeGraphPlayer"
#define NODE_GRAPH_PLAYER_LUA_FLAG "cfNodeGraphPlayer"
#define CHECK_NODE_GRAPH_PLAYER(L, arg) static_cast<NodeGraphPlayer*>(CheckNodeLuaType(L, arg, NODE_GRAPH_PLAYER_LUA_NAME, NODE_GRAPH_PLAYER_LUA_FLAG));

struct NodeGraphPlayer_Lua
{
    // Playback
    static int Play(lua_State* L);
    static int Pause(lua_State* L);
    static int Stop(lua_State* L);
    static int Reset(lua_State* L);
    static int IsPlaying(lua_State* L);
    static int IsPaused(lua_State* L);

    // Asset
    static int SetNodeGraphAsset(lua_State* L);
    static int GetNodeGraphAsset(lua_State* L);

    // Input setters
    static int SetInputFloat(lua_State* L);
    static int SetInputInt(lua_State* L);
    static int SetInputBool(lua_State* L);
    static int SetInputString(lua_State* L);
    static int SetInputVector(lua_State* L);
    static int SetInputColor(lua_State* L);

    // Output getters
    static int GetOutputFloat(lua_State* L);
    static int GetOutputInt(lua_State* L);
    static int GetOutputBool(lua_State* L);
    static int GetOutputVector(lua_State* L);
    static int GetOutputColor(lua_State* L);

    // Settings
    static int SetPlayOnStart(lua_State* L);
    static int GetPlayOnStart(lua_State* L);

    static void Bind();
};

#endif
