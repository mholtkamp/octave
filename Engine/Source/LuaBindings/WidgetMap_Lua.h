#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/WidgetMap.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define WIDGET_MAP_LUA_NAME "WidgetMap"
#define WIDGET_MAP_LUA_FLAG "cfWidgetMap"
#define CHECK_WIDGET_MAP(L, arg) CheckAssetLuaType<WidgetMap>(L, arg, WIDGET_MAP_LUA_NAME, WIDGET_MAP_LUA_FLAG)

struct WidgetMap_Lua
{
    static int Instantiate(lua_State* L);
    static void Bind();
};

#endif

