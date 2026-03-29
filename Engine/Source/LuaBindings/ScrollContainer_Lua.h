#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/ScrollContainer.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SCROLL_CONTAINER_LUA_NAME "ScrollContainer"
#define SCROLL_CONTAINER_LUA_FLAG "cfScrollContainer"
#define CHECK_SCROLL_CONTAINER(L, arg) (ScrollContainer*)CheckNodeLuaType(L, arg, SCROLL_CONTAINER_LUA_NAME, SCROLL_CONTAINER_LUA_FLAG);

struct ScrollContainer_Lua
{
    // Scroll Position
    static int SetScrollOffset(lua_State* L);
    static int GetScrollOffset(lua_State* L);
    static int SetScrollOffsetX(lua_State* L);
    static int SetScrollOffsetY(lua_State* L);
    static int ScrollToTop(lua_State* L);
    static int ScrollToBottom(lua_State* L);
    static int ScrollToLeft(lua_State* L);
    static int ScrollToRight(lua_State* L);

    // Size Mode
    static int SetScrollSizeMode(lua_State* L);
    static int GetScrollSizeMode(lua_State* L);

    // Scrollbar Mode
    static int SetHorizontalScrollbarMode(lua_State* L);
    static int GetHorizontalScrollbarMode(lua_State* L);
    static int SetVerticalScrollbarMode(lua_State* L);
    static int GetVerticalScrollbarMode(lua_State* L);
    static int SetScrollbarWidth(lua_State* L);
    static int GetScrollbarWidth(lua_State* L);

    // Scroll Behavior
    static int SetScrollSpeed(lua_State* L);
    static int GetScrollSpeed(lua_State* L);
    static int SetMomentumEnabled(lua_State* L);
    static int IsMomentumEnabled(lua_State* L);
    static int SetMomentumFriction(lua_State* L);
    static int GetMomentumFriction(lua_State* L);

    // Query
    static int CanScrollHorizontally(lua_State* L);
    static int CanScrollVertically(lua_State* L);
    static int GetContentSize(lua_State* L);
    static int GetMaxScrollOffset(lua_State* L);
    static int IsDragging(lua_State* L);
    static int IsScrolling(lua_State* L);
    static int GetContentWidget(lua_State* L);

    // Colors
    static int SetScrollbarColor(lua_State* L);
    static int GetScrollbarColor(lua_State* L);
    static int SetScrollbarHoveredColor(lua_State* L);
    static int GetScrollbarHoveredColor(lua_State* L);
    static int SetScrollbarTrackColor(lua_State* L);
    static int GetScrollbarTrackColor(lua_State* L);

    // Textures
    static int SetScrollbarTexture(lua_State* L);
    static int GetScrollbarTexture(lua_State* L);
    static int SetTrackTexture(lua_State* L);
    static int GetTrackTexture(lua_State* L);

    // Scroll Buttons
    static int SetShowScrollButtons(lua_State* L);
    static int GetShowScrollButtons(lua_State* L);
    static int SetButtonSize(lua_State* L);
    static int GetButtonSize(lua_State* L);
    static int SetUpButtonTexture(lua_State* L);
    static int GetUpButtonTexture(lua_State* L);
    static int SetDownButtonTexture(lua_State* L);
    static int GetDownButtonTexture(lua_State* L);
    static int SetLeftButtonTexture(lua_State* L);
    static int GetLeftButtonTexture(lua_State* L);
    static int SetRightButtonTexture(lua_State* L);
    static int GetRightButtonTexture(lua_State* L);
    static int SetButtonColor(lua_State* L);
    static int GetButtonColor(lua_State* L);

    // Access internal widgets
    static int GetHScrollbar(lua_State* L);
    static int GetVScrollbar(lua_State* L);
    static int GetHTrack(lua_State* L);
    static int GetVTrack(lua_State* L);
    static int GetUpButton(lua_State* L);
    static int GetDownButton(lua_State* L);
    static int GetLeftButton(lua_State* L);
    static int GetRightButton(lua_State* L);

    static void Bind();
};

#endif
