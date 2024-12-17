#include "LuaBindings/Input_Lua.h"

#include "InputDevices.h"
#include "Input/Input.h"

#if LUA_ENABLED

int Input_Lua::IsKeyDown(lua_State* L)
{
    KeyCode key = CHECK_KEY(L, 1);

    bool down = ::IsKeyDown(key);

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsKeyJustDownRepeat(lua_State* L)
{
    KeyCode key = CHECK_KEY(L, 1);

    bool down = ::IsKeyJustDownRepeat(key);

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsKeyJustDown(lua_State* L)
{
    KeyCode key = CHECK_KEY(L, 1);

    bool down = ::IsKeyJustDown(key);

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsKeyJustUp(lua_State* L)
{
    KeyCode key = CHECK_KEY(L, 1);

    bool up = ::IsKeyJustUp(key);

    lua_pushboolean(L, up);
    return 1;
}

int Input_Lua::IsControlDown(lua_State* L)
{
    bool down = ::IsControlDown();

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsShiftDown(lua_State* L)
{
    bool down = ::IsShiftDown();

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsMouseButtonDown(lua_State* L)
{
    MouseCode mouse = CHECK_MOUSE(L, 1);

    bool down = ::IsMouseButtonDown(mouse);

    lua_pushboolean(L, down);
    return 1;
}

int Input_Lua::IsMouseButtonJustDown(lua_State* L)
{
    MouseCode mouse = CHECK_MOUSE(L, 1);

    bool ret = ::IsMouseButtonJustDown(mouse);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsMouseButtonJustUp(lua_State* L)
{
    MouseCode mouse = CHECK_MOUSE(L, 1);

    bool ret = ::IsMouseButtonJustUp(mouse);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::GetMousePosition(lua_State* L)
{
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    ::GetMousePosition(mouseX, mouseY);

    lua_pushinteger(L, mouseX);
    lua_pushinteger(L, mouseY);
    return 2;
}

int Input_Lua::GetScrollWheelDelta(lua_State* L)
{
    int32_t delta = ::GetScrollWheelDelta();

    lua_pushinteger(L, delta);
    return 1;
}

int Input_Lua::GetMouseDelta(lua_State* L)
{
    int32_t deltaX = 0;
    int32_t deltaY = 0;

    INP_GetMouseDelta(deltaX, deltaY);

    lua_pushinteger(L, deltaX);
    lua_pushinteger(L, deltaY);
    return 2;
}

int Input_Lua::IsTouchDown(lua_State* L)
{
    int index = CHECK_INTEGER(L, 1);
    --index;

    bool ret = ::IsTouchDown(index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsPointerJustUp(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    bool ret = ::IsPointerJustUp(index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsPointerJustDown(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    bool ret = ::IsPointerJustDown(index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsPointerDown(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    bool ret = ::IsPointerDown(index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::GetTouchPosition(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    int32_t touchX = 0;
    int32_t touchY = 0;
    ::GetTouchPosition(touchX, touchY, index);

    lua_pushinteger(L, touchX);
    lua_pushinteger(L, touchY);
    return 2;
}

int Input_Lua::GetTouchPositionNormalized(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    float touchX = 0;
    float touchY = 0;
    ::GetTouchPositionNormalized(touchX, touchY, index);

    lua_pushnumber(L, touchX);
    lua_pushnumber(L, touchY);
    return 2;
}

int Input_Lua::GetPointerPosition(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    int32_t x = 0;
    int32_t y = 0;
    ::GetPointerPosition(x, y, index);

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

int Input_Lua::GetPointerPositionNormalized(lua_State* L)
{
    int index = 1;
    if (!lua_isnone(L, 1)) { index = CHECK_INTEGER(L, 1); }
    --index;

    float x = 0;
    float y = 0;
    ::GetPointerPositionNormalized(x, y, index);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

int Input_Lua::IsGamepadButtonDown(lua_State* L)
{
    GamepadButtonCode button = CHECK_GAMEPAD(L, 1);
    int index = lua_isinteger(L, 2) ? lua_tointeger(L, 2) - 1 : 0;

    bool ret = ::IsGamepadButtonDown(button, index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsGamepadButtonJustDown(lua_State* L)
{
    GamepadButtonCode button = CHECK_GAMEPAD(L, 1);
    int index = lua_isinteger(L, 2) ? lua_tointeger(L, 2) - 1 : 0;

    bool ret = ::IsGamepadButtonJustDown(button, index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::IsGamepadButtonJustUp(lua_State* L)
{
    GamepadButtonCode button = CHECK_GAMEPAD(L, 1);
    int index = lua_isinteger(L, 2) ? lua_tointeger(L, 2) - 1 : 0;

    bool ret = ::IsGamepadButtonJustUp(button, index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::GetGamepadAxisValue(lua_State* L)
{
    GamepadAxisCode axis = CHECK_GAMEPAD_AXIS(L, 1);
    int index = lua_isinteger(L, 2) ? lua_tointeger(L, 2) - 1 : 0;

    float ret = ::GetGamepadAxisValue(axis, index);

    lua_pushnumber(L, ret);
    return 1;
}

int Input_Lua::GetGamepadType(lua_State* L)
{
    int index = lua_isinteger(L, 1) ? lua_tointeger(L, 1) - 1: 0;

    GamepadType padType = ::GetGamepadType(index);

    const char* ret = "";
    switch (padType)
    {
    case GamepadType::Standard:
        ret = "Standard";
        break;
    case GamepadType::GameCube:
        ret = "GameCube";
        break;
    case GamepadType::Wiimote:
        ret = "Wiimote";
        break;
    case GamepadType::WiiClassic:
        ret = "WiiClassic";
        break;

    default:
        ret = "Unknown";
        break;
    }

    lua_pushstring(L, ret);
    return 1;
}

int Input_Lua::IsGamepadConnected(lua_State* L)
{
    int index = lua_isinteger(L, 1) ? lua_tointeger(L, 1) - 1 : 0;

    bool ret = ::IsGamepadConnected(index);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::ShowCursor(lua_State* L)
{
    bool show = CHECK_BOOLEAN(L, 1);

    INP_ShowCursor(show);

    return 0;
}

int Input_Lua::SetCursorPosition(lua_State* L)
{
    int32_t x = CHECK_INTEGER(L, 1);
    int32_t y = CHECK_INTEGER(L, 2);

    INP_SetCursorPos(x, y);

    return 0;
}

int Input_Lua::GetKeysJustDown(lua_State* L)
{
     LuaPushDatum(L, GetEngineState()->mInput.mJustDownKeys);
     return 1;
}

int Input_Lua::IsAnyKeyJustDown(lua_State* L)
{
    bool ret = (GetEngineState()->mInput.mJustDownKeys.size() > 0);

    lua_pushboolean(L, ret);
    return 1;
}

int Input_Lua::ConvertKeyCodeToChar(lua_State* L)
{
    int32_t keyCode = CHECK_INTEGER(L, 1);

    // Make a string with a single char (and null terminator)
    char keyChar[2];
    keyChar[0] = INP_ConvertKeyCodeToChar(keyCode);
    keyChar[1] = 0;

    lua_pushstring(L, keyChar);
    return 1;
}

int Input_Lua::ShowSoftKeyboard(lua_State* L)
{
    bool show = CHECK_BOOLEAN(L, 1);

    INP_ShowSoftKeyboard(show);

    return 0;
}

int Input_Lua::IsSoftKeyboardShown(lua_State* L)
{
    bool ret = INP_IsSoftKeyboardShown();

    lua_pushboolean(L, ret);
    return 1;
}

void Input_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, IsKeyDown);

    REGISTER_TABLE_FUNC(L, tableIdx, IsKeyJustDownRepeat);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsKeyJustDownRepeat, "IsKeyPressedRepeat");

    REGISTER_TABLE_FUNC(L, tableIdx, IsKeyJustDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsKeyJustDown, "IsKeyPressed");

    REGISTER_TABLE_FUNC(L, tableIdx, IsKeyJustUp);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsKeyJustUp, "IsKeyReleased");

    REGISTER_TABLE_FUNC(L, tableIdx, IsControlDown);

    REGISTER_TABLE_FUNC(L, tableIdx, IsShiftDown);

    REGISTER_TABLE_FUNC(L, tableIdx, IsMouseButtonDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsMouseButtonDown, "IsMouseDown");

    REGISTER_TABLE_FUNC(L, tableIdx, IsMouseButtonJustDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsMouseButtonJustDown, "IsMousePressed");

    REGISTER_TABLE_FUNC(L, tableIdx, IsMouseButtonJustUp);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsMouseButtonJustUp, "IsMouseReleased");

    REGISTER_TABLE_FUNC(L, tableIdx, GetMousePosition);

    REGISTER_TABLE_FUNC(L, tableIdx, GetScrollWheelDelta);

    REGISTER_TABLE_FUNC(L, tableIdx, GetMouseDelta);

    REGISTER_TABLE_FUNC(L, tableIdx, IsTouchDown);

    REGISTER_TABLE_FUNC(L, tableIdx, IsPointerJustUp);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsPointerJustUp, "IsPointerReleased");

    REGISTER_TABLE_FUNC(L, tableIdx, IsPointerJustDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsPointerJustDown, "IsPointerPressed");

    REGISTER_TABLE_FUNC(L, tableIdx, IsPointerDown);

    REGISTER_TABLE_FUNC(L, tableIdx, GetTouchPosition);

    REGISTER_TABLE_FUNC(L, tableIdx, GetTouchPositionNormalized);

    REGISTER_TABLE_FUNC(L, tableIdx, GetPointerPosition);

    REGISTER_TABLE_FUNC(L, tableIdx, GetPointerPositionNormalized);

    REGISTER_TABLE_FUNC(L, tableIdx, IsGamepadButtonDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsGamepadButtonDown, "IsGamepadDown");

    REGISTER_TABLE_FUNC(L, tableIdx, IsGamepadButtonJustDown);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsGamepadButtonJustDown, "IsGamepadPressed");

    REGISTER_TABLE_FUNC(L, tableIdx, IsGamepadButtonJustUp);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, IsGamepadButtonJustUp, "IsGamepadReleased");

    REGISTER_TABLE_FUNC(L, tableIdx, GetGamepadAxisValue);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, GetGamepadAxisValue, "GetGamepadAxis");

    REGISTER_TABLE_FUNC(L, tableIdx, GetGamepadType);

    REGISTER_TABLE_FUNC(L, tableIdx, IsGamepadConnected);

    REGISTER_TABLE_FUNC(L, tableIdx, ShowCursor);

    REGISTER_TABLE_FUNC(L, tableIdx, SetCursorPosition);

    REGISTER_TABLE_FUNC(L, tableIdx, GetKeysJustDown);

    REGISTER_TABLE_FUNC(L, tableIdx, IsAnyKeyJustDown);

    REGISTER_TABLE_FUNC(L, tableIdx, ConvertKeyCodeToChar);

    REGISTER_TABLE_FUNC(L, tableIdx, ShowSoftKeyboard);

    REGISTER_TABLE_FUNC(L, tableIdx, IsSoftKeyboardShown);

    lua_setglobal(L, INPUT_LUA_NAME);
    OCT_ASSERT(lua_gettop(L) == 0);

    BindKeyTable();
    BindMouseTable();
    BindGamepadTable();
}

void Input_Lua::BindKeyTable()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, KeyCode::KEY_BACK);
    lua_setfield(L, tableIdx, "Back");
    lua_pushinteger(L, KeyCode::KEY_ESCAPE);
    lua_setfield(L, tableIdx, "Escape");

    lua_pushinteger(L, KeyCode::KEY_0);
    lua_setfield(L, tableIdx, "N0");
    lua_pushinteger(L, KeyCode::KEY_1);
    lua_setfield(L, tableIdx, "N1");
    lua_pushinteger(L, KeyCode::KEY_2);
    lua_setfield(L, tableIdx, "N2");
    lua_pushinteger(L, KeyCode::KEY_3);
    lua_setfield(L, tableIdx, "N3");
    lua_pushinteger(L, KeyCode::KEY_4);
    lua_setfield(L, tableIdx, "N4");
    lua_pushinteger(L, KeyCode::KEY_5);
    lua_setfield(L, tableIdx, "N5");
    lua_pushinteger(L, KeyCode::KEY_6);
    lua_setfield(L, tableIdx, "N6");
    lua_pushinteger(L, KeyCode::KEY_7);
    lua_setfield(L, tableIdx, "N7");
    lua_pushinteger(L, KeyCode::KEY_8);
    lua_setfield(L, tableIdx, "N8");
    lua_pushinteger(L, KeyCode::KEY_9);
    lua_setfield(L, tableIdx, "N9");

    // Alternate number key names
    lua_pushinteger(L, KeyCode::KEY_0);
    lua_setfield(L, tableIdx, "Zero");
    lua_pushinteger(L, KeyCode::KEY_1);
    lua_setfield(L, tableIdx, "One");
    lua_pushinteger(L, KeyCode::KEY_2);
    lua_setfield(L, tableIdx, "Two");
    lua_pushinteger(L, KeyCode::KEY_3);
    lua_setfield(L, tableIdx, "Three");
    lua_pushinteger(L, KeyCode::KEY_4);
    lua_setfield(L, tableIdx, "Four");
    lua_pushinteger(L, KeyCode::KEY_5);
    lua_setfield(L, tableIdx, "Five");
    lua_pushinteger(L, KeyCode::KEY_6);
    lua_setfield(L, tableIdx, "Six");
    lua_pushinteger(L, KeyCode::KEY_7);
    lua_setfield(L, tableIdx, "Seven");
    lua_pushinteger(L, KeyCode::KEY_8);
    lua_setfield(L, tableIdx, "Eight");
    lua_pushinteger(L, KeyCode::KEY_9);
    lua_setfield(L, tableIdx, "Nine");

#if PLATFORM_3DS
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "A");
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "B");
#else
    lua_pushinteger(L, KeyCode::KEY_A);
    lua_setfield(L, tableIdx, "A");
    lua_pushinteger(L, KeyCode::KEY_B);
    lua_setfield(L, tableIdx, "B");
#endif
    lua_pushinteger(L, KeyCode::KEY_C);
    lua_setfield(L, tableIdx, "C");
    lua_pushinteger(L, KeyCode::KEY_D);
    lua_setfield(L, tableIdx, "D");
    lua_pushinteger(L, KeyCode::KEY_E);
    lua_setfield(L, tableIdx, "E");
    lua_pushinteger(L, KeyCode::KEY_F);
    lua_setfield(L, tableIdx, "F");
    lua_pushinteger(L, KeyCode::KEY_G);
    lua_setfield(L, tableIdx, "G");
    lua_pushinteger(L, KeyCode::KEY_H);
    lua_setfield(L, tableIdx, "H");
    lua_pushinteger(L, KeyCode::KEY_I);
    lua_setfield(L, tableIdx, "I");
    lua_pushinteger(L, KeyCode::KEY_J);
    lua_setfield(L, tableIdx, "J");
    lua_pushinteger(L, KeyCode::KEY_K);
    lua_setfield(L, tableIdx, "K");
#if PLATFORM_3DS
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "L");
#else
    lua_pushinteger(L, KeyCode::KEY_L);
    lua_setfield(L, tableIdx, "L");
#endif
    lua_pushinteger(L, KeyCode::KEY_M);
    lua_setfield(L, tableIdx, "M");
    lua_pushinteger(L, KeyCode::KEY_N);
    lua_setfield(L, tableIdx, "N");
    lua_pushinteger(L, KeyCode::KEY_O);
    lua_setfield(L, tableIdx, "O");
    lua_pushinteger(L, KeyCode::KEY_P);
    lua_setfield(L, tableIdx, "P");
    lua_pushinteger(L, KeyCode::KEY_Q);
    lua_setfield(L, tableIdx, "Q");
#if PLATFORM_3DS
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "R");
#else
    lua_pushinteger(L, KeyCode::KEY_R);
    lua_setfield(L, tableIdx, "R");
#endif
    lua_pushinteger(L, KeyCode::KEY_S);
    lua_setfield(L, tableIdx, "S");
    lua_pushinteger(L, KeyCode::KEY_T);
    lua_setfield(L, tableIdx, "T");
    lua_pushinteger(L, KeyCode::KEY_U);
    lua_setfield(L, tableIdx, "U");
    lua_pushinteger(L, KeyCode::KEY_V);
    lua_setfield(L, tableIdx, "V");
    lua_pushinteger(L, KeyCode::KEY_W);
    lua_setfield(L, tableIdx, "W");
#if PLATFORM_3DS
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "X");
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "Y");
#else
    lua_pushinteger(L, KeyCode::KEY_X);
    lua_setfield(L, tableIdx, "X");
    lua_pushinteger(L, KeyCode::KEY_Y);
    lua_setfield(L, tableIdx, "Y");
#endif
    lua_pushinteger(L, KeyCode::KEY_Z);
    lua_setfield(L, tableIdx, "Z");

    lua_pushinteger(L, KeyCode::KEY_SPACE);
    lua_setfield(L, tableIdx, "Space");
    lua_pushinteger(L, KeyCode::KEY_ENTER);
    lua_setfield(L, tableIdx, "Enter");
    lua_pushinteger(L, KeyCode::KEY_BACKSPACE);
    lua_setfield(L, tableIdx, "Backspace");
    lua_pushinteger(L, KeyCode::KEY_TAB);
    lua_setfield(L, tableIdx, "Tab");

    lua_pushinteger(L, KeyCode::KEY_SHIFT_L);
    lua_setfield(L, tableIdx, "ShiftL");
    lua_pushinteger(L, KeyCode::KEY_CONTROL_L);
    lua_setfield(L, tableIdx, "ControlL");
    lua_pushinteger(L, KeyCode::KEY_ALT_L);
    lua_setfield(L, tableIdx, "AltL");
    lua_pushinteger(L, KeyCode::KEY_SHIFT_R);
    lua_setfield(L, tableIdx, "ShiftR");
    lua_pushinteger(L, KeyCode::KEY_CONTROL_R);
    lua_setfield(L, tableIdx, "ControlR");
    lua_pushinteger(L, KeyCode::KEY_ALT_R);
    lua_setfield(L, tableIdx, "AltR");

    lua_pushinteger(L, KeyCode::KEY_INSERT);
    lua_setfield(L, tableIdx, "Insert");
    lua_pushinteger(L, KeyCode::KEY_DELETE);
    lua_setfield(L, tableIdx, "Delete");
    lua_pushinteger(L, KeyCode::KEY_HOME);
    lua_setfield(L, tableIdx, "Home");
    lua_pushinteger(L, KeyCode::KEY_END);
    lua_setfield(L, tableIdx, "End");
    lua_pushinteger(L, KeyCode::KEY_PAGE_UP);
    lua_setfield(L, tableIdx, "PageUp");
    lua_pushinteger(L, KeyCode::KEY_PAGE_DOWN);
    lua_setfield(L, tableIdx, "PageDown");

#if PLATFORM_3DS
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "Up");
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "Down");
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "Left");
    lua_pushinteger(L, 0);
    lua_setfield(L, tableIdx, "Right");
#else
    lua_pushinteger(L, KeyCode::KEY_UP);
    lua_setfield(L, tableIdx, "Up");
    lua_pushinteger(L, KeyCode::KEY_DOWN);
    lua_setfield(L, tableIdx, "Down");
    lua_pushinteger(L, KeyCode::KEY_LEFT);
    lua_setfield(L, tableIdx, "Left");
    lua_pushinteger(L, KeyCode::KEY_RIGHT);
    lua_setfield(L, tableIdx, "Right");
#endif

    lua_pushinteger(L, KeyCode::KEY_NUMPAD0);
    lua_setfield(L, tableIdx, "Numpad0");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD1);
    lua_setfield(L, tableIdx, "Numpad1");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD2);
    lua_setfield(L, tableIdx, "Numpad2");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD3);
    lua_setfield(L, tableIdx, "Numpad3");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD4);
    lua_setfield(L, tableIdx, "Numpad4");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD5);
    lua_setfield(L, tableIdx, "Numpad5");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD6);
    lua_setfield(L, tableIdx, "Numpad6");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD7);
    lua_setfield(L, tableIdx, "Numpad7");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD8);
    lua_setfield(L, tableIdx, "Numpad8");
    lua_pushinteger(L, KeyCode::KEY_NUMPAD9);
    lua_setfield(L, tableIdx, "Numpad9");

    lua_pushinteger(L, KeyCode::KEY_F1);
    lua_setfield(L, tableIdx, "F1");
    lua_pushinteger(L, KeyCode::KEY_F2);
    lua_setfield(L, tableIdx, "F2");
    lua_pushinteger(L, KeyCode::KEY_F3);
    lua_setfield(L, tableIdx, "F3");
    lua_pushinteger(L, KeyCode::KEY_F4);
    lua_setfield(L, tableIdx, "F4");
    lua_pushinteger(L, KeyCode::KEY_F5);
    lua_setfield(L, tableIdx, "F5");
    lua_pushinteger(L, KeyCode::KEY_F6);
    lua_setfield(L, tableIdx, "F6");
    lua_pushinteger(L, KeyCode::KEY_F7);
    lua_setfield(L, tableIdx, "F7");
    lua_pushinteger(L, KeyCode::KEY_F8);
    lua_setfield(L, tableIdx, "F8");
    lua_pushinteger(L, KeyCode::KEY_F9);
    lua_setfield(L, tableIdx, "F9");
    lua_pushinteger(L, KeyCode::KEY_F10);
    lua_setfield(L, tableIdx, "F10");
    lua_pushinteger(L, KeyCode::KEY_F11);
    lua_setfield(L, tableIdx, "F11");
    lua_pushinteger(L, KeyCode::KEY_F12);
    lua_setfield(L, tableIdx, "F12");

    lua_pushinteger(L, KeyCode::KEY_PERIOD);
    lua_setfield(L, tableIdx, "Period");
    lua_pushinteger(L, KeyCode::KEY_COMMA);
    lua_setfield(L, tableIdx, "Comma");
    lua_pushinteger(L, KeyCode::KEY_PLUS);
    lua_setfield(L, tableIdx, "Plus");
    lua_pushinteger(L, KeyCode::KEY_MINUS);
    lua_setfield(L, tableIdx, "Minus");

    lua_pushinteger(L, KeyCode::KEY_COLON);
    lua_setfield(L, tableIdx, "Colon");
    lua_pushinteger(L, KeyCode::KEY_QUESTION);
    lua_setfield(L, tableIdx, "Question");
    lua_pushinteger(L, KeyCode::KEY_SQUIGGLE);
    lua_setfield(L, tableIdx, "Squiggle");
    lua_pushinteger(L, KeyCode::KEY_LEFT_BRACKET);
    lua_setfield(L, tableIdx, "LeftBracket");
    lua_pushinteger(L, KeyCode::KEY_BACK_SLASH);
    lua_setfield(L, tableIdx, "BackSlash");
    lua_pushinteger(L, KeyCode::KEY_RIGHT_BRACKET);
    lua_setfield(L, tableIdx, "RightBracket");
    lua_pushinteger(L, KeyCode::KEY_QUOTE);
    lua_setfield(L, tableIdx, "Quote");

    lua_pushinteger(L, KeyCode::KEY_DECIMAL);
    lua_setfield(L, tableIdx, "Decimal");

    lua_setglobal(L, "Key");
    OCT_ASSERT(lua_gettop(L) == 0);
}

void Input_Lua::BindMouseTable()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, MouseCode::MOUSE_LEFT);
    lua_setfield(L, tableIdx, "1");
    lua_pushinteger(L, MouseCode::MOUSE_LEFT);
    lua_setfield(L, tableIdx, "Left");

    lua_pushinteger(L, MouseCode::MOUSE_RIGHT);
    lua_setfield(L, tableIdx, "2");
    lua_pushinteger(L, MouseCode::MOUSE_RIGHT);
    lua_setfield(L, tableIdx, "Right");

    lua_pushinteger(L, MouseCode::MOUSE_MIDDLE);
    lua_setfield(L, tableIdx, "3");
    lua_pushinteger(L, MouseCode::MOUSE_MIDDLE);
    lua_setfield(L, tableIdx, "Middle");

    lua_pushinteger(L, MouseCode::MOUSE_X1);
    lua_setfield(L, tableIdx, "4");
    lua_pushinteger(L, MouseCode::MOUSE_X1);
    lua_setfield(L, tableIdx, "X1");

    lua_pushinteger(L, MouseCode::MOUSE_X2);
    lua_setfield(L, tableIdx, "5");
    lua_pushinteger(L, MouseCode::MOUSE_X2);
    lua_setfield(L, tableIdx, "X2");

    lua_setglobal(L, "Mouse");
    OCT_ASSERT(lua_gettop(L) == 0);
}

void Input_Lua::BindGamepadTable()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    // Buttons
    lua_pushinteger(L, GAMEPAD_A);
    lua_setfield(L, tableIdx, "A");
    lua_pushinteger(L, GAMEPAD_B);
    lua_setfield(L, tableIdx, "B");
    lua_pushinteger(L, GAMEPAD_C);
    lua_setfield(L, tableIdx, "C");
    lua_pushinteger(L, GAMEPAD_X);
    lua_setfield(L, tableIdx, "X");
    lua_pushinteger(L, GAMEPAD_Y);
    lua_setfield(L, tableIdx, "Y");
    lua_pushinteger(L, GAMEPAD_Z);
    lua_setfield(L, tableIdx, "Z");
    lua_pushinteger(L, GAMEPAD_L1);
    lua_setfield(L, tableIdx, "L1");
    lua_pushinteger(L, GAMEPAD_R1);
    lua_setfield(L, tableIdx, "R1");
    lua_pushinteger(L, GAMEPAD_L2);
    lua_setfield(L, tableIdx, "L2");
    lua_pushinteger(L, GAMEPAD_R2);
    lua_setfield(L, tableIdx, "R2");
    lua_pushinteger(L, GAMEPAD_THUMBL);
    lua_setfield(L, tableIdx, "L3");
    lua_pushinteger(L, GAMEPAD_THUMBR);
    lua_setfield(L, tableIdx, "R3");
    lua_pushinteger(L, GAMEPAD_START);
    lua_setfield(L, tableIdx, "Start");
    lua_pushinteger(L, GAMEPAD_SELECT);
    lua_setfield(L, tableIdx, "Select");
    lua_pushinteger(L, GAMEPAD_LEFT);
    lua_setfield(L, tableIdx, "Left");
    lua_pushinteger(L, GAMEPAD_RIGHT);
    lua_setfield(L, tableIdx, "Right");
    lua_pushinteger(L, GAMEPAD_UP);
    lua_setfield(L, tableIdx, "Up");
    lua_pushinteger(L, GAMEPAD_DOWN);
    lua_setfield(L, tableIdx, "Down");
    lua_pushinteger(L, GAMEPAD_HOME);
    lua_setfield(L, tableIdx, "Home");

    // Axes
    lua_pushinteger(L, GAMEPAD_AXIS_LTRIGGER);
    lua_setfield(L, tableIdx, "AxisL");
    lua_pushinteger(L, GAMEPAD_AXIS_RTRIGGER);
    lua_setfield(L, tableIdx, "AxisR");
    lua_pushinteger(L, GAMEPAD_AXIS_LTHUMB_X);
    lua_setfield(L, tableIdx, "AxisLX");
    lua_pushinteger(L, GAMEPAD_AXIS_LTHUMB_Y);
    lua_setfield(L, tableIdx, "AxisLY");
    lua_pushinteger(L, GAMEPAD_AXIS_RTHUMB_X);
    lua_setfield(L, tableIdx, "AxisRX");
    lua_pushinteger(L, GAMEPAD_AXIS_RTHUMB_Y);
    lua_setfield(L, tableIdx, "AxisRY");

    lua_setglobal(L, "Gamepad");
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
