#include "LuaBindings/SpinBox_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int SpinBox_Lua::SetValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    float value = CHECK_NUMBER(L, 2);

    spinBox->SetValue(value);

    return 0;
}

int SpinBox_Lua::GetValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    float ret = spinBox->GetValue();

    lua_pushnumber(L, ret);
    return 1;
}

int SpinBox_Lua::SetMinValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    float value = CHECK_NUMBER(L, 2);

    spinBox->SetMinValue(value);

    return 0;
}

int SpinBox_Lua::GetMinValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    float ret = spinBox->GetMinValue();

    lua_pushnumber(L, ret);
    return 1;
}

int SpinBox_Lua::SetMaxValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    float value = CHECK_NUMBER(L, 2);

    spinBox->SetMaxValue(value);

    return 0;
}

int SpinBox_Lua::GetMaxValue(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    float ret = spinBox->GetMaxValue();

    lua_pushnumber(L, ret);
    return 1;
}

int SpinBox_Lua::SetStep(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    float step = CHECK_NUMBER(L, 2);

    spinBox->SetStep(step);

    return 0;
}

int SpinBox_Lua::GetStep(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    float ret = spinBox->GetStep();

    lua_pushnumber(L, ret);
    return 1;
}

int SpinBox_Lua::SetPrefix(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    const char* prefix = CHECK_STRING(L, 2);

    spinBox->SetPrefix(prefix);

    return 0;
}

int SpinBox_Lua::GetPrefix(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    const std::string& ret = spinBox->GetPrefix();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int SpinBox_Lua::SetSuffix(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    const char* suffix = CHECK_STRING(L, 2);

    spinBox->SetSuffix(suffix);

    return 0;
}

int SpinBox_Lua::GetSuffix(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    const std::string& ret = spinBox->GetSuffix();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int SpinBox_Lua::SetBackgroundColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    spinBox->SetBackgroundColor(color);

    return 0;
}

int SpinBox_Lua::GetBackgroundColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    glm::vec4 ret = spinBox->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::SetTextColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    spinBox->SetTextColor(color);

    return 0;
}

int SpinBox_Lua::GetTextColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    glm::vec4 ret = spinBox->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::SetButtonColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    spinBox->SetButtonColor(color);

    return 0;
}

int SpinBox_Lua::GetButtonColor(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    glm::vec4 ret = spinBox->GetButtonColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::GetBackground(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    Quad* ret = spinBox->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::GetTextWidget(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    Text* ret = spinBox->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::GetIncrementButton(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    Button* ret = spinBox->GetIncrementButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int SpinBox_Lua::GetDecrementButton(lua_State* L)
{
    SpinBox* spinBox = CHECK_SPINBOX(L, 1);

    Button* ret = spinBox->GetDecrementButton();

    Node_Lua::Create(L, ret);
    return 1;
}

void SpinBox_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SPINBOX_LUA_NAME,
        SPINBOX_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Value
    REGISTER_TABLE_FUNC(L, mtIndex, SetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStep);
    REGISTER_TABLE_FUNC(L, mtIndex, GetStep);

    // Display
    REGISTER_TABLE_FUNC(L, mtIndex, SetPrefix);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPrefix);
    REGISTER_TABLE_FUNC(L, mtIndex, SetSuffix);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSuffix);

    // Visual
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetButtonColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetButtonColor);

    // Children
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackground);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, GetIncrementButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetDecrementButton);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
