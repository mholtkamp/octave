#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/LineEdit.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define LINEEDIT_LUA_NAME "LineEdit"
#define LINEEDIT_LUA_FLAG "cfLineEdit"
#define CHECK_LINEEDIT(L, arg) (LineEdit*)CheckNodeLuaType(L, arg, LINEEDIT_LUA_NAME, LINEEDIT_LUA_FLAG);

struct LineEdit_Lua
{
    // Title
    static int SetTitle(lua_State* L);
    static int GetTitle(lua_State* L);
    static int SetTitleWidth(lua_State* L);
    static int GetTitleWidth(lua_State* L);

    // InputField passthrough
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

    // Children
    static int GetTitleWidget(lua_State* L);
    static int GetInputField(lua_State* L);

    static void Bind();
};

#endif
