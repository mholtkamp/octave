#include "LuaBindings/LineEdit_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"

#if LUA_ENABLED

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

int LineEdit_Lua::SetBackgroundColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetBackgroundColor(color);

    return 0;
}

int LineEdit_Lua::GetBackgroundColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetFocusedBackgroundColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetFocusedBackgroundColor(color);

    return 0;
}

int LineEdit_Lua::GetFocusedBackgroundColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetFocusedBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetTextColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetTextColor(color);

    return 0;
}

int LineEdit_Lua::GetTextColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetPlaceholderColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetPlaceholderColor(color);

    return 0;
}

int LineEdit_Lua::GetPlaceholderColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetPlaceholderColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetCaretColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetCaretColor(color);

    return 0;
}

int LineEdit_Lua::GetCaretColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetCaretColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetSelectionColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    lineEdit->SetSelectionColor(color);

    return 0;
}

int LineEdit_Lua::GetSelectionColor(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    glm::vec4 ret = lineEdit->GetSelectionColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::SetTextPadding(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);
    float padding = CHECK_NUMBER(L, 2);

    lineEdit->SetTextPadding(padding);

    return 0;
}

int LineEdit_Lua::GetTextPadding(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    float ret = lineEdit->GetTextPadding();

    lua_pushnumber(L, ret);
    return 1;
}

int LineEdit_Lua::GetBackground(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    Quad* ret = lineEdit->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::GetTextWidget(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    Text* ret = lineEdit->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int LineEdit_Lua::GetCaret(lua_State* L)
{
    LineEdit* lineEdit = CHECK_LINEEDIT(L, 1);

    Quad* ret = lineEdit->GetCaret();

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
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetFocusedBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetFocusedBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPlaceholderColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlaceholderColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCaretColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCaretColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectionColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectionColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextPadding);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextPadding);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackground);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCaret);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
