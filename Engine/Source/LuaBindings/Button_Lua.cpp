#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"
#include "AssetManager.h"

#if LUA_ENABLED

int Button_Lua::SetSelected(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Button::SetSelectedButton(button);

    return 0;
}

int Button_Lua::GetSelected(lua_State* L)
{
    Button* button = Button::GetSelectedButton();

    Node_Lua::Create(L, button);
    return 1;
}

int Button_Lua::EnableMouseHandling(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);
    Button::SetHandleMouse(enable);
    return 0;
}

int Button_Lua::EnableGamepadHandling(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);
    Button::SetHandleGamepad(enable);
    return 0;
}

int Button_Lua::EnableKeyboardHandling(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);
    Button::SetHandleKeyboard(enable);
    return 0;
}

int Button_Lua::IsSelected(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    
    bool selected = button->IsSelected();

    lua_pushboolean(L, selected);
    return 1;
}

int Button_Lua::Activate(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    button->Activate();
    return 0;
}

int Button_Lua::GetState(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    ButtonState ret = button->GetState();

    lua_pushinteger(L, (int32_t)ret);
    return 1;
}

int Button_Lua::SetLocked(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    button->SetState(ButtonState::Locked);

    return 0;
}

int Button_Lua::SetTextString(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    const char* value = CHECK_STRING(L, 2);

    button->SetTextString(value);

    return 0;
}

int Button_Lua::GetTextString(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    const std::string& ret = button->GetTextString();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int Button_Lua::SetStateTextures(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Texture* normal = nullptr;
    Texture* hovered = nullptr;
    Texture* pressed = nullptr;
    Texture* locked = nullptr;
    if (!lua_isnil(L, 2)) { normal = CHECK_TEXTURE(L, 2); }
    if (!lua_isnil(L, 3)) { hovered = CHECK_TEXTURE(L, 3); }
    if (!lua_isnil(L, 4)) { pressed = CHECK_TEXTURE(L, 4); }
    if (!lua_isnil(L, 5)) { locked = CHECK_TEXTURE(L, 5); }

    button->SetNormalTexture(normal);
    button->SetHoveredTexture(hovered);
    button->SetPressedTexture(pressed);
    button->SetLockedTexture(locked);

    return 0;
}

int Button_Lua::SetStateColors(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    glm::vec4 normal = CHECK_VECTOR(L, 2);
    glm::vec4 hovered = CHECK_VECTOR(L, 3);
    glm::vec4 pressed = CHECK_VECTOR(L, 4);
    glm::vec4 locked = CHECK_VECTOR(L, 5);

    button->SetNormalColor(normal);
    button->SetHoveredColor(hovered);
    button->SetPressedColor(pressed);
    button->SetLockedColor(locked);

    return 0;
}

int Button_Lua::GetStateTextures(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    Texture* normal = button->GetNormalTexture();
    Texture* hovered = button->GetHoveredTexture();
    Texture* pressed = button->GetPressedTexture();
    Texture* locked = button->GetLockedTexture();

    Asset_Lua::Create(L, normal);
    Asset_Lua::Create(L, hovered);
    Asset_Lua::Create(L, pressed);
    Asset_Lua::Create(L, locked);
    return 4;
}

int Button_Lua::GetStateColors(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    glm::vec4 normal = button->GetNormalColor();
    glm::vec4 hovered = button->GetHoveredColor();
    glm::vec4 pressed = button->GetPressedColor();
    glm::vec4 locked = button->GetLockedColor();

    Vector_Lua::Create(L, normal);
    Vector_Lua::Create(L, hovered);
    Vector_Lua::Create(L, pressed);
    Vector_Lua::Create(L, locked);
    return 4;
}

int Button_Lua::SetNavigation(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Node* up = lua_isnoneornil(L, 2) ? nullptr : CHECK_NODE(L, 2);
    Node* down = lua_isnoneornil(L, 3) ? nullptr : CHECK_NODE(L, 3);
    Node* left = lua_isnoneornil(L, 4) ? nullptr : CHECK_NODE(L, 4);
    Node* right = lua_isnoneornil(L, 5) ? nullptr : CHECK_NODE(L, 5);

    button->SetNavUp(up);
    button->SetNavDown(down);
    button->SetNavLeft(left);
    button->SetNavRight(right);

    return 0;
}

int Button_Lua::GetNavigation(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    Node* up = button->GetNavUp();
    Node* down = button->GetNavDown();
    Node* left = button->GetNavLeft();
    Node* right = button->GetNavRight();

    Node_Lua::Create(L, up);
    Node_Lua::Create(L, down);
    Node_Lua::Create(L, left);
    Node_Lua::Create(L, right);

    return 4;
}

int Button_Lua::GetText(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    Text* ret = button->GetText();

    Node_Lua::Create(L, ret);
    return 1;
}

int Button_Lua::GetQuad(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    Quad* ret = button->GetQuad();

    Node_Lua::Create(L, ret);
    return 1;
}

void Button_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        BUTTON_LUA_NAME,
        BUTTON_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSelected);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelected);
    REGISTER_TABLE_FUNC(L, mtIndex, EnableMouseHandling);
    REGISTER_TABLE_FUNC(L, mtIndex, EnableGamepadHandling);
    REGISTER_TABLE_FUNC(L, mtIndex, EnableKeyboardHandling);
    REGISTER_TABLE_FUNC(L, mtIndex, IsSelected);
    REGISTER_TABLE_FUNC(L, mtIndex, Activate);
    REGISTER_TABLE_FUNC(L, mtIndex, GetState);
    REGISTER_TABLE_FUNC(L, mtIndex, SetLocked);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextString);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextString);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStateTextures);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStateColors);
    REGISTER_TABLE_FUNC(L, mtIndex, GetStateTextures);
    REGISTER_TABLE_FUNC(L, mtIndex, GetStateColors);
    REGISTER_TABLE_FUNC(L, mtIndex, SetNavigation);
    REGISTER_TABLE_FUNC(L, mtIndex, GetNavigation);
    REGISTER_TABLE_FUNC(L, mtIndex, GetText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetQuad);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
