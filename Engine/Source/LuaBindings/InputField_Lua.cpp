#include "LuaBindings/InputField_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"

#if LUA_ENABLED

int InputField_Lua::SetText(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    const char* text = CHECK_STRING(L, 2);

    inputField->SetText(text);

    return 0;
}

int InputField_Lua::GetText(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    const std::string& ret = inputField->GetText();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int InputField_Lua::SetPlaceholder(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    const char* placeholder = CHECK_STRING(L, 2);

    inputField->SetPlaceholder(placeholder);

    return 0;
}

int InputField_Lua::GetPlaceholder(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    const std::string& ret = inputField->GetPlaceholder();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int InputField_Lua::SetCaretPosition(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    int32_t pos = CHECK_INTEGER(L, 2);

    inputField->SetCaretPosition(pos);

    return 0;
}

int InputField_Lua::GetCaretPosition(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    int32_t ret = inputField->GetCaretPosition();

    lua_pushinteger(L, ret);
    return 1;
}

int InputField_Lua::SelectAll(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    inputField->SelectAll();

    return 0;
}

int InputField_Lua::ClearSelection(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    inputField->ClearSelection();

    return 0;
}

int InputField_Lua::HasSelection(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    bool ret = inputField->HasSelection();

    lua_pushboolean(L, ret);
    return 1;
}

int InputField_Lua::GetSelectedText(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    std::string ret = inputField->GetSelectedText();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int InputField_Lua::DeleteSelection(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    inputField->DeleteSelection();

    return 0;
}

int InputField_Lua::Select(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    int32_t start = CHECK_INTEGER(L, 2);
    int32_t end = CHECK_INTEGER(L, 3);

    inputField->Select(start, end);

    return 0;
}

int InputField_Lua::GetSelectionStart(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    int32_t ret = inputField->GetSelectionStart();

    lua_pushinteger(L, ret);
    return 1;
}

int InputField_Lua::GetSelectionEnd(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    int32_t ret = inputField->GetSelectionEnd();

    lua_pushinteger(L, ret);
    return 1;
}

int InputField_Lua::SetFocused(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    bool focused = CHECK_BOOLEAN(L, 2);

    inputField->SetFocused(focused);

    return 0;
}

int InputField_Lua::IsFocused(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    bool ret = inputField->IsFocused();

    lua_pushboolean(L, ret);
    return 1;
}

int InputField_Lua::SetPasswordMode(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    bool enabled = CHECK_BOOLEAN(L, 2);

    inputField->SetPasswordMode(enabled);

    return 0;
}

int InputField_Lua::IsPasswordMode(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    bool ret = inputField->IsPasswordMode();

    lua_pushboolean(L, ret);
    return 1;
}

int InputField_Lua::SetMaxLength(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    int32_t maxLen = CHECK_INTEGER(L, 2);

    inputField->SetMaxLength(maxLen);

    return 0;
}

int InputField_Lua::GetMaxLength(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    int32_t ret = inputField->GetMaxLength();

    lua_pushinteger(L, ret);
    return 1;
}

int InputField_Lua::SetEditable(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    bool editable = CHECK_BOOLEAN(L, 2);

    inputField->SetEditable(editable);

    return 0;
}

int InputField_Lua::IsEditable(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    bool ret = inputField->IsEditable();

    lua_pushboolean(L, ret);
    return 1;
}

int InputField_Lua::SetBackgroundColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetBackgroundColor(color);

    return 0;
}

int InputField_Lua::GetBackgroundColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetFocusedBackgroundColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetFocusedBackgroundColor(color);

    return 0;
}

int InputField_Lua::GetFocusedBackgroundColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetFocusedBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetTextColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetTextColor(color);

    return 0;
}

int InputField_Lua::GetTextColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetPlaceholderColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetPlaceholderColor(color);

    return 0;
}

int InputField_Lua::GetPlaceholderColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetPlaceholderColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetCaretColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetCaretColor(color);

    return 0;
}

int InputField_Lua::GetCaretColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetCaretColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetSelectionColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    inputField->SetSelectionColor(color);

    return 0;
}

int InputField_Lua::GetSelectionColor(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    glm::vec4 ret = inputField->GetSelectionColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::SetTextPadding(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);
    float padding = CHECK_NUMBER(L, 2);

    inputField->SetTextPadding(padding);

    return 0;
}

int InputField_Lua::GetTextPadding(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    float ret = inputField->GetTextPadding();

    lua_pushnumber(L, ret);
    return 1;
}

int InputField_Lua::GetBackground(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    Quad* ret = inputField->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::GetTextWidget(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    Text* ret = inputField->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int InputField_Lua::GetCaret(lua_State* L)
{
    InputField* inputField = CHECK_INPUTFIELD(L, 1);

    Quad* ret = inputField->GetCaret();

    Node_Lua::Create(L, ret);
    return 1;
}

void InputField_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        INPUTFIELD_LUA_NAME,
        INPUTFIELD_LUA_FLAG,
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
