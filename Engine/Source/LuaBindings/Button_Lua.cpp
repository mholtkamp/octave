#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Quad.h"

#if LUA_ENABLED

int Button_Lua::GetState(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);

    ButtonState ret = button->GetState();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Button_Lua::SetState(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    ButtonState value = (ButtonState) CHECK_INTEGER(L, 2);

    button->SetState(value);

    return 0;
}

int Button_Lua::SetNormalTexture(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2)) { texture = CHECK_TEXTURE(L, 2); };

    button->SetNormalTexture(texture);

    return 0;
}

int Button_Lua::SetHoveredTexture(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2)) { texture = CHECK_TEXTURE(L, 2); };

    button->SetHoveredTexture(texture);

    return 0;
}

int Button_Lua::SetPressedTexture(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2)) { texture = CHECK_TEXTURE(L, 2); };

    button->SetPressedTexture(texture);

    return 0;
}

int Button_Lua::SetDisabledTexture(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2)) { texture = CHECK_TEXTURE(L, 2); };

    button->SetDisabledTexture(texture);

    return 0;
}

int Button_Lua::SetNormalColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    button->SetNormalColor(value);

    return 0;
}

int Button_Lua::SetHoveredColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    button->SetHoveredColor(value);

    return 0;
}

int Button_Lua::SetPressedColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    button->SetPressedColor(value);

    return 0;
}

int Button_Lua::SetDisabledColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    button->SetDisabledColor(value);

    return 0;
}

int Button_Lua::SetUseQuadStateColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    button->SetUseQuadStateColor(value);

    return 0;
}

int Button_Lua::SetUseTextStateColor(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    button->SetUseTextStateColor(value);

    return 0;
}

int Button_Lua::SetHandleMouseInput(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    button->SetHandleMouseInput(value);

    return 0;
}

int Button_Lua::SetHoverHandler(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    button->SetScriptHoverHandler(func);

    return 0;
}

int Button_Lua::SetPressedHandler(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    button->SetScriptPressedHandler(func);

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

    const char* ret = button->GetTextString().c_str();

    lua_pushstring(L, ret);
    return 1;
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

    REGISTER_TABLE_FUNC(L, mtIndex, GetState);

    REGISTER_TABLE_FUNC(L, mtIndex, SetState);

    REGISTER_TABLE_FUNC(L, mtIndex, SetNormalTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHoveredTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPressedTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, SetDisabledTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, SetNormalColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHoveredColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPressedColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetDisabledColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUseQuadStateColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUseTextStateColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHandleMouseInput);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHoverHandler);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPressedHandler);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTextString);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTextString);

    REGISTER_TABLE_FUNC(L, mtIndex, GetText);

    REGISTER_TABLE_FUNC(L, mtIndex, GetQuad);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
