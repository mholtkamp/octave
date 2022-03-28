#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Font_Lua.h"

#if LUA_ENABLED

int Text_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, TEXT_LUA_NAME);
}

int Text_Lua::SetFont(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    Font* font = CHECK_FONT(L, 2);

    text->SetFont(font);

    return 0;
}

int Text_Lua::GetFont(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    Font* font = text->GetFont();

    Asset_Lua::Create(L, font);
    return 1;
}

int Text_Lua::SetOutlineColor(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    text->SetOutlineColor(value);

    return 0;
}

int Text_Lua::GetOutlineColor(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    glm::vec4 ret = text->GetOutlineColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Text_Lua::SetSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    text->SetSize(value);

    return 0;
}

int Text_Lua::GetSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetScaledSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetScaledSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetOutlineSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetOutlineSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetSoftness(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetSoftness();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetCutoff(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetCutoff();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::SetText(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    const char* value = CHECK_STRING(L, 2);

    text->SetText(value);

    return 0;
}

int Text_Lua::GetText(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    const char* ret = text->GetText().c_str();

    lua_pushstring(L, ret);
    return 1;
}

int Text_Lua::GetTextWidth(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetTextWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetTextHeight(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetTextHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetScaledMinExtent(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    glm::vec2 ret = text->GetScaledMinExtent();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Text_Lua::GetScaledMaxExtent(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    glm::vec2 ret = text->GetScaledMaxExtent();

    Vector_Lua::Create(L, ret);
    return 1;
}

void Text_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_LUA_NAME,
        TEXT_LUA_FLAG,
        WIDGET_LUA_NAME);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Widget_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, SetFont);
    lua_setfield(L, mtIndex, "SetFont");

    lua_pushcfunction(L, GetFont);
    lua_setfield(L, mtIndex, "GetFont");

    lua_pushcfunction(L, SetOutlineColor);
    lua_setfield(L, mtIndex, "SetOutlineColor");

    lua_pushcfunction(L, GetOutlineColor);
    lua_setfield(L, mtIndex, "GetOutlineColor");

    lua_pushcfunction(L, SetSize);
    lua_setfield(L, mtIndex, "SetSize");

    lua_pushcfunction(L, GetSize);
    lua_setfield(L, mtIndex, "GetSize");

    lua_pushcfunction(L, GetScaledSize);
    lua_setfield(L, mtIndex, "GetScaledSize");

    lua_pushcfunction(L, GetOutlineSize);
    lua_setfield(L, mtIndex, "GetOutlineSize");

    lua_pushcfunction(L, GetSoftness);
    lua_setfield(L, mtIndex, "GetSoftness");

    lua_pushcfunction(L, GetCutoff);
    lua_setfield(L, mtIndex, "GetCutoff");

    lua_pushcfunction(L, SetText);
    lua_setfield(L, mtIndex, "SetText");

    lua_pushcfunction(L, GetText);
    lua_setfield(L, mtIndex, "GetText");

    lua_pushcfunction(L, GetTextWidth);
    lua_setfield(L, mtIndex, "GetTextWidth");

    lua_pushcfunction(L, GetTextHeight);
    lua_setfield(L, mtIndex, "GetTextHeight");

    lua_pushcfunction(L, GetScaledMinExtent);
    lua_setfield(L, mtIndex, "GetScaledMinExtent");

    lua_pushcfunction(L, GetScaledMaxExtent);
    lua_setfield(L, mtIndex, "GetScaledMaxExtent");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
