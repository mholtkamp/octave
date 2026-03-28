#include "LuaBindings/Slider_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"

#if LUA_ENABLED

int Slider_Lua::SetValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetValue(value);

    return 0;
}

int Slider_Lua::GetValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetValue();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetMinValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetMinValue(value);

    return 0;
}

int Slider_Lua::GetMinValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetMinValue();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetMaxValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetMaxValue(value);

    return 0;
}

int Slider_Lua::GetMaxValue(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetMaxValue();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetStep(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetStep(value);

    return 0;
}

int Slider_Lua::GetStep(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetStep();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetRange(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float minValue = CHECK_NUMBER(L, 2);
    float maxValue = CHECK_NUMBER(L, 3);

    slider->SetRange(minValue, maxValue);

    return 0;
}

int Slider_Lua::IsDragging(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    bool ret = slider->IsDragging();

    lua_pushboolean(L, ret);
    return 1;
}

int Slider_Lua::SetGrabberWidth(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetGrabberWidth(value);

    return 0;
}

int Slider_Lua::GetGrabberWidth(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetGrabberWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetTrackHeight(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    float value = CHECK_NUMBER(L, 2);

    slider->SetTrackHeight(value);

    return 0;
}

int Slider_Lua::GetTrackHeight(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    float ret = slider->GetTrackHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Slider_Lua::SetBackgroundColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    slider->SetBackgroundColor(color);

    return 0;
}

int Slider_Lua::GetBackgroundColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    glm::vec4 ret = slider->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Slider_Lua::SetGrabberColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    slider->SetGrabberColor(color);

    return 0;
}

int Slider_Lua::GetGrabberColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    glm::vec4 ret = slider->GetGrabberColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Slider_Lua::SetGrabberHoveredColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    slider->SetGrabberHoveredColor(color);

    return 0;
}

int Slider_Lua::GetGrabberHoveredColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    glm::vec4 ret = slider->GetGrabberHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Slider_Lua::SetGrabberPressedColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    slider->SetGrabberPressedColor(color);

    return 0;
}

int Slider_Lua::GetGrabberPressedColor(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    glm::vec4 ret = slider->GetGrabberPressedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Slider_Lua::GetBackground(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    Quad* ret = slider->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int Slider_Lua::GetGrabber(lua_State* L)
{
    Slider* slider = CHECK_SLIDER(L, 1);

    Quad* ret = slider->GetGrabber();

    Node_Lua::Create(L, ret);
    return 1;
}

void Slider_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SLIDER_LUA_NAME,
        SLIDER_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMinValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxValue);
    REGISTER_TABLE_FUNC(L, mtIndex, SetStep);
    REGISTER_TABLE_FUNC(L, mtIndex, GetStep);
    REGISTER_TABLE_FUNC(L, mtIndex, SetRange);
    REGISTER_TABLE_FUNC(L, mtIndex, IsDragging);
    REGISTER_TABLE_FUNC(L, mtIndex, SetGrabberWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetGrabberWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTrackHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTrackHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetGrabberColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetGrabberColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetGrabberHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetGrabberHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetGrabberPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetGrabberPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackground);
    REGISTER_TABLE_FUNC(L, mtIndex, GetGrabber);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
