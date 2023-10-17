#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/TextMeshComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TEXT_MESH_COMPONENT_LUA_NAME "TextMeshComponent"
#define TEXT_MESH_COMPONENT_LUA_FLAG "cfTextMeshComponent"
#define CHECK_TEXT_MESH_COMPONENT(L, arg) static_cast<TextMeshComponent*>(CheckComponentLuaType(L, arg, TEXT_MESH_COMPONENT_LUA_NAME, TEXT_MESH_COMPONENT_LUA_FLAG));

struct TextMeshComponent_Lua
{
    static int SetText(lua_State* L);
    static int GetText(lua_State* L);
    static int SetFont(lua_State* L);
    static int GetFont(lua_State* L);
    static int SetColor(lua_State* L);
    static int GetColor(lua_State* L);
    static int SetBlendMode(lua_State* L);
    static int GetBlendMode(lua_State* L);
    static int SetHorizontalJustification(lua_State* L);
    static int GetHorizontalJustification(lua_State* L);
    static int SetVerticalJustification(lua_State* L);
    static int GetVerticalJustification(lua_State* L);

    static void Bind();
};

#endif
