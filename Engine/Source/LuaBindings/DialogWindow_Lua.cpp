#include "LuaBindings/DialogWindow_Lua.h"
#include "LuaBindings/Window_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Canvas_Lua.h"
#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#if LUA_ENABLED

// Confirm Button

int DialogWindow_Lua::SetConfirmText(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    const char* text = CHECK_STRING(L, 2);

    dialog->SetConfirmText(text);

    return 0;
}

int DialogWindow_Lua::GetConfirmText(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    const std::string& text = dialog->GetConfirmText();

    lua_pushstring(L, text.c_str());
    return 1;
}

int DialogWindow_Lua::SetConfirmNormalColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetConfirmNormalColor(color);

    return 0;
}

int DialogWindow_Lua::GetConfirmNormalColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetConfirmNormalColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetConfirmHoveredColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetConfirmHoveredColor(color);

    return 0;
}

int DialogWindow_Lua::GetConfirmHoveredColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetConfirmHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetConfirmPressedColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetConfirmPressedColor(color);

    return 0;
}

int DialogWindow_Lua::GetConfirmPressedColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetConfirmPressedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetConfirmTexture(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    dialog->SetConfirmTexture(texture);

    return 0;
}

int DialogWindow_Lua::GetConfirmTexture(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    Texture* ret = dialog->GetConfirmTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetShowConfirmButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    bool show = CHECK_BOOLEAN(L, 2);

    dialog->SetShowConfirmButton(show);

    return 0;
}

int DialogWindow_Lua::GetShowConfirmButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    bool ret = dialog->GetShowConfirmButton();

    lua_pushboolean(L, ret);
    return 1;
}

// Reject Button

int DialogWindow_Lua::SetRejectText(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    const char* text = CHECK_STRING(L, 2);

    dialog->SetRejectText(text);

    return 0;
}

int DialogWindow_Lua::GetRejectText(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    const std::string& text = dialog->GetRejectText();

    lua_pushstring(L, text.c_str());
    return 1;
}

int DialogWindow_Lua::SetRejectNormalColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetRejectNormalColor(color);

    return 0;
}

int DialogWindow_Lua::GetRejectNormalColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetRejectNormalColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetRejectHoveredColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetRejectHoveredColor(color);

    return 0;
}

int DialogWindow_Lua::GetRejectHoveredColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetRejectHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetRejectPressedColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetRejectPressedColor(color);

    return 0;
}

int DialogWindow_Lua::GetRejectPressedColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetRejectPressedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetRejectTexture(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    dialog->SetRejectTexture(texture);

    return 0;
}

int DialogWindow_Lua::GetRejectTexture(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    Texture* ret = dialog->GetRejectTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetShowRejectButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    bool show = CHECK_BOOLEAN(L, 2);

    dialog->SetShowRejectButton(show);

    return 0;
}

int DialogWindow_Lua::GetShowRejectButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    bool ret = dialog->GetShowRejectButton();

    lua_pushboolean(L, ret);
    return 1;
}

// Button Bar Layout

int DialogWindow_Lua::SetButtonBarHeight(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    float height = CHECK_NUMBER(L, 2);

    dialog->SetButtonBarHeight(height);

    return 0;
}

int DialogWindow_Lua::GetButtonBarHeight(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    float ret = dialog->GetButtonBarHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int DialogWindow_Lua::SetButtonSpacing(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    float spacing = CHECK_NUMBER(L, 2);

    dialog->SetButtonSpacing(spacing);

    return 0;
}

int DialogWindow_Lua::GetButtonSpacing(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    float ret = dialog->GetButtonSpacing();

    lua_pushnumber(L, ret);
    return 1;
}

int DialogWindow_Lua::SetButtonBarAlignment(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    ButtonBarAlignment alignment = (ButtonBarAlignment)CHECK_INTEGER(L, 2);

    dialog->SetButtonBarAlignment(alignment);

    return 0;
}

int DialogWindow_Lua::GetButtonBarAlignment(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    int ret = (int)dialog->GetButtonBarAlignment();

    lua_pushinteger(L, ret);
    return 1;
}

int DialogWindow_Lua::SetButtonBarColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    dialog->SetButtonBarColor(color);

    return 0;
}

int DialogWindow_Lua::GetButtonBarColor(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    glm::vec4 ret = dialog->GetButtonBarColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::SetButtonWidth(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    float width = CHECK_NUMBER(L, 2);

    dialog->SetButtonWidth(width);

    return 0;
}

int DialogWindow_Lua::GetButtonWidth(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    float ret = dialog->GetButtonWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int DialogWindow_Lua::SetButtonBarPadding(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);
    float padding = CHECK_NUMBER(L, 2);

    dialog->SetButtonBarPadding(padding);

    return 0;
}

int DialogWindow_Lua::GetButtonBarPadding(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    float ret = dialog->GetButtonBarPadding();

    lua_pushnumber(L, ret);
    return 1;
}

// Dialog Actions

int DialogWindow_Lua::Confirm(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    dialog->Confirm();

    return 0;
}

int DialogWindow_Lua::Reject(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    dialog->Reject();

    return 0;
}

// Internal widget access

int DialogWindow_Lua::GetButtonBar(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    Canvas* ret = dialog->GetButtonBar();

    Node_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::GetConfirmButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    Button* ret = dialog->GetConfirmButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int DialogWindow_Lua::GetRejectButton(lua_State* L)
{
    DialogWindow* dialog = CHECK_DIALOGWINDOW(L, 1);

    Button* ret = dialog->GetRejectButton();

    Node_Lua::Create(L, ret);
    return 1;
}

void DialogWindow_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        DIALOGWINDOW_LUA_NAME,
        DIALOGWINDOW_LUA_FLAG,
        WINDOW_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Confirm Button
    REGISTER_TABLE_FUNC(L, mtIndex, SetConfirmText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmText);
    REGISTER_TABLE_FUNC(L, mtIndex, SetConfirmNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetConfirmHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetConfirmPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetConfirmTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetShowConfirmButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetShowConfirmButton);

    // Reject Button
    REGISTER_TABLE_FUNC(L, mtIndex, SetRejectText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectText);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRejectNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRejectHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRejectPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRejectTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetShowRejectButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetShowRejectButton);

    // Button Bar Layout
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonBarHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonBarHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonBarAlignment);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonBarAlignment);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonBarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonBarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonBarPadding);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonBarPadding);

    // Dialog Actions
    REGISTER_TABLE_FUNC(L, mtIndex, Confirm);
    REGISTER_TABLE_FUNC(L, mtIndex, Reject);

    // Internal widget access
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonBar);
    REGISTER_TABLE_FUNC(L, mtIndex, GetConfirmButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRejectButton);

    lua_pop(L, 1);
}

#endif
