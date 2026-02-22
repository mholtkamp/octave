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

int NodeGraphPlayer_Lua::SetInputByte(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    uint8_t value = (uint8_t)CHECK_NUMBER(L, 3);
    lua_pushboolean(L, node->SetInputByte(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputAsset(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Asset* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_ASSET(L, 3);
    }
    lua_pushboolean(L, node->SetInputAsset(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputVector2D(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    glm::vec2 value = CHECK_VECTOR(L, 3);
    lua_pushboolean(L, node->SetInputVector2D(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputShort(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    int16_t value = (int16_t)CHECK_NUMBER(L, 3);
    lua_pushboolean(L, node->SetInputShort(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputNode(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputNode(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputNode3D(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputNode3D(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputWidget(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputWidget(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputText(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputText(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputQuad(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputQuad(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputAudio3D(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputAudio3D(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputScene(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Asset* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_ASSET(L, 3);
    }
    lua_pushboolean(L, node->SetInputScene(name, value));
    return 1;
}

int NodeGraphPlayer_Lua::SetInputSpline3D(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Node* value = nullptr;
    if (!lua_isnil(L, 3))
    {
        value = CHECK_NODE(L, 3);
    }
    lua_pushboolean(L, node->SetInputSpline3D(name, value));
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

int NodeGraphPlayer_Lua::GetOutputString(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    std::string value = node->GetOutputString(pinIndex);
    lua_pushstring(L, value.c_str());
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputVector2D(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    glm::vec2 value = node->GetOutputVector2D(pinIndex);
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

int NodeGraphPlayer_Lua::GetOutputByte(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    lua_pushinteger(L, node->GetOutputByte(pinIndex));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputShort(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    lua_pushinteger(L, node->GetOutputShort(pinIndex));
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputAsset(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    Asset* value = node->GetOutputAsset(pinIndex);
    Asset_Lua::Create(L, value);
    return 1;
}

int NodeGraphPlayer_Lua::GetOutputNode(lua_State* L)
{
    NodeGraphPlayer* node = CHECK_NODE_GRAPH_PLAYER(L, 1);
    uint32_t pinIndex = (uint32_t)CHECK_NUMBER(L, 2);
    Node* value = node->GetOutputNode(pinIndex);
    Node_Lua::Create(L, value);
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
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputByte);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputAsset);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputVector2D);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputShort);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputNode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputNode3D);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputText);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputQuad);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputAudio3D);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputScene);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInputSpline3D);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputFloat);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputInt);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputBool);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputString);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputVector2D);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputVector);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputByte);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputShort);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputAsset);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOutputNode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPlayOnStart);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlayOnStart);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
