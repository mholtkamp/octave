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

int Button_Lua::SetState(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    button->SetState((ButtonState)value);

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

    REGISTER_TABLE_FUNC(L, mtIndex, Activate);
    REGISTER_TABLE_FUNC(L, mtIndex, GetState);
    REGISTER_TABLE_FUNC(L, mtIndex, SetState);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextString);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextString);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStateTextures);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStateColors);
    REGISTER_TABLE_FUNC(L, mtIndex, GetText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetQuad);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
