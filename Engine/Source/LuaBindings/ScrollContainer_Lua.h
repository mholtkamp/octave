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
    static int SetScrollOffset(lua_State* L);
    static int GetScrollOffset(lua_State* L);
    static int SetScrollOffsetX(lua_State* L);
    static int SetScrollOffsetY(lua_State* L);
    static int ScrollToTop(lua_State* L);
    static int ScrollToBottom(lua_State* L);
    static int ScrollToLeft(lua_State* L);
    static int ScrollToRight(lua_State* L);

    static int SetScrollSizeMode(lua_State* L);
    static int GetScrollSizeMode(lua_State* L);

    static int SetHorizontalScrollbarMode(lua_State* L);
    static int GetHorizontalScrollbarMode(lua_State* L);
    static int SetVerticalScrollbarMode(lua_State* L);
    static int GetVerticalScrollbarMode(lua_State* L);
    static int SetScrollbarWidth(lua_State* L);
    static int GetScrollbarWidth(lua_State* L);

    static int SetScrollSpeed(lua_State* L);
    static int GetScrollSpeed(lua_State* L);
    static int SetMomentumEnabled(lua_State* L);
    static int IsMomentumEnabled(lua_State* L);
    static int SetMomentumFriction(lua_State* L);
    static int GetMomentumFriction(lua_State* L);

    static int CanScrollHorizontally(lua_State* L);
    static int CanScrollVertically(lua_State* L);
    static int GetContentSize(lua_State* L);
    static int GetMaxScrollOffset(lua_State* L);
    static int IsDragging(lua_State* L);
    static int IsScrolling(lua_State* L);

    static int GetContentWidget(lua_State* L);

    static int SetScrollbarColor(lua_State* L);
    static int GetScrollbarColor(lua_State* L);
    static int SetScrollbarHoveredColor(lua_State* L);
    static int GetScrollbarHoveredColor(lua_State* L);
    static int SetScrollbarTrackColor(lua_State* L);
    static int GetScrollbarTrackColor(lua_State* L);

    static int GetHScrollbar(lua_State* L);
    static int GetVScrollbar(lua_State* L);

    static void Bind();
};

#endif
