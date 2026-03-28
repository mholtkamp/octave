#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/InputField.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define INPUTFIELD_LUA_NAME "InputField"
#define INPUTFIELD_LUA_FLAG "cfInputField"
#define CHECK_INPUTFIELD(L, arg) (InputField*)CheckNodeLuaType(L, arg, INPUTFIELD_LUA_NAME, INPUTFIELD_LUA_FLAG);

struct InputField_Lua
{
    static int SetText(lua_State* L);
    static int GetText(lua_State* L);
    static int SetPlaceholder(lua_State* L);
    static int GetPlaceholder(lua_State* L);
    static int SetCaretPosition(lua_State* L);
    static int GetCaretPosition(lua_State* L);
    static int SelectAll(lua_State* L);
    static int ClearSelection(lua_State* L);
    static int HasSelection(lua_State* L);
    static int GetSelectedText(lua_State* L);
    static int DeleteSelection(lua_State* L);
    static int Select(lua_State* L);
    static int GetSelectionStart(lua_State* L);
    static int GetSelectionEnd(lua_State* L);
    static int SetFocused(lua_State* L);
    static int IsFocused(lua_State* L);
    static int SetPasswordMode(lua_State* L);
    static int IsPasswordMode(lua_State* L);
    static int SetMaxLength(lua_State* L);
    static int GetMaxLength(lua_State* L);
    static int SetEditable(lua_State* L);
    static int IsEditable(lua_State* L);
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetFocusedBackgroundColor(lua_State* L);
    static int GetFocusedBackgroundColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);
    static int SetPlaceholderColor(lua_State* L);
    static int GetPlaceholderColor(lua_State* L);
    static int SetCaretColor(lua_State* L);
    static int GetCaretColor(lua_State* L);
    static int SetSelectionColor(lua_State* L);
    static int GetSelectionColor(lua_State* L);
    static int SetTextPadding(lua_State* L);
    static int GetTextPadding(lua_State* L);
    static int GetBackground(lua_State* L);
    static int GetTextWidget(lua_State* L);
    static int GetCaret(lua_State* L);

    static void Bind();
};

#endif
