#include "ToolTipWidget_Lua.h"

#if LUA_ENABLED

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaTypeCheck.h"
#include "Nodes/Widgets/ToolTipWidget.h"

// Content

int ToolTipWidget_Lua::SetTooltipTitle(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    const char* value = CHECK_STRING(L, 2);
    widget->SetTooltipTitle(value);
    return 0;
}

int ToolTipWidget_Lua::GetTooltipTitle(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushstring(L, widget->GetTooltipTitle().c_str());
    return 1;
}

int ToolTipWidget_Lua::SetTooltipText(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    const char* value = CHECK_STRING(L, 2);
    widget->SetTooltipText(value);
    return 0;
}

int ToolTipWidget_Lua::GetTooltipText(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushstring(L, widget->GetTooltipText().c_str());
    return 1;
}

int ToolTipWidget_Lua::SetContent(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    const char* name = CHECK_STRING(L, 2);
    const char* desc = CHECK_STRING(L, 3);
    widget->SetContent(name, desc);
    return 0;
}

int ToolTipWidget_Lua::ConfigureFromWidget(lua_State* L)
{
    ToolTipWidget* tooltip = CHECK_TOOLTIPWIDGET(L, 1);
    Widget* source = CHECK_WIDGET(L, 2);
    tooltip->ConfigureFromWidget(source);
    return 0;
}

// Styling - Background

int ToolTipWidget_Lua::SetBackgroundColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);
    widget->SetBackgroundColor(color);
    return 0;
}

int ToolTipWidget_Lua::GetBackgroundColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    Vector_Lua::Create(L, widget->GetBackgroundColor());
    return 1;
}

int ToolTipWidget_Lua::SetCornerRadius(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);
    widget->SetCornerRadius(value);
    return 0;
}

int ToolTipWidget_Lua::GetCornerRadius(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetCornerRadius());
    return 1;
}

// Styling - Text

int ToolTipWidget_Lua::SetTitleFontSize(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);
    widget->SetTitleFontSize(value);
    return 0;
}

int ToolTipWidget_Lua::GetTitleFontSize(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetTitleFontSize());
    return 1;
}

int ToolTipWidget_Lua::SetTextFontSize(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);
    widget->SetTextFontSize(value);
    return 0;
}

int ToolTipWidget_Lua::GetTextFontSize(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetTextFontSize());
    return 1;
}

int ToolTipWidget_Lua::SetTitleColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);
    widget->SetTitleColor(color);
    return 0;
}

int ToolTipWidget_Lua::GetTitleColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    Vector_Lua::Create(L, widget->GetTitleColor());
    return 1;
}

int ToolTipWidget_Lua::SetTextColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);
    widget->SetTextColor(color);
    return 0;
}

int ToolTipWidget_Lua::GetTextColor(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    Vector_Lua::Create(L, widget->GetTextColor());
    return 1;
}

// Styling - Layout

int ToolTipWidget_Lua::SetPadding(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float left = CHECK_NUMBER(L, 2);
    float top = CHECK_NUMBER(L, 3);
    float right = CHECK_NUMBER(L, 4);
    float bottom = CHECK_NUMBER(L, 5);
    widget->SetPadding(left, top, right, bottom);
    return 0;
}

int ToolTipWidget_Lua::GetPaddingLeft(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetPaddingLeft());
    return 1;
}

int ToolTipWidget_Lua::GetPaddingTop(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetPaddingTop());
    return 1;
}

int ToolTipWidget_Lua::GetPaddingRight(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetPaddingRight());
    return 1;
}

int ToolTipWidget_Lua::GetPaddingBottom(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetPaddingBottom());
    return 1;
}

int ToolTipWidget_Lua::SetMaxWidth(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);
    widget->SetMaxWidth(value);
    return 0;
}

int ToolTipWidget_Lua::GetMaxWidth(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetMaxWidth());
    return 1;
}

int ToolTipWidget_Lua::SetTitleTextSpacing(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);
    widget->SetTitleTextSpacing(value);
    return 0;
}

int ToolTipWidget_Lua::GetTitleTextSpacing(lua_State* L)
{
    ToolTipWidget* widget = CHECK_TOOLTIPWIDGET(L, 1);
    lua_pushnumber(L, widget->GetTitleTextSpacing());
    return 1;
}

void ToolTipWidget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TOOLTIPWIDGET_LUA_NAME,
        TOOLTIPWIDGET_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Content
    REGISTER_TABLE_FUNC(L, mtIndex, SetTooltipTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTooltipTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTooltipText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTooltipText);
    REGISTER_TABLE_FUNC(L, mtIndex, SetContent);
    REGISTER_TABLE_FUNC(L, mtIndex, ConfigureFromWidget);

    // Styling - Background
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCornerRadius);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCornerRadius);

    // Styling - Text
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);

    // Styling - Layout
    REGISTER_TABLE_FUNC(L, mtIndex, SetPadding);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPaddingLeft);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPaddingTop);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPaddingRight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPaddingBottom);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleTextSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleTextSpacing);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
