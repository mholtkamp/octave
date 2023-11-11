#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "Log.h"
#include "Engine.h"
#include "Renderer.h"

#include "Nodes/Node.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if  LUA_ENABLED

int Widget_Lua::GetRect(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    Rect ret = widget->GetRect();

    Rect_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::UpdateRect(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    widget->UpdateRect();

    return 0;
}

int Widget_Lua::SetX(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetX(value);

    return 0;
}

int Widget_Lua::SetY(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetY(value);

    return 0;
}

int Widget_Lua::SetWidth(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetWidth(value);

    return 0;
}

int Widget_Lua::SetHeight(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetHeight(value);

    return 0;
}

int Widget_Lua::SetXRatio(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetXRatio(value);

    return 0;
}

int Widget_Lua::SetYRatio(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetYRatio(value);

    return 0;
}

int Widget_Lua::SetWidthRatio(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetWidthRatio(value);

    return 0;
}

int Widget_Lua::SetHeightRatio(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetHeightRatio(value);

    return 0;
}

int Widget_Lua::SetLeftMargin(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetLeftMargin(value);

    return 0;
}

int Widget_Lua::SetTopMargin(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetTopMargin(value);

    return 0;
}

int Widget_Lua::SetRightMargin(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetRightMargin(value);

    return 0;
}

int Widget_Lua::SetBottomMargin(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetBottomMargin(value);

    return 0;
}

int Widget_Lua::SetPosition(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 pos = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        pos.x = CHECK_NUMBER(L, 2);
        pos.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        pos = CHECK_VECTOR(L, 2);
    }

    widget->SetPosition(pos);

    return 0;
}

int Widget_Lua::SetDimensions(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 dim = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        dim.x = CHECK_NUMBER(L, 2);
        dim.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        dim = CHECK_VECTOR(L, 2);
    }

    widget->SetDimensions(dim);

    return 0;
}

int Widget_Lua::SetRect(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    Rect rect = CHECK_RECT(L, 2);

    widget->SetRect(rect);

    return 0;
}

int Widget_Lua::SetRatios(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float x = CHECK_NUMBER(L, 2);
    float y = CHECK_NUMBER(L, 3);
    float w = CHECK_NUMBER(L, 4);
    float h = CHECK_NUMBER(L, 5);

    widget->SetRatios(x, y, w, h);

    return 0;
}

int Widget_Lua::SetMargins(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float left = CHECK_NUMBER(L, 2);
    float top = CHECK_NUMBER(L, 3);
    float right = CHECK_NUMBER(L, 4);
    float bottom = CHECK_NUMBER(L, 5);

    widget->SetMargins(left, top, right, bottom);

    return 0;
}

int Widget_Lua::SetOffset(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 off = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        off.x = CHECK_NUMBER(L, 2);
        off.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        off = CHECK_VECTOR(L, 2);
    }

    widget->SetOffset(off.x, off.y);

    return 0;
}

int Widget_Lua::GetOffset(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetOffset();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::SetSize(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 size = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        size.x = CHECK_NUMBER(L, 2);
        size.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        size = CHECK_VECTOR(L, 2);
    }

    widget->SetSize(size.x, size.y);

    return 0;
}

int Widget_Lua::GetSize(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetSize();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::GetAnchorMode(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    AnchorMode ret = widget->GetAnchorMode();

    lua_pushinteger(L, (int) ret);
    return 1;
}

int Widget_Lua::SetAnchorMode(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    AnchorMode value = (AnchorMode)CHECK_INTEGER(L, 2);

    widget->SetAnchorMode(value);

    return 0;
}

int Widget_Lua::StretchX(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->StretchX();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::StretchY(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->StretchY();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::GetX(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetX();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::GetY(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetY();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::GetWidth(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::GetHeight(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::GetPosition(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetPosition();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::GetDimensions(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetDimensions();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::GetParentWidth(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetParentWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::GetParentHeight(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetParentHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::SetColor(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    widget->SetColor(value);

    return 0;
}

int Widget_Lua::ShouldHandleInput(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->ShouldHandleInput();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::MarkDirty(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    widget->MarkDirty();

    return 0;
}

int Widget_Lua::IsDirty(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->IsDirty();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::ContainsMouse(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->ContainsMouse();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::ContainsPoint(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    int32_t x = (int32_t)CHECK_INTEGER(L, 2);
    int32_t y = (int32_t)CHECK_INTEGER(L, 3);

    bool ret = widget->ContainsPoint(x, y);

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::SetRotation(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetRotation(value);

    return 0;
}

int Widget_Lua::GetRotation(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetRotation();

    lua_pushnumber(L, ret);
    return 1;
}

int Widget_Lua::SetPivot(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 value = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        value.x = CHECK_NUMBER(L, 2);
        value.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        value = CHECK_VECTOR(L, 2);
    }

    widget->SetPivot(value);

    return 0;
}

int Widget_Lua::GetPivot(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetPivot();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::SetScale(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    glm::vec2 value = { 0.0 , 0.0 };
    int numArgs = lua_gettop(L);
    if (numArgs >= 3)
    {
        value.x = CHECK_NUMBER(L, 2);
        value.y = CHECK_NUMBER(L, 3);
    }
    else
    {
        value = CHECK_VECTOR(L, 2);
    }

    widget->SetScale(value);

    return 0;
}

int Widget_Lua::GetScale(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    glm::vec2 ret = widget->GetScale();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::IsScissorEnabled(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->IsScissorEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::EnableScissor(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    widget->EnableScissor(value);

    return 0;
}

int Widget_Lua::SetOpacity(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    uint8_t value = (uint8_t) CHECK_INTEGER(L, 2);

    widget->SetOpacity(value);

    return 0;
}

int Widget_Lua::SetOpacityFloat(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    float value = CHECK_NUMBER(L, 2);

    widget->SetOpacityFloat(value);

    return 0;
}

int Widget_Lua::GetOpacity(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    uint8_t ret = widget->GetOpacity();

    lua_pushinteger(L, int32_t(ret));
    return 1;
}

int Widget_Lua::GetOpacityFloat(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    float ret = widget->GetOpacityFloat();

    lua_pushnumber(L, ret);
    return 1;
}

void Widget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        WIDGET_LUA_NAME,
        WIDGET_LUA_FLAG,
        NODE_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRect);

    REGISTER_TABLE_FUNC(L, mtIndex, UpdateRect);

    REGISTER_TABLE_FUNC(L, mtIndex, SetX);

    REGISTER_TABLE_FUNC(L, mtIndex, SetY);

    REGISTER_TABLE_FUNC(L, mtIndex, SetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, SetXRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, SetYRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, SetWidthRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHeightRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLeftMargin);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTopMargin);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRightMargin);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBottomMargin);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, SetDimensions);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRect);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRatios);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMargins);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAnchorMode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnchorMode);

    REGISTER_TABLE_FUNC(L, mtIndex, StretchX);

    REGISTER_TABLE_FUNC(L, mtIndex, StretchY);

    REGISTER_TABLE_FUNC(L, mtIndex, GetX);

    REGISTER_TABLE_FUNC(L, mtIndex, GetY);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, GetDimensions);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParentWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParentHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, ShouldHandleInput);

    REGISTER_TABLE_FUNC(L, mtIndex, MarkDirty);

    REGISTER_TABLE_FUNC(L, mtIndex, IsDirty);

    REGISTER_TABLE_FUNC(L, mtIndex, ContainsMouse);

    REGISTER_TABLE_FUNC(L, mtIndex, ContainsPoint);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRotation);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotation);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPivot);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPivot);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScale);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScale);

    REGISTER_TABLE_FUNC(L, mtIndex, IsScissorEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableScissor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOpacity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOpacityFloat);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOpacity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOpacityFloat);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif