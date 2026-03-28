#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/ProgressBar.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PROGRESSBAR_LUA_NAME "ProgressBar"
#define PROGRESSBAR_LUA_FLAG "cfProgressBar"
#define CHECK_PROGRESSBAR(L, arg) (ProgressBar*)CheckNodeLuaType(L, arg, PROGRESSBAR_LUA_NAME, PROGRESSBAR_LUA_FLAG);

struct ProgressBar_Lua
{
    // Value
    static int SetValue(lua_State* L);
    static int GetValue(lua_State* L);
    static int SetMinValue(lua_State* L);
    static int GetMinValue(lua_State* L);
    static int SetMaxValue(lua_State* L);
    static int GetMaxValue(lua_State* L);
    static int GetRatio(lua_State* L);

    // Display
    static int SetShowPercentage(lua_State* L);
    static int IsShowingPercentage(lua_State* L);

    // Visual
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetFillColor(lua_State* L);
    static int GetFillColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);

    // Children
    static int GetBackgroundQuad(lua_State* L);
    static int GetFillQuad(lua_State* L);
    static int GetTextWidget(lua_State* L);

    static void Bind();
};

#endif
