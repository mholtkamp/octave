#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/SpinBox.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SPINBOX_LUA_NAME "SpinBox"
#define SPINBOX_LUA_FLAG "cfSpinBox"
#define CHECK_SPINBOX(L, arg) (SpinBox*)CheckNodeLuaType(L, arg, SPINBOX_LUA_NAME, SPINBOX_LUA_FLAG);

struct SpinBox_Lua
{
    // Value
    static int SetValue(lua_State* L);
    static int GetValue(lua_State* L);
    static int SetMinValue(lua_State* L);
    static int GetMinValue(lua_State* L);
    static int SetMaxValue(lua_State* L);
    static int GetMaxValue(lua_State* L);
    static int SetStep(lua_State* L);
    static int GetStep(lua_State* L);

    // Display
    static int SetPrefix(lua_State* L);
    static int GetPrefix(lua_State* L);
    static int SetSuffix(lua_State* L);
    static int GetSuffix(lua_State* L);

    // Visual
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);
    static int SetButtonColor(lua_State* L);
    static int GetButtonColor(lua_State* L);

    // Children
    static int GetBackground(lua_State* L);
    static int GetTextWidget(lua_State* L);
    static int GetIncrementButton(lua_State* L);
    static int GetDecrementButton(lua_State* L);

    static void Bind();
};

#endif
