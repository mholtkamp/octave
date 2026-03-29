#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Window.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define WINDOW_LUA_NAME "Window"
#define WINDOW_LUA_FLAG "cfWindow"
#define CHECK_WINDOW(L, arg) (Window*)CheckNodeLuaType(L, arg, WINDOW_LUA_NAME, WINDOW_LUA_FLAG);

struct Window_Lua
{
    // Window ID
    static int SetWindowId(lua_State* L);
    static int GetWindowId(lua_State* L);

    // Title
    static int SetTitle(lua_State* L);
    static int GetTitle(lua_State* L);
    static int SetTitleAlignment(lua_State* L);
    static int GetTitleAlignment(lua_State* L);
    static int SetTitleFontSize(lua_State* L);
    static int GetTitleFontSize(lua_State* L);
    static int SetTitleOffset(lua_State* L);
    static int GetTitleOffset(lua_State* L);

    // Content Padding
    static int SetContentPadding(lua_State* L);
    static int SetContentPaddingLeft(lua_State* L);
    static int SetContentPaddingTop(lua_State* L);
    static int SetContentPaddingRight(lua_State* L);
    static int SetContentPaddingBottom(lua_State* L);
    static int GetContentPaddingLeft(lua_State* L);
    static int GetContentPaddingTop(lua_State* L);
    static int GetContentPaddingRight(lua_State* L);
    static int GetContentPaddingBottom(lua_State* L);

    // Visibility
    static int Show(lua_State* L);
    static int Hide(lua_State* L);
    static int Close(lua_State* L);

    // Properties
    static int SetDraggable(lua_State* L);
    static int IsDraggable(lua_State* L);
    static int SetResizable(lua_State* L);
    static int IsResizable(lua_State* L);
    static int SetShowCloseButton(lua_State* L);
    static int GetShowCloseButton(lua_State* L);
    static int SetTitleBarHeight(lua_State* L);
    static int GetTitleBarHeight(lua_State* L);
    static int SetMinSize(lua_State* L);
    static int GetMinSize(lua_State* L);
    static int SetMaxSize(lua_State* L);
    static int GetMaxSize(lua_State* L);
    static int SetResizeHandleSize(lua_State* L);
    static int GetResizeHandleSize(lua_State* L);

    // Content
    static int SetContentWidget(lua_State* L);
    static int GetContentContainer(lua_State* L);
    static int GetContentWidget(lua_State* L);

    // Colors
    static int SetTitleBarColor(lua_State* L);
    static int GetTitleBarColor(lua_State* L);
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);

    // Textures
    static int SetBackgroundTexture(lua_State* L);
    static int GetBackgroundTexture(lua_State* L);
    static int SetTitleBarTexture(lua_State* L);
    static int GetTitleBarTexture(lua_State* L);
    static int SetCloseButtonTexture(lua_State* L);
    static int GetCloseButtonTexture(lua_State* L);

    // Close Button Colors
    static int SetCloseButtonNormalColor(lua_State* L);
    static int GetCloseButtonNormalColor(lua_State* L);
    static int SetCloseButtonHoveredColor(lua_State* L);
    static int GetCloseButtonHoveredColor(lua_State* L);
    static int SetCloseButtonPressedColor(lua_State* L);
    static int GetCloseButtonPressedColor(lua_State* L);

    // Internal widget access
    static int GetTitleBar(lua_State* L);
    static int GetTitleText(lua_State* L);
    static int GetCloseButton(lua_State* L);
    static int GetResizeHandle(lua_State* L);
    static int GetBackground(lua_State* L);

    static void Bind();
};

#endif
