#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/TextField.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TEXT_FIELD_LUA_NAME "TextField"
#define TEXT_FIELD_LUA_FLAG "cfTextField"
#define CHECK_TEXT_FIELD(L, arg) (TextField*) CheckHierarchyLuaType<Widget_Lua>(L, arg, TEXT_FIELD_LUA_NAME, TEXT_FIELD_LUA_FLAG)->mWidget;

struct TextField_Lua
{
    static int CreateNew(lua_State* L);

    static int SetTextEditHandler(lua_State* L);
    static int SetTextConfirmHandler(lua_State* L);
    static int GetSelectedTextField(lua_State* L);

    static void Bind();
};

#endif