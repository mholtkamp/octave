#pragma once

#include "Engine.h"
#include "InputDevices.h"
#include "LuaBindings/LuaUtils.h"

#define INPUT_LUA_NAME "Input"

#define CHECK_KEY(L, arg) (KeyCode) luaL_checkinteger(L, arg);
#define CHECK_MOUSE(L, arg) (MouseCode) luaL_checkinteger(L, arg);
#define CHECK_GAMEPAD(L, arg) (GamepadButtonCode) luaL_checkinteger(L, arg);
#define CHECK_GAMEPAD_AXIS(L, arg) (GamepadAxisCode) luaL_checkinteger(L, arg);

struct Input_Lua
{
    static int IsKeyDown(lua_State* L);
    static int IsKeyJustDownRepeat(lua_State* L);
    static int IsKeyJustDown(lua_State* L);
    static int IsKeyJustUp(lua_State* L);

    static int IsControlDown(lua_State* L);
    static int IsShiftDown(lua_State* L);

    static int IsMouseButtonDown(lua_State* L);
    static int IsMouseButtonJustDown(lua_State* L);
    static int IsMouseButtonJustUp(lua_State* L);
    static int GetMousePosition(lua_State* L);
    static int GetScrollWheelDelta(lua_State* L);
    static int GetMouseDelta(lua_State* L);

    static int IsTouchDown(lua_State* L);
    static int IsPointerJustUp(lua_State* L);
    static int IsPointerJustDown(lua_State* L);
    static int IsPointerDown(lua_State* L);

    static int GetTouchPosition(lua_State* L);
    static int GetTouchPositionNormalized(lua_State* L);
    static int GetPointerPosition(lua_State* L);
    static int GetPointerPositionNormalized(lua_State* L);

    static int IsGamepadButtonDown(lua_State* L);
    static int IsGamepadButtonJustDown(lua_State* L);
    static int IsGamepadButtonJustUp(lua_State* L);
    static int GetGamepadAxisValue(lua_State* L);
    static int GetGamepadType(lua_State* L);
    static int IsGamepadConnected(lua_State* L);

    static int ShowCursor(lua_State* L);
    static int SetCursorPosition(lua_State* L);

    static int GetKeysJustDown(lua_State* L);
    static int IsAnyKeyJustDown(lua_State* L);
    static int ConvertKeyCodeToChar(lua_State* L);

    static int ShowSoftKeyboard(lua_State* L);
    static int IsSoftKeyboardShown(lua_State* L);

    static void Bind();
    static void BindKeyTable();
    static void BindMouseTable();
    static void BindGamepadTable();
};
