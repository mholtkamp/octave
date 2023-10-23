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

    lua_pushcfunction(L, GetRect);
    lua_setfield(L, mtIndex, "GetRect");

    lua_pushcfunction(L, UpdateRect);
    lua_setfield(L, mtIndex, "UpdateRect");

    lua_pushcfunction(L, SetX);
    lua_setfield(L, mtIndex, "SetX");

    lua_pushcfunction(L, SetY);
    lua_setfield(L, mtIndex, "SetY");

    lua_pushcfunction(L, SetWidth);
    lua_setfield(L, mtIndex, "SetWidth");

    lua_pushcfunction(L, SetHeight);
    lua_setfield(L, mtIndex, "SetHeight");

    lua_pushcfunction(L, SetXRatio);
    lua_setfield(L, mtIndex, "SetXRatio");

    lua_pushcfunction(L, SetYRatio);
    lua_setfield(L, mtIndex, "SetYRatio");

    lua_pushcfunction(L, SetWidthRatio);
    lua_setfield(L, mtIndex, "SetWidthRatio");

    lua_pushcfunction(L, SetHeightRatio);
    lua_setfield(L, mtIndex, "SetHeightRatio");

    lua_pushcfunction(L, SetLeftMargin);
    lua_setfield(L, mtIndex, "SetLeftMargin");

    lua_pushcfunction(L, SetTopMargin);
    lua_setfield(L, mtIndex, "SetTopMargin");

    lua_pushcfunction(L, SetRightMargin);
    lua_setfield(L, mtIndex, "SetRightMargin");

    lua_pushcfunction(L, SetBottomMargin);
    lua_setfield(L, mtIndex, "SetBottomMargin");

    lua_pushcfunction(L, SetPosition);
    lua_setfield(L, mtIndex, "SetPosition");

    lua_pushcfunction(L, SetDimensions);
    lua_setfield(L, mtIndex, "SetDimensions");

    lua_pushcfunction(L, SetRect);
    lua_setfield(L, mtIndex, "SetRect");

    lua_pushcfunction(L, SetRatios);
    lua_setfield(L, mtIndex, "SetRatios");

    lua_pushcfunction(L, SetMargins);
    lua_setfield(L, mtIndex, "SetMargins");

    lua_pushcfunction(L, SetOffset);
    lua_setfield(L, mtIndex, "SetOffset");

    lua_pushcfunction(L, GetOffset);
    lua_setfield(L, mtIndex, "GetOffset");

    lua_pushcfunction(L, SetSize);
    lua_setfield(L, mtIndex, "SetSize");

    lua_pushcfunction(L, GetSize);
    lua_setfield(L, mtIndex, "GetSize");

    lua_pushcfunction(L, GetAnchorMode);
    lua_setfield(L, mtIndex, "GetAnchorMode");

    lua_pushcfunction(L, SetAnchorMode);
    lua_setfield(L, mtIndex, "SetAnchorMode");

    lua_pushcfunction(L, StretchX);
    lua_setfield(L, mtIndex, "StretchX");

    lua_pushcfunction(L, StretchY);
    lua_setfield(L, mtIndex, "StretchY");

    lua_pushcfunction(L, GetX);
    lua_setfield(L, mtIndex, "GetX");

    lua_pushcfunction(L, GetY);
    lua_setfield(L, mtIndex, "GetY");

    lua_pushcfunction(L, GetWidth);
    lua_setfield(L, mtIndex, "GetWidth");

    lua_pushcfunction(L, GetHeight);
    lua_setfield(L, mtIndex, "GetHeight");

    lua_pushcfunction(L, GetPosition);
    lua_setfield(L, mtIndex, "GetPosition");

    lua_pushcfunction(L, GetDimensions);
    lua_setfield(L, mtIndex, "GetDimensions");

    lua_pushcfunction(L, GetParentWidth);
    lua_setfield(L, mtIndex, "GetParentWidth");

    lua_pushcfunction(L, GetParentHeight);
    lua_setfield(L, mtIndex, "GetParentHeight");

    lua_pushcfunction(L, SetColor);
    lua_setfield(L, mtIndex, "SetColor");

    lua_pushcfunction(L, ShouldHandleInput);
    lua_setfield(L, mtIndex, "ShouldHandleInput");

    lua_pushcfunction(L, MarkDirty);
    lua_setfield(L, mtIndex, "MarkDirty");

    lua_pushcfunction(L, IsDirty);
    lua_setfield(L, mtIndex, "IsDirty");

    lua_pushcfunction(L, ContainsMouse);
    lua_setfield(L, mtIndex, "ContainsMouse");

    lua_pushcfunction(L, ContainsPoint);
    lua_setfield(L, mtIndex, "ContainsPoint");

    lua_pushcfunction(L, SetRotation);
    lua_setfield(L, mtIndex, "SetRotation");

    lua_pushcfunction(L, GetRotation);
    lua_setfield(L, mtIndex, "GetRotation");

    lua_pushcfunction(L, SetPivot);
    lua_setfield(L, mtIndex, "SetPivot");

    lua_pushcfunction(L, GetPivot);
    lua_setfield(L, mtIndex, "GetPivot");

    lua_pushcfunction(L, SetScale);
    lua_setfield(L, mtIndex, "SetScale");

    lua_pushcfunction(L, GetScale);
    lua_setfield(L, mtIndex, "GetScale");

    lua_pushcfunction(L, IsScissorEnabled);
    lua_setfield(L, mtIndex, "IsScissorEnabled");

    lua_pushcfunction(L, EnableScissor);
    lua_setfield(L, mtIndex, "EnableScissor");

    lua_pushcfunction(L, SetOpacity);
    lua_setfield(L, mtIndex, "SetOpacity");

    lua_pushcfunction(L, SetOpacityFloat);
    lua_setfield(L, mtIndex, "SetOpacityFloat");

    lua_pushcfunction(L, GetOpacity);
    lua_setfield(L, mtIndex, "GetOpacity");

    lua_pushcfunction(L, GetOpacityFloat);
    lua_setfield(L, mtIndex, "GetOpacityFloat");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif