#include "LuaBindings/ScrollContainer_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"

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

int ScrollContainer_Lua::SetChildInputPriority(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    bool priority = CHECK_BOOLEAN(L, 2);

    scrollContainer->SetChildInputPriority(priority);

    return 0;
}

int ScrollContainer_Lua::GetChildInputPriority(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->GetChildInputPriority();

    lua_pushboolean(L, ret);
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

int ScrollContainer_Lua::SetScrollbarTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetScrollbarTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetScrollbarTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetScrollbarTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetTrackTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetTrackTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetTrackTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetTrackTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetShowScrollButtons(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    bool show = CHECK_BOOLEAN(L, 2);

    scrollContainer->SetShowScrollButtons(show);

    return 0;
}

int ScrollContainer_Lua::GetShowScrollButtons(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    bool ret = scrollContainer->GetShowScrollButtons();

    lua_pushboolean(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetButtonSize(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    float size = CHECK_NUMBER(L, 2);

    scrollContainer->SetButtonSize(size);

    return 0;
}

int ScrollContainer_Lua::GetButtonSize(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    float ret = scrollContainer->GetButtonSize();

    lua_pushnumber(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetUpButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetUpButtonTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetUpButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetUpButtonTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetDownButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetDownButtonTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetDownButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetDownButtonTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetLeftButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetLeftButtonTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetLeftButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetLeftButtonTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetRightButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    scrollContainer->SetRightButtonTexture(texture);

    return 0;
}

int ScrollContainer_Lua::GetRightButtonTexture(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Texture* ret = scrollContainer->GetRightButtonTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::SetButtonColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    scrollContainer->SetButtonColor(color);

    return 0;
}

int ScrollContainer_Lua::GetButtonColor(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    glm::vec4 ret = scrollContainer->GetButtonColor();

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

int ScrollContainer_Lua::GetHTrack(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Quad* ret = scrollContainer->GetHTrack();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetVTrack(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Quad* ret = scrollContainer->GetVTrack();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetUpButton(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Button* ret = scrollContainer->GetUpButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetDownButton(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Button* ret = scrollContainer->GetDownButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetLeftButton(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Button* ret = scrollContainer->GetLeftButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int ScrollContainer_Lua::GetRightButton(lua_State* L)
{
    ScrollContainer* scrollContainer = CHECK_SCROLL_CONTAINER(L, 1);

    Button* ret = scrollContainer->GetRightButton();

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

    // Scroll Position
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffsetX);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollOffsetY);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToTop);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToBottom);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToLeft);
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToRight);

    // Size Mode
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollSizeMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollSizeMode);

    // Scrollbar Mode
    REGISTER_TABLE_FUNC(L, mtIndex, SetHorizontalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetHorizontalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetVerticalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVerticalScrollbarMode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarWidth);

    // Scroll Behavior
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollSpeed);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollSpeed);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMomentumEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, IsMomentumEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMomentumFriction);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMomentumFriction);

    // Query
    REGISTER_TABLE_FUNC(L, mtIndex, CanScrollHorizontally);
    REGISTER_TABLE_FUNC(L, mtIndex, CanScrollVertically);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxScrollOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, IsDragging);
    REGISTER_TABLE_FUNC(L, mtIndex, IsScrolling);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentWidget);

    // Child Input Priority
    REGISTER_TABLE_FUNC(L, mtIndex, SetChildInputPriority);
    REGISTER_TABLE_FUNC(L, mtIndex, GetChildInputPriority);

    // Colors
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarTrackColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarTrackColor);

    // Textures
    REGISTER_TABLE_FUNC(L, mtIndex, SetScrollbarTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollbarTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTrackTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTrackTexture);

    // Scroll Buttons
    REGISTER_TABLE_FUNC(L, mtIndex, SetShowScrollButtons);
    REGISTER_TABLE_FUNC(L, mtIndex, GetShowScrollButtons);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetUpButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetUpButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetDownButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetDownButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetLeftButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetLeftButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRightButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRightButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonColor);

    // Access internal widgets
    REGISTER_TABLE_FUNC(L, mtIndex, GetHScrollbar);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVScrollbar);
    REGISTER_TABLE_FUNC(L, mtIndex, GetHTrack);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVTrack);
    REGISTER_TABLE_FUNC(L, mtIndex, GetUpButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetDownButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetLeftButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRightButton);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
