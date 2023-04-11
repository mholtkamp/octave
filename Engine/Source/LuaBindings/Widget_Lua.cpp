#include "LuaBindings/Widget_Lua.h"

#include "Log.h"
#include "Engine.h"
#include "Renderer.h"

#include "Components/Component.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/ScriptWidget_Lua.h"

#if  LUA_ENABLED

int Widget_Lua::Create(lua_State* L, Widget* widget)
{
    if (widget != nullptr)
    {
        Widget_Lua* widgetLua = (Widget_Lua*)lua_newuserdata(L, sizeof(Widget_Lua));
        widgetLua->mWidget = widget;
        widgetLua->mAlloced = false; // This is only a wrapper. Not reposible for mWidget allocation.

        int udIndex = lua_gettop(L);

        luaL_getmetatable(L, widget->GetClassName());
        if (lua_isnil(L, -1))
        {
            // Could not find this type's metatable, so just use top level
            LogWarning("Could not find object's metatable, so the top-level metatable will be used.")

            lua_pop(L, 1);
            luaL_getmetatable(L, WIDGET_LUA_NAME);
        }

        OCT_ASSERT(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Widget_Lua::CreateNew(lua_State* L)
{
    // This func will create a new Widget(), and mark the mScriptOwned flag.
    // Will also flag the Widget_Lua wrapper object mAlloced flag to indicate that this 
    // Widget_Lua userdata object should delete the widget when __gc is called.
    const char* className = "Widget";

    int numArgs = lua_gettop(L);
    if (numArgs >= 1 && lua_isstring(L, 1))
    {
        className = lua_tostring(L, 1);
    }

    return CreateNew(L, className);
}

int Widget_Lua::CreateNew(lua_State* L, const char* className, Widget** outWidget)
{
    Widget* widget = CreateWidget(className);

    if (widget != nullptr)
    {
        widget->SetScriptOwned(true);
        Widget_Lua::Create(L, widget);
        Widget_Lua* widgetLua = (Widget_Lua*)lua_touserdata(L, -1);
        widgetLua->mAlloced = true;
    }
    else
    {
        LogError("Failed to instantiate widget class %s", className);
        lua_pushnil(L);
    }

    if (outWidget != nullptr)
    {
        *outWidget = widget;
    }

    return 1;
}

int Widget_Lua::Destroy(lua_State* L)
{
    // This function will be called when garbage collecting widget userdata but
    // ALSO when garbage collecting the Widget metatables during shutdown.
    // So first check if we are Destroy()ing a Widget userdata.

    Widget_Lua* widgetLua = nullptr;
    if (lua_isuserdata(L, 1))
    {
        // Calling CHECK_WIDGET() here may cause a crash if the widget was managed by native code
        // and was already deleted. So here... just assume we are only working on Widgets.
        // And hopefully we crash if whatever is being Destroy()'d isn't actually a widget.

        //CHECK_WIDGET(L, 1);
        widgetLua = (Widget_Lua*)lua_touserdata(L, 1);
    }

    // We need to delete the mWidget data if this wrapper allocated the data.
    if (widgetLua && 
        widgetLua->mAlloced)
    {
        Widget* widget = widgetLua->mWidget;

        if (widget->GetParent() != nullptr)
        {
            LogWarning("Detaching script-owned widget from parent because it is beging GC'd.");
        }

        Renderer::Get()->RemoveWidget(widget, 0);
        Renderer::Get()->RemoveWidget(widget, 1);
        widgetLua->mWidget->DetachFromParent();
        delete widgetLua->mWidget;
        widgetLua->mWidget = nullptr;
    }

    return 0;
}

int Widget_Lua::Equals(lua_State* L)
{
    Widget* widgetA = CHECK_WIDGET(L, 1);
    Widget* widgetB = nullptr;

    if (lua_isuserdata(L, 2))
    {
        widgetB = CHECK_WIDGET(L, 2);
    }

    bool ret = (widgetA == widgetB);

    lua_pushboolean(L, ret);
    return 1;
}

int Widget_Lua::GetName(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    const char* ret = widget->GetName().c_str();

    lua_pushstring(L, ret);
    return 1;
}

int Widget_Lua::SetName(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    const char* name = CHECK_STRING(L, 2);

    widget->SetName(name);

    return 0;
}

int Widget_Lua::FindChild(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    const char* name = CHECK_STRING(L, 2);
    bool recurse = false;
    if (!lua_isnone(L,3)) { recurse = CHECK_BOOLEAN(L, 3); }

    Widget* ret = widget->FindChild(name, recurse);

    Widget_Lua::Create(L, ret);
    return 1;
}

int Widget_Lua::CreateChildWidget(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    const char* typeName = CHECK_STRING(L, 2);

    Widget* childWidget =  CreateWidget(typeName);

    if (childWidget != nullptr)
    {
        widget->AddChild(childWidget);
    }

    Widget_Lua::Create(L, childWidget);

    return 1;
}

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

int Widget_Lua::SetVisible(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    widget->SetVisible(value);

    return 0;
}

int Widget_Lua::IsVisible(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    bool ret = widget->IsVisible();

    lua_pushboolean(L, ret);
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

int Widget_Lua::AddChild(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    Widget* child = CHECK_WIDGET(L, 2);
    int32_t index = -1;
    if (!lua_isnone(L, 3)) { index = (int32_t)CHECK_INTEGER(L, 3); }

    widget->AddChild(child, index);

    return 0;
}

int Widget_Lua::RemoveChild(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    if (lua_isinteger(L, 2))
    {
        int32_t index = (int32_t)CHECK_INTEGER(L, 2);
        widget->RemoveChild(index);
    }
    else
    {
        Widget* child = CHECK_WIDGET(L, 2);
        widget->RemoveChild(child);
    }

    return 0;
}

int Widget_Lua::GetChild(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    // Convert index from 1 based to 0 based.
    --index;

    Widget* child = widget->GetChild(index);

    Widget_Lua::Create(L, child);
    return 1;
}

int Widget_Lua::GetParent(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    Widget* parent = widget->GetParent();

    Widget_Lua::Create(L, parent);
    return 1;
}

int Widget_Lua::DetachFromParent(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    widget->DetachFromParent();

    return 0;
}

int Widget_Lua::GetNumChildren(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    uint32_t ret = widget->GetNumChildren();

    lua_pushinteger(L, (int)ret);
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

int Widget_Lua::CheckType(lua_State* L)
{
    bool ret = false;
    CHECK_WIDGET(L, 1);
    const char* typeName = CHECK_STRING(L, 2);

    if (lua_getmetatable(L, 1))
    {
        char classFlag[64];
        snprintf(classFlag, 64, "cf%s", typeName);
        lua_getfield(L, 1, classFlag);

        if (!lua_isnil(L, -1))
        {
            ret = true;
        }
    }

    return ret;
}

void Widget_Lua::BindCommon(lua_State* L, int mtIndex)
{
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, Destroy);
    lua_rawset(L, mtIndex);

    lua_pushstring(L, "__eq");
    lua_pushcfunction(L, Equals);
    lua_rawset(L, mtIndex);

    bool isScriptWidget = (lua_getfield(L, mtIndex, SCRIPT_WIDGET_LUA_FLAG) != LUA_TNIL);
    if (isScriptWidget)
    {
        // If this is a ScriptWidget, then we need to set its __index and __newIndex
        // This lets use invoke script fuctions directly from the ScriptWidget userdata, and we don't
        // need to write code like: myWidget:GetScript():MyNativeFunc()
        // instead it's just myWidget:MyNativeFunc()
        lua_pushstring(L, "__index");
        lua_pushcfunction(L, ScriptWidget_Lua::CustomIndex);
        lua_rawset(L, mtIndex);

        lua_pushstring(L, "__newindex");
        lua_pushcfunction(L, ScriptWidget_Lua::CustomNewIndex);
        lua_rawset(L, mtIndex);
    }

    lua_pop(L, 1);
}

void Widget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        WIDGET_LUA_NAME,
        WIDGET_LUA_FLAG,
        nullptr);

    BindCommon(L, mtIndex);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Equals);
    lua_setfield(L, mtIndex, "Equals");

    lua_pushcfunction(L, GetName);
    lua_setfield(L, mtIndex, "GetName");

    lua_pushcfunction(L, SetName);
    lua_setfield(L, mtIndex, "SetName");

    lua_pushcfunction(L, FindChild);
    lua_setfield(L, mtIndex, "FindChild");

    lua_pushcfunction(L, CreateChildWidget);
    lua_setfield(L, mtIndex, "CreateChildWidget");

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

    lua_pushcfunction(L, SetVisible);
    lua_setfield(L, mtIndex, "SetVisible");

    lua_pushcfunction(L, IsVisible);
    lua_setfield(L, mtIndex, "IsVisible");

    lua_pushcfunction(L, SetColor);
    lua_setfield(L, mtIndex, "SetColor");

    lua_pushcfunction(L, ShouldHandleInput);
    lua_setfield(L, mtIndex, "ShouldHandleInput");

    lua_pushcfunction(L, AddChild);
    lua_setfield(L, mtIndex, "AddChild");

    lua_pushcfunction(L, RemoveChild);
    lua_setfield(L, mtIndex, "RemoveChild");

    lua_pushcfunction(L, GetChild);
    lua_setfield(L, mtIndex, "GetChild");

    lua_pushcfunction(L, GetParent);
    lua_setfield(L, mtIndex, "GetParent");

    lua_pushcfunction(L, DetachFromParent);
    lua_setfield(L, mtIndex, "DetachFromParent");

    lua_pushcfunction(L, GetNumChildren);
    lua_setfield(L, mtIndex, "GetNumChildren");

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

    lua_pushcfunction(L, CheckType);
    lua_setfield(L, mtIndex, "CheckType");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif