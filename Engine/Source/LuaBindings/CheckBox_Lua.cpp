#include "LuaBindings/CheckBox_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int CheckBox_Lua::SetChecked(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    bool checked = CHECK_BOOLEAN(L, 2);

    checkBox->SetChecked(checked);

    return 0;
}

int CheckBox_Lua::IsChecked(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    bool ret = checkBox->IsChecked();

    lua_pushboolean(L, ret);
    return 1;
}

int CheckBox_Lua::Toggle(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    checkBox->Toggle();

    return 0;
}

int CheckBox_Lua::SetText(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    const char* text = CHECK_STRING(L, 2);

    checkBox->SetText(text);

    return 0;
}

int CheckBox_Lua::GetText(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    const std::string& ret = checkBox->GetText();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int CheckBox_Lua::SetCheckedColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    checkBox->SetCheckedColor(color);

    return 0;
}

int CheckBox_Lua::GetCheckedColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    glm::vec4 ret = checkBox->GetCheckedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int CheckBox_Lua::SetUncheckedColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    checkBox->SetUncheckedColor(color);

    return 0;
}

int CheckBox_Lua::GetUncheckedColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    glm::vec4 ret = checkBox->GetUncheckedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int CheckBox_Lua::SetTextColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    checkBox->SetTextColor(color);

    return 0;
}

int CheckBox_Lua::GetTextColor(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    glm::vec4 ret = checkBox->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int CheckBox_Lua::SetBoxSize(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    float size = CHECK_NUMBER(L, 2);

    checkBox->SetBoxSize(size);

    return 0;
}

int CheckBox_Lua::GetBoxSize(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    float ret = checkBox->GetBoxSize();

    lua_pushnumber(L, ret);
    return 1;
}

int CheckBox_Lua::SetSpacing(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);
    float spacing = CHECK_NUMBER(L, 2);

    checkBox->SetSpacing(spacing);

    return 0;
}

int CheckBox_Lua::GetSpacing(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    float ret = checkBox->GetSpacing();

    lua_pushnumber(L, ret);
    return 1;
}

int CheckBox_Lua::GetBoxQuad(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    Quad* ret = checkBox->GetBoxQuad();

    Node_Lua::Create(L, ret);
    return 1;
}

int CheckBox_Lua::GetTextWidget(lua_State* L)
{
    CheckBox* checkBox = CHECK_CHECKBOX(L, 1);

    Text* ret = checkBox->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

void CheckBox_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CHECKBOX_LUA_NAME,
        CHECKBOX_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // State
    REGISTER_TABLE_FUNC(L, mtIndex, SetChecked);
    REGISTER_TABLE_FUNC(L, mtIndex, IsChecked);
    REGISTER_TABLE_FUNC(L, mtIndex, Toggle);

    // Text
    REGISTER_TABLE_FUNC(L, mtIndex, SetText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetText);

    // Visual
    REGISTER_TABLE_FUNC(L, mtIndex, SetCheckedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCheckedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetUncheckedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetUncheckedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetBoxSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBoxSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSpacing);

    // Children
    REGISTER_TABLE_FUNC(L, mtIndex, GetBoxQuad);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidget);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
