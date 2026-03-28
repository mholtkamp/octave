#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Slider.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SLIDER_LUA_NAME "Slider"
#define SLIDER_LUA_FLAG "cfSlider"
#define CHECK_SLIDER(L, arg) (Slider*)CheckNodeLuaType(L, arg, SLIDER_LUA_NAME, SLIDER_LUA_FLAG);

struct Slider_Lua
{
    static int SetValue(lua_State* L);
    static int GetValue(lua_State* L);
    static int SetMinValue(lua_State* L);
    static int GetMinValue(lua_State* L);
    static int SetMaxValue(lua_State* L);
    static int GetMaxValue(lua_State* L);
    static int SetStep(lua_State* L);
    static int GetStep(lua_State* L);
    static int SetRange(lua_State* L);
    static int IsDragging(lua_State* L);
    static int SetGrabberWidth(lua_State* L);
    static int GetGrabberWidth(lua_State* L);
    static int SetTrackHeight(lua_State* L);
    static int GetTrackHeight(lua_State* L);
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetGrabberColor(lua_State* L);
    static int GetGrabberColor(lua_State* L);
    static int SetGrabberHoveredColor(lua_State* L);
    static int GetGrabberHoveredColor(lua_State* L);
    static int SetGrabberPressedColor(lua_State* L);
    static int GetGrabberPressedColor(lua_State* L);
    static int GetBackground(lua_State* L);
    static int GetGrabber(lua_State* L);

    static void Bind();
};

#endif
