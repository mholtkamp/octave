#include "LuaBindings/NodeGraphPlayer_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "Assets/NodeGraphAsset.h"

#if LUA_ENABLED

int NodeGraphPlayer_Lua::Play(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    node->Play();
    return 0;
}

int NodeGraphPlayer_Lua::Pause(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    node->Pause();
    return 0;
}

int NodeGraphPlayer_Lua::Stop(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    node->StopGraph();
    return 0;
}

int NodeGraphPlayer_Lua::Reset(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    node->Reset();
    return 0;
}

int NodeGraphPlayer_Lua::IsPlaying(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    lua_pushboolean(L, node->IsPlaying());
    return 1;
}

int NodeGraphPlayer_Lua::IsPaused(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    lua_pushboolean(L, node->IsPaused());
    return 1;
}

int NodeGraphPlayer_Lua::SetNodeGraphAsset(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    NodeGraphAsset* asset = nullptr;

    if (!lua_isnil(L, 2))
    {
        Asset* rawAsset = CHECK_ASSET(L, 2);
        asset = rawAsset ? rawAsset->As<NodeGraphAsset>() : nullptr;
    }

    node->SetNodeGraphAsset(asset);
    return 0;
}

int NodeGraphPlayer_Lua::GetNodeGraphAsset(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    NodeGraphAsset* asset = node->GetNodeGraphAsset();
    Asset_Lua::Create(L, asset);
    return 1;
}

int NodeGraphPlayer_Lua::SetInputFloat(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    float value = CHECK_NUMBER(L, 3);
    lua_pushboolean(L, node->SetInputFloat(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputInt(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    int32_t value = (int32_t)CHECK_NUMBER(L, 3);
    lua_pushboolean(L, node->SetInputInt(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputBool(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    bool value = CHECK_BOOLEAN(L, 3);
    lua_pushboolean(L, node->SetInputBool(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputString(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    const char* value = CHECK_STRING(L, 3);
    lua_pushboolean(L, node->SetInputString(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputVector(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);
    lua_pushboolean(L, node->SetInputVector(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputColor(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    glm::vec4 value = CHECK_VECTOR(L, 3);
    lua_pushboolean(L, node->SetInputColor(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputFloat(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    lua_pushnumber(L, node->GetOutputFloat(pinIndex));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputInt(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    lua_pushinteger(L, node->GetOutputInt(pinIndex));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputBool(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    lua_pushboolean(L, node->GetOutputBool(pinIndex));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputVector(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    glm::vec3 value = node->GetOutputVector(pinIndex);
    Vector_Lua::Create(L, value);
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputColor(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    glm::vec4 value = node->GetOutputColor(pinIndex);
    Vector_Lua::Create(L, value);
    return 1;
}

int NodeGraphPlayer_Lua::SetPlayOnStart(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    bool playOnStart = CHECK_BOOLEAN(L, 2);
    node->SetPlayOnStart(playOnStart);
    return 0;
}

int NodeGraphPlayer_Lua::GetPlayOnStart(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    lua_pushboolean(L, node->GetPlayOnStart());
    return 1;
}

void NodeGraphPlayer_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        NODE_GRAPH_PLAYER_LUA_NAME,
        NODE_GRAPH_PLAYER_LUA_FLAG,
        NODE_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Play);
    REGISTER_TABLE_FUNC(L, mtIndex, Pause);
    REGISTER_TABLE_FUNC(L, mtIndex, Stop);
    REGISTER_TABLE_FUNC(L, mtIndex, Reset);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPlaying);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPaused);
    REGISTER_TABLE_FUNC(L, mtIndex, SetNodeGraphAsset);
    REGISTER_TABLE_FUNC(L, mtIndex, GetNodeGraphAsset);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputFloat);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputInt);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputBool);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputString);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputVector);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputFloat);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputInt);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputBool);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputVector);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPlayOnStart);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlayOnStart);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
