#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/DialogWindow.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define DIALOGWINDOW_LUA_NAME "DialogWindow"
#define DIALOGWINDOW_LUA_FLAG "cfDialogWindow"
#define CHECK_DIALOGWINDOW(L, arg) (DialogWindow*)CheckNodeLuaType(L, arg, DIALOGWINDOW_LUA_NAME, DIALOGWINDOW_LUA_FLAG);

struct DialogWindow_Lua
{
    // Confirm Button
    static int SetConfirmText(lua_State* L);
    static int GetConfirmText(lua_State* L);
    static int SetConfirmNormalColor(lua_State* L);
    static int GetConfirmNormalColor(lua_State* L);
    static int SetConfirmHoveredColor(lua_State* L);
    static int GetConfirmHoveredColor(lua_State* L);
    static int SetConfirmPressedColor(lua_State* L);
    static int GetConfirmPressedColor(lua_State* L);
    static int SetConfirmTexture(lua_State* L);
    static int GetConfirmTexture(lua_State* L);
    static int SetShowConfirmButton(lua_State* L);
    static int GetShowConfirmButton(lua_State* L);

    // Reject Button
    static int SetRejectText(lua_State* L);
    static int GetRejectText(lua_State* L);
    static int SetRejectNormalColor(lua_State* L);
    static int GetRejectNormalColor(lua_State* L);
    static int SetRejectHoveredColor(lua_State* L);
    static int GetRejectHoveredColor(lua_State* L);
    static int SetRejectPressedColor(lua_State* L);
    static int GetRejectPressedColor(lua_State* L);
    static int SetRejectTexture(lua_State* L);
    static int GetRejectTexture(lua_State* L);
    static int SetShowRejectButton(lua_State* L);
    static int GetShowRejectButton(lua_State* L);

    // Button Bar Layout
    static int SetButtonBarHeight(lua_State* L);
    static int GetButtonBarHeight(lua_State* L);
    static int SetButtonSpacing(lua_State* L);
    static int GetButtonSpacing(lua_State* L);
    static int SetButtonBarAlignment(lua_State* L);
    static int GetButtonBarAlignment(lua_State* L);
    static int SetButtonBarColor(lua_State* L);
    static int GetButtonBarColor(lua_State* L);
    static int SetButtonWidth(lua_State* L);
    static int GetButtonWidth(lua_State* L);
    static int SetButtonBarPadding(lua_State* L);
    static int GetButtonBarPadding(lua_State* L);

    // Dialog Actions
    static int Confirm(lua_State* L);
    static int Reject(lua_State* L);

    // Internal widget access
    static int GetButtonBar(lua_State* L);
    static int GetConfirmButton(lua_State* L);
    static int GetRejectButton(lua_State* L);

    static void Bind();
};

#endif
