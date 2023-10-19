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
    const char* tableName = CheckTableName(L, 2);
    const char* funcName = CHECK_STRING(L, 3);

    button->SetScriptHoverHandler(tableName, funcName);

    return 0;
}

int Button_Lua::SetPressedHandler(lua_State* L)
{
    Button* button = CHECK_BUTTON(L, 1);
    const char* tableName = CheckTableName(L, 2);
    const char* funcName = CHECK_STRING(L, 3);

    button->SetScriptPressedHandler(tableName, funcName);

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

    lua_pushcfunction(L, GetState);
    lua_setfield(L, mtIndex, "GetState");

    lua_pushcfunction(L, SetState);
    lua_setfield(L, mtIndex, "SetState");

    lua_pushcfunction(L, SetNormalTexture);
    lua_setfield(L, mtIndex, "SetNormalTexture");

    lua_pushcfunction(L, SetHoveredTexture);
    lua_setfield(L, mtIndex, "SetHoveredTexture");

    lua_pushcfunction(L, SetPressedTexture);
    lua_setfield(L, mtIndex, "SetPressedTexture");

    lua_pushcfunction(L, SetDisabledTexture);
    lua_setfield(L, mtIndex, "SetDisabledTexture");

    lua_pushcfunction(L, SetNormalColor);
    lua_setfield(L, mtIndex, "SetNormalColor");

    lua_pushcfunction(L, SetHoveredColor);
    lua_setfield(L, mtIndex, "SetHoveredColor");

    lua_pushcfunction(L, SetPressedColor);
    lua_setfield(L, mtIndex, "SetPressedColor");

    lua_pushcfunction(L, SetDisabledColor);
    lua_setfield(L, mtIndex, "SetDisabledColor");

    lua_pushcfunction(L, SetUseQuadStateColor);
    lua_setfield(L, mtIndex, "SetUseQuadStateColor");

    lua_pushcfunction(L, SetUseTextStateColor);
    lua_setfield(L, mtIndex, "SetUseTextStateColor");

    lua_pushcfunction(L, SetHandleMouseInput);
    lua_setfield(L, mtIndex, "SetHandleMouseInput");

    lua_pushcfunction(L, SetHoverHandler);
    lua_setfield(L, mtIndex, "SetHoverHandler");

    lua_pushcfunction(L, SetPressedHandler);
    lua_setfield(L, mtIndex, "SetPressedHandler");

    lua_pushcfunction(L, SetTextString);
    lua_setfield(L, mtIndex, "SetTextString");

    lua_pushcfunction(L, GetTextString);
    lua_setfield(L, mtIndex, "GetTextString");

    lua_pushcfunction(L, GetText);
    lua_setfield(L, mtIndex, "GetText");

    lua_pushcfunction(L, GetQuad);
    lua_setfield(L, mtIndex, "GetQuad");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
