#include "LuaBindings/ProgressBar_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int ProgressBar_Lua::SetValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    float value = CHECK_NUMBER(L, 2);

    progressBar->SetValue(value);

    return 0;
}

int ProgressBar_Lua::GetValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    float ret = progressBar->GetValue();

    lua_pushnumber(L, ret);
    return 1;
}

int ProgressBar_Lua::SetMinValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    float value = CHECK_NUMBER(L, 2);

    progressBar->SetMinValue(value);

    return 0;
}

int ProgressBar_Lua::GetMinValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    float ret = progressBar->GetMinValue();

    lua_pushnumber(L, ret);
    return 1;
}

int ProgressBar_Lua::SetMaxValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    float value = CHECK_NUMBER(L, 2);

    progressBar->SetMaxValue(value);

    return 0;
}

int ProgressBar_Lua::GetMaxValue(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    float ret = progressBar->GetMaxValue();

    lua_pushnumber(L, ret);
    return 1;
}

int ProgressBar_Lua::GetRatio(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    float ret = progressBar->GetRatio();

    lua_pushnumber(L, ret);
    return 1;
}

int ProgressBar_Lua::SetShowPercentage(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    bool show = CHECK_BOOLEAN(L, 2);

    progressBar->SetShowPercentage(show);

    return 0;
}

int ProgressBar_Lua::IsShowingPercentage(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    bool ret = progressBar->IsShowingPercentage();

    lua_pushboolean(L, ret);
    return 1;
}

int ProgressBar_Lua::SetBackgroundColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    progressBar->SetBackgroundColor(color);

    return 0;
}

int ProgressBar_Lua::GetBackgroundColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    glm::vec4 ret = progressBar->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ProgressBar_Lua::SetFillColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    progressBar->SetFillColor(color);

    return 0;
}

int ProgressBar_Lua::GetFillColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    glm::vec4 ret = progressBar->GetFillColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ProgressBar_Lua::SetTextColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    progressBar->SetTextColor(color);

    return 0;
}

int ProgressBar_Lua::GetTextColor(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    glm::vec4 ret = progressBar->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ProgressBar_Lua::GetBackgroundQuad(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    Quad* ret = progressBar->GetBackgroundQuad();

    Node_Lua::Create(L, ret);
    return 1;
}

int ProgressBar_Lua::GetFillQuad(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    Quad* ret = progressBar->GetFillQuad();

    Node_Lua::Create(L, ret);
    return 1;
}

int ProgressBar_Lua::GetTextWidget(lua_State* L)
{
    ProgressBar* progressBar = CHECK_PROGRESSBAR(L, 1);

    Text* ret = progressBar->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

void ProgressBar_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PROGRESSBAR_LUA_NAME,
        PROGRESSBAR_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Value
    REGISTER_TABLE_FUNC(L, mtIndex, SetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetRatio);

    // Display
    REGISTER_TABLE_FUNC(L, mtIndex, SetShowPercentage);
    REGISTER_TABLE_FUNC(L, mtIndex, IsShowingPercentage);

    // Visual
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetFillColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetFillColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);

    // Children
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundQuad);
    REGISTER_TABLE_FUNC(L, mtIndex, GetFillQuad);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidget);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
