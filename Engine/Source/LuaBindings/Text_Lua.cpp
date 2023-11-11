#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Font_Lua.h"

#if LUA_ENABLED

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

int Text_Lua::SetTextSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    text->SetTextSize(value);

    return 0;
}

int Text_Lua::GetTextSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetTextSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Text_Lua::GetScaledTextSize(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    float ret = text->GetScaledTextSize();

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

int Text_Lua::SetHorizontalJustification(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    text->SetHorizontalJustification((Justification)value);

    return 0;
}

int Text_Lua::GetHorizontalJustification(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    Justification ret = text->GetHorizontalJustification();

    lua_pushinteger(L, (int32_t)ret);
    return 1;
}

int Text_Lua::SetVerticalJustification(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    text->SetVerticalJustification((Justification)value);

    return 0;
}

int Text_Lua::GetVerticalJustification(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    Justification ret = text->GetVerticalJustification();

    lua_pushinteger(L, (int32_t)ret);
    return 1;
}

int Text_Lua::EnableWordWrap(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    text->EnableWordWrap(enable);

    return 0;
}

int Text_Lua::IsWordWrapEnabled(lua_State* L)
{
    Text* text = CHECK_TEXT(L, 1);

    bool ret = text->IsWordWrapEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

void Text_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_LUA_NAME,
        TEXT_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFont);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFont);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOutlineColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOutlineColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTextSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTextSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScaledTextSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOutlineSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSoftness);

    REGISTER_TABLE_FUNC(L, mtIndex, GetCutoff);

    REGISTER_TABLE_FUNC(L, mtIndex, SetText);

    REGISTER_TABLE_FUNC(L, mtIndex, GetText);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTextHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScaledMinExtent);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScaledMaxExtent);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHorizontalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHorizontalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVerticalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVerticalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableWordWrap);

    REGISTER_TABLE_FUNC(L, mtIndex, IsWordWrapEnabled);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
