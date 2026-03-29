#include "LuaBindings/ScrollContainer_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int ScrollContainer_Lua::SetScrollOffset(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float x = CHECK_NUMBER(L, 2);
    float y = CHECK_NUMBER(L, 3);

    scrollContainer->SetScrollOffset(glm::vec2(x, y));

    return 0;
}

int ScrollContainer_Lua::GetScrollOffset(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec2 offset = scrollContainer->GetScrollOffset();

    lua_pushnumber(L, offset.x);
    lua_pushnumber(L, offset.y);
    return 2;
}

int ScrollContainer_Lua::SetScrollOffsetX(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float x = CHECK_NUMBER(L, 2);

    scrollContainer->SetScrollOffsetX(x);

    return 0;
}

int ScrollContainer_Lua::SetScrollOffsetY(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float y = CHECK_NUMBER(L, 2);

    scrollContainer->SetScrollOffsetY(y);

    return 0;
}

int ScrollContainer_Lua::ScrollToTop(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    scrollContainer->ScrollToTop();

    return 0;
}

int ScrollContainer_Lua::ScrollToBottom(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    scrollContainer->ScrollToBottom();

    return 0;
}

int ScrollContainer_Lua::ScrollToLeft(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    scrollContainer->ScrollToLeft();

    return 0;
}

int ScrollContainer_Lua::ScrollToRight(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    scrollContainer->ScrollToRight();

    return 0;
}

int ScrollContainer_Lua::SetScrollSizeMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    ScrollSizeMode mode = (ScrollSizeMode)CHECK_INTEGER(L, 2);

    scrollContainer->SetScrollSizeMode(mode);

    return 0;
}

int ScrollContainer_Lua::GetScrollSizeMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    int ret = (int)scrollContainer->GetScrollSizeMode();

    lua_pushinteger(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetHorizontalScrollbarMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    ScrollbarMode mode = (ScrollbarMode)CHECK_INTEGER(L, 2);

    scrollContainer->SetHorizontalScrollbarMode(mode);

    return 0;
}

int ScrollContainer_Lua::GetHorizontalScrollbarMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    int ret = (int)scrollContainer->GetHorizontalScrollbarMode();

    lua_pushinteger(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetVerticalScrollbarMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    ScrollbarMode mode = (ScrollbarMode)CHECK_INTEGER(L, 2);

    scrollContainer->SetVerticalScrollbarMode(mode);

    return 0;
}

int ScrollContainer_Lua::GetVerticalScrollbarMode(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    int ret = (int)scrollContainer->GetVerticalScrollbarMode();

    lua_pushinteger(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetScrollbarWidth(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float width = CHECK_NUMBER(L, 2);

    scrollContainer->SetScrollbarWidth(width);

    return 0;
}

int ScrollContainer_Lua::GetScrollbarWidth(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    float ret = scrollContainer->GetScrollbarWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetScrollSpeed(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float speed = CHECK_NUMBER(L, 2);

    scrollContainer->SetScrollSpeed(speed);

    return 0;
}

int ScrollContainer_Lua::GetScrollSpeed(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    float ret = scrollContainer->GetScrollSpeed();

    lua_pushnumber(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetMomentumEnabled(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    bool enabled = CHECK_BOOLEAN(L, 2);

    scrollContainer->SetMomentumEnabled(enabled);

    return 0;
}

int ScrollContainer_Lua::IsMomentumEnabled(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->IsMomentumEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetMomentumFriction(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float friction = CHECK_NUMBER(L, 2);

    scrollContainer->SetMomentumFriction(friction);

    return 0;
}

int ScrollContainer_Lua::GetMomentumFriction(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    float ret = scrollContainer->GetMomentumFriction();

    lua_pushnumber(L, ret);
    return 1;
}

int ScrollContainer_Lua::CanScrollHorizontally(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->CanScrollHorizontally();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::CanScrollVertically(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->CanScrollVertically();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetContentSize(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec2 size = scrollContainer->GetContentSize();

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ScrollContainer_Lua::GetMaxScrollOffset(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec2 maxOffset = scrollContainer->GetMaxScrollOffset();

    lua_pushnumber(L, maxOffset.x);
    lua_pushnumber(L, maxOffset.y);
    return 2;
}

int ScrollContainer_Lua::IsDragging(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->IsDragging();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::IsScrolling(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->IsScrolling();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetContentWidget(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Widget* ret = scrollContainer->GetContentWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetScrollbarColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    scrollContainer->SetScrollbarColor(color);

    return 0;
}

int ScrollContainer_Lua::GetScrollbarColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec4 ret = scrollContainer->GetScrollbarColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetScrollbarHoveredColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    scrollContainer->SetScrollbarHoveredColor(color);

    return 0;
}

int ScrollContainer_Lua::GetScrollbarHoveredColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec4 ret = scrollContainer->GetScrollbarHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetScrollbarTrackColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    scrollContainer->SetScrollbarTrackColor(color);

    return 0;
}

int ScrollContainer_Lua::GetScrollbarTrackColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec4 ret = scrollContainer->GetScrollbarTrackColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetHScrollbar(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Quad* ret = scrollContainer->GetHScrollbar();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetVScrollbar(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Quad* ret = scrollContainer->GetVScrollbar();

    Node_Lua::Create(L, ret);
    return 1;
}

void ScrollContainer_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SCROLL_CONTAINER_LUA_NAME,
        SCROLL_CONTAINER_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffsetX);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffsetY);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToTop);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToBottom);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToLeft);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToRight);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollSizeMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollSizeMode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHorizontalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetHorizontalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetVerticalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVerticalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollSpeed);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollSpeed);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMomentumEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, IsMomentumEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMomentumFriction);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMomentumFriction);

    REGISTER_TABLE_FUNC(L, mtIndex, CanScrollHorizontally);
    REGISTER_TABLE_FUNC(L, mtIndex, CanScrollVertically);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, IsDragging);
    REGISTER_TABLE_FUNC(L, mtIndex, IsScrolling);

    REGISTER_TABLE_FUNC(L, mtIndex, GetContentWidget);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarTrackColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarTrackColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHScrollbar);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVScrollbar);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
