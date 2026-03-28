#include "LuaBindings/LineEdit_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/InputField_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#include "Nodes/Widgets/InputField.h"

#if LUA_ENABLED

int LineEdit_Lua::SetTitle(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    const char* title = CHECK_STRING(L, 2);

    lineEdit->SetTitle(title);

    return 0;
}

int LineEdit_Lua::GetTitle(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    const std::string& ret = lineEdit->GetTitle();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int LineEdit_Lua::SetTitleWidth(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    float width = CHECK_NUMBER(L, 2);

    lineEdit->SetTitleWidth(width);

    return 0;
}

int LineEdit_Lua::GetTitleWidth(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    float ret = lineEdit->GetTitleWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int LineEdit_Lua::SetText(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    const char* text = CHECK_STRING(L, 2);

    lineEdit->SetText(text);

    return 0;
}

int LineEdit_Lua::GetText(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    const std::string& ret = lineEdit->GetText();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int LineEdit_Lua::SetPlaceholder(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    const char* placeholder = CHECK_STRING(L, 2);

    lineEdit->SetPlaceholder(placeholder);

    return 0;
}

int LineEdit_Lua::GetPlaceholder(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    const std::string& ret = lineEdit->GetPlaceholder();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int LineEdit_Lua::SetCaretPosition(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    int32_t pos = CHECK_INTEGER(L, 2);

    lineEdit->SetCaretPosition(pos);

    return 0;
}

int LineEdit_Lua::GetCaretPosition(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    int32_t ret = lineEdit->GetCaretPosition();

    lua_pushinteger(L, ret);
    return 1;
}

int LineEdit_Lua::SelectAll(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    lineEdit->SelectAll();

    return 0;
}

int LineEdit_Lua::ClearSelection(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    lineEdit->ClearSelection();

    return 0;
}

int LineEdit_Lua::HasSelection(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    bool ret = lineEdit->HasSelection();

    lua_pushboolean(L, ret);
    return 1;
}

int LineEdit_Lua::GetSelectedText(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    std::string ret = lineEdit->GetSelectedText();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int LineEdit_Lua::DeleteSelection(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    lineEdit->DeleteSelection();

    return 0;
}

int LineEdit_Lua::Select(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    int32_t start = CHECK_INTEGER(L, 2);
    int32_t end = CHECK_INTEGER(L, 3);

    lineEdit->Select(start, end);

    return 0;
}

int LineEdit_Lua::GetSelectionStart(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    int32_t ret = lineEdit->GetSelectionStart();

    lua_pushinteger(L, ret);
    return 1;
}

int LineEdit_Lua::GetSelectionEnd(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    int32_t ret = lineEdit->GetSelectionEnd();

    lua_pushinteger(L, ret);
    return 1;
}

int LineEdit_Lua::SetFocused(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    bool focused = CHECK_BOOLEAN(L, 2);

    lineEdit->SetFocused(focused);

    return 0;
}

int LineEdit_Lua::IsFocused(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    bool ret = lineEdit->IsFocused();

    lua_pushboolean(L, ret);
    return 1;
}

int LineEdit_Lua::SetPasswordMode(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    bool enabled = CHECK_BOOLEAN(L, 2);

    lineEdit->SetPasswordMode(enabled);

    return 0;
}

int LineEdit_Lua::IsPasswordMode(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    bool ret = lineEdit->IsPasswordMode();

    lua_pushboolean(L, ret);
    return 1;
}

int LineEdit_Lua::SetMaxLength(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    int32_t maxLen = CHECK_INTEGER(L, 2);

    lineEdit->SetMaxLength(maxLen);

    return 0;
}

int LineEdit_Lua::GetMaxLength(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    int32_t ret = lineEdit->GetMaxLength();

    lua_pushinteger(L, ret);
    return 1;
}

int LineEdit_Lua::SetEditable(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    bool editable = CHECK_BOOLEAN(L, 2);

    lineEdit->SetEditable(editable);

    return 0;
}

int LineEdit_Lua::IsEditable(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    bool ret = lineEdit->IsEditable();

    lua_pushboolean(L, ret);
    return 1;
}

int LineEdit_Lua::GetTitleWidget(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    Text* ret = lineEdit->GetTitleWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::GetInputField(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    InputField* ret = lineEdit->GetInputField();

    Node_Lua::Create(L, ret);
    return 1;
}

void LineEdit_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        LINEEDIT_LUA_NAME,
        LINEEDIT_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Title
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleWidth);

    // InputField passthrough
    REGISTER_TABLE_FUNC(L, mtIndex, SetText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetText);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPlaceholder);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlaceholder);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCaretPosition);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCaretPosition);
    REGISTER_TABLE_FUNC(L, mtIndex, SelectAll);
    REGISTER_TABLE_FUNC(L, mtIndex, ClearSelection);
    REGISTER_TABLE_FUNC(L, mtIndex, HasSelection);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedText);
    REGISTER_TABLE_FUNC(L, mtIndex, DeleteSelection);
    REGISTER_TABLE_FUNC(L, mtIndex, Select);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectionStart);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectionEnd);
    REGISTER_TABLE_FUNC(L, mtIndex, SetFocused);
    REGISTER_TABLE_FUNC(L, mtIndex, IsFocused);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPasswordMode);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPasswordMode);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxLength);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxLength);
    REGISTER_TABLE_FUNC(L, mtIndex, SetEditable);
    REGISTER_TABLE_FUNC(L, mtIndex, IsEditable);

    // Children
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, GetInputField);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
