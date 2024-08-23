#pragma once

#include "Input/InputConstants.h"

#include <vector>
#include <stdint.h>

#if PLATFORM_WINDOWS
#include <windows.h>
#include <Xinput.h>
#elif PLATFORM_DOLPHIN
#include <gccore.h>
#elif PLATFORM_3DS
#include <3ds.h>
#endif

enum class GamepadType
{
    Standard,
    GameCube,
    Wiimote,
    WiiClassic,

    Count
};

struct GamepadState
{
    int32_t mDevice = -1;
    int32_t mButtons[INPUT_MAX_GAMEPAD_BUTTONS] = { };
    float mAxes[INPUT_MAX_GAMEPAD_AXES] = { };
    GamepadType mType = GamepadType::Standard;
    bool mConnected = false;
};

struct InputState
{
    bool mKeys[INPUT_MAX_KEYS] = { };
    bool mMouseButtons[INPUT_MAX_MOUSE_BUTTONS] = { };
    bool mTouches[INPUT_MAX_TOUCHES] = { };
    GamepadState mGamepads[INPUT_MAX_GAMEPADS];

    bool mPrevKeys[INPUT_MAX_KEYS] = { };
    bool mPrevMouseButtons[INPUT_MAX_MOUSE_BUTTONS] = { };
    bool mPrevTouches[INPUT_MAX_TOUCHES] = { };
    GamepadState mPrevGamepads[INPUT_MAX_GAMEPADS];

    bool mRepeatKeys[INPUT_MAX_KEYS] = { };
    int32_t mScrollWheelDelta = 0;
    int32_t mMouseDeltaX = 0;
    int32_t mMouseDeltaY = 0;

    int32_t mPointerX[INPUT_MAX_TOUCHES] = { 0 };
    int32_t mPointerY[INPUT_MAX_TOUCHES] = { 0 };
    int32_t mNumControllers = 0;

    std::vector<int32_t> mJustDownKeys;

    bool mCursorLocked = false;
    bool mCursorTrapped = false;
    bool mCursorShown = true;

#if PLATFORM_WINDOWS
    XINPUT_STATE mXinputStates[INPUT_MAX_GAMEPADS] = { };
    XINPUT_STATE mXinputPrevStates[INPUT_MAX_GAMEPADS] = { };
    bool mActiveControllers[INPUT_MAX_GAMEPADS] = { };
#endif
};

enum MouseCode
{
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MOUSE_X1,
    MOUSE_X2
};

enum GamepadButtonCode
{
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_C,
    GAMEPAD_X,
    GAMEPAD_Y,
    GAMEPAD_Z,
    GAMEPAD_L1,
    GAMEPAD_R1,
    GAMEPAD_L2,
    GAMEPAD_R2,
    GAMEPAD_THUMBL,
    GAMEPAD_THUMBR,
    GAMEPAD_START,
    GAMEPAD_SELECT,
    GAMEPAD_LEFT,
    GAMEPAD_RIGHT,
    GAMEPAD_UP,
    GAMEPAD_DOWN,
    GAMEPAD_HOME
};

enum GamepadAxisCode
{
    GAMEPAD_AXIS_LTRIGGER,
    GAMEPAD_AXIS_RTRIGGER,
    GAMEPAD_AXIS_LTHUMB_X,
    GAMEPAD_AXIS_LTHUMB_Y,
    GAMEPAD_AXIS_RTHUMB_X,
    GAMEPAD_AXIS_RTHUMB_Y
};

#if PLATFORM_WINDOWS

enum KeyCode
{
    KEY_BACK = 10,
    KEY_ESCAPE = 27,

    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,

    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,

    KEY_SPACE = 32,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,

    KEY_SHIFT_L = 16,
    KEY_CONTROL_L = 17,
    KEY_ALT_L = 18,
    KEY_SHIFT_R = 16,
    KEY_CONTROL_R = 17,
    KEY_ALT_R = 18,

    KEY_INSERT = 0x2D,
    KEY_DELETE = 0x2E,
    KEY_HOME = 0x24,
    KEY_END = 0x23,
    KEY_PAGE_UP = 33,
    KEY_PAGE_DOWN = 34,

    KEY_UP = 38,
    KEY_DOWN = 40,
    KEY_LEFT = 37,
    KEY_RIGHT = 39,

    KEY_NUMPAD0 = 96,
    KEY_NUMPAD1 = 97,
    KEY_NUMPAD2 = 98,
    KEY_NUMPAD3 = 99,
    KEY_NUMPAD4 = 100,
    KEY_NUMPAD5 = 101,
    KEY_NUMPAD6 = 102,
    KEY_NUMPAD7 = 103,
    KEY_NUMPAD8 = 104,
    KEY_NUMPAD9 = 105,

    KEY_F1 = 112,
    KEY_F2 = 113,
    KEY_F3 = 114,
    KEY_F4 = 115,
    KEY_F5 = 116,
    KEY_F6 = 117,
    KEY_F7 = 118,
    KEY_F8 = 119,
    KEY_F9 = 120,
    KEY_F10 = 121,
    KEY_F11 = 122,
    KEY_F12 = 123,

    KEY_PERIOD = 0xBE,
    KEY_COMMA = 0xBC,
    KEY_PLUS = 0xBB,
    KEY_MINUS = 0xBD,

    KEY_COLON = 0xBA,
    KEY_QUESTION = 0xBF,
    KEY_SQUIGGLE = 0xC0,
    KEY_LEFT_BRACKET = 0xDB,
    KEY_BACK_SLASH = 0xDC,
    KEY_RIGHT_BRACKET = 0xDD,
    KEY_QUOTE = 0xDE,

    KEY_DECIMAL = 0x6E
};

#elif PLATFORM_ANDROID

enum KeyCode
{
    KEY_BACK = 4,
    KEY_ESCAPE = 255,

    KEY_0 = 7,
    KEY_1 = 8,
    KEY_2 = 9,
    KEY_3 = 10,
    KEY_4 = 11,
    KEY_5 = 12,
    KEY_6 = 13,
    KEY_7 = 14,
    KEY_8 = 15,
    KEY_9 = 16,

    KEY_A = 29,
    KEY_B = 30,
    KEY_C = 31,
    KEY_D = 32,
    KEY_E = 33,
    KEY_F = 34,
    KEY_G = 35,
    KEY_H = 36,
    KEY_I = 37,
    KEY_J = 38,
    KEY_K = 39,
    KEY_L = 40,
    KEY_M = 41,
    KEY_N = 42,
    KEY_O = 43,
    KEY_P = 44,
    KEY_Q = 45,
    KEY_R = 46,
    KEY_S = 47,
    KEY_T = 48,
    KEY_U = 49,
    KEY_V = 50,
    KEY_W = 51,
    KEY_X = 52,
    KEY_Y = 53,
    KEY_Z = 54,

    KEY_SPACE = 62,
    KEY_ENTER = 66,
    KEY_BACKSPACE = 67,
    KEY_TAB = 61,

    KEY_SHIFT_L = 60,
    KEY_CONTROL_L = 113,
    KEY_ALT_L = 255,
    KEY_SHIFT_R = 60,
    KEY_CONTROL_R = 113,
    KEY_ALT_R = 255,

    KEY_UP = 19,
    KEY_DOWN = 20,
    KEY_LEFT = 21,
    KEY_RIGHT = 22,

    KEY_NUMPAD0 = 144,
    KEY_NUMPAD1 = 145,
    KEY_NUMPAD2 = 146,
    KEY_NUMPAD3 = 147,
    KEY_NUMPAD4 = 148,
    KEY_NUMPAD5 = 149,
    KEY_NUMPAD6 = 150,
    KEY_NUMPAD7 = 151,
    KEY_NUMPAD8 = 152,
    KEY_NUMPAD9 = 153,

    KEY_F1 = 131,
    KEY_F2 = 132,
    KEY_F3 = 133,
    KEY_F4 = 134,
    KEY_F5 = 135,
    KEY_F6 = 136,
    KEY_F7 = 137,
    KEY_F8 = 138,
    KEY_F9 = 139,
    KEY_F10 = 140,
    KEY_F11 = 141,
    KEY_F12 = 142,

    KEY_PERIOD = 56,
    KEY_COMMA = 55,
    KEY_PLUS = 70,
    KEY_MINUS = 69,
    KEY_COLON = 74,
    KEY_QUESTION = 76,
    KEY_SQUIGGLE = 216, // Couldnt find keycode
    KEY_LEFT_BRACKET = 71,
    KEY_BACK_SLASH = 73,
    KEY_RIGHT_BRACKET = 72,
    KEY_QUOTE = 218, // Couldnt find keycode

    KEY_INSERT = 255,
    KEY_DELETE = 67,
    KEY_HOME = 255,
    KEY_END = 255,
    KEY_PAGE_UP = 255,
    KEY_PAGE_DOWN = 255,

    KEY_DECIMAL = 255 //???
};

// TODO: Use these button values in INP_Update()

/*
TODO: ANDROID-PLAT - These enum names conflict with the GamepadButtonCode enum above.
enum GamepadCode
{
    GAMEPAD_A = 96,
    GAMEPAD_B = 97,
    GAMEPAD_C = 98,
    GAMEPAD_X = 99,
    GAMEPAD_Y = 100,
    GAMEPAD_Z = 101,
    GAMEPAD_L1 = 102,
    GAMEPAD_R1 = 103,
    GAMEPAD_L2 = 104,
    GAMEPAD_R2 = 105,
    GAMEPAD_THUMBL = 106,
    GAMEPAD_THUMBR = 107,
    GAMEPAD_START = 108,
    GAMEPAD_SELECT = 109,
    GAMEPAD_LEFT = 0,
    GAMEPAD_RIGHT = 0,
    GAMEPAD_UP = 0,
    GAMEPAD_DOWN = 0,

    GAMEPAD_AXIS_X = 0,
    GAMEPAD_AXIS_Y = 1,
    GAMEPAD_AXIS_Z = 11,
    GAMEPAD_AXIS_RZ = 14,
    GAMEPAD_AXIS_HAT_X = 15,
    GAMEPAD_AXIS_HAT_Y = 16,
    GAMEPAD_AXIS_LTRIGGER = 17,
    GAMEPAD_AXIS_RTRIGGER = 18
};
*/

#elif PLATFORM_3DS

// 3DS has conflicts with KEY_A, KEY_B, KEY_L, KEY_DOWN, etc
// because it uses those names for the gamepad buttons.
// For now this version of the enum omits those

enum KeyCode
{
    KEY_BACK = 10,
    KEY_ESCAPE = 255,

    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,

    //KEY_A = 65,
    //KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    //KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    //KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    //KEY_X = 88,
    //KEY_Y = 89,
    KEY_Z = 90,

    KEY_SPACE = 32,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,

    KEY_SHIFT_L = 16,
    KEY_CONTROL_L = 17,
    KEY_ALT_L = 255,
    KEY_SHIFT_R = 16,
    KEY_CONTROL_R = 17,
    KEY_ALT_R = 255,

    KEY_INSERT = 255,
    KEY_DELETE = 255,
    KEY_HOME = 255,
    KEY_END = 255,
    KEY_PAGE_UP = 33,
    KEY_PAGE_DOWN = 34,

    //KEY_UP = 38,
    //KEY_DOWN = 40,
    //KEY_LEFT = 37,
    //KEY_RIGHT = 39,

    KEY_NUMPAD0 = 96,
    KEY_NUMPAD1 = 97,
    KEY_NUMPAD2 = 98,
    KEY_NUMPAD3 = 99,
    KEY_NUMPAD4 = 100,
    KEY_NUMPAD5 = 101,
    KEY_NUMPAD6 = 102,
    KEY_NUMPAD7 = 103,
    KEY_NUMPAD8 = 104,
    KEY_NUMPAD9 = 105,

    KEY_F1 = 112,
    KEY_F2 = 113,
    KEY_F3 = 114,
    KEY_F4 = 115,
    KEY_F5 = 116,
    KEY_F6 = 117,
    KEY_F7 = 118,
    KEY_F8 = 119,
    KEY_F9 = 120,
    KEY_F10 = 121,
    KEY_F11 = 122,
    KEY_F12 = 123,

    KEY_PERIOD = 0xBE,
    KEY_COMMA = 0xBC,
    KEY_PLUS = 0xBB,
    KEY_MINUS = 0xBD,

    KEY_COLON = 0xBA,
    KEY_QUESTION = 0xBF,
    KEY_SQUIGGLE = 0xC0,
    KEY_LEFT_BRACKET = 0xDB,
    KEY_BACK_SLASH = 0xDC,
    KEY_RIGHT_BRACKET = 0xDD,
    KEY_QUOTE = 0xDE,

    KEY_DECIMAL = 0x6E
};

#elif PLATFORM_LINUX

enum KeyCode
{
    KEY_BACK = 22,
    KEY_ESCAPE = 9,

    KEY_0 = 19,
    KEY_1 = 10,
    KEY_2 = 11,
    KEY_3 = 12,
    KEY_4 = 13,
    KEY_5 = 14,
    KEY_6 = 15,
    KEY_7 = 16,
    KEY_8 = 17,
    KEY_9 = 18,

    KEY_A = 38,
    KEY_B = 56,
    KEY_C = 54,
    KEY_D = 40,
    KEY_E = 26,
    KEY_F = 41,
    KEY_G = 42,
    KEY_H = 43,
    KEY_I = 31,
    KEY_J = 44,
    KEY_K = 45,
    KEY_L = 46,
    KEY_M = 58,
    KEY_N = 57,
    KEY_O = 32,
    KEY_P = 33,
    KEY_Q = 24,
    KEY_R = 27,
    KEY_S = 39,
    KEY_T = 28,
    KEY_U = 30,
    KEY_V = 55,
    KEY_W = 25,
    KEY_X = 53,
    KEY_Y = 29,
    KEY_Z = 52,

    KEY_SPACE = 65,
    KEY_ENTER = 36,
    KEY_BACKSPACE = 22,
    KEY_TAB = 23,

    KEY_SHIFT_L = 50,
    KEY_CONTROL_L = 37,
    KEY_ALT_L = 64,
    KEY_SHIFT_R = 62,
    KEY_CONTROL_R = 105,
    KEY_ALT_R = 113,

    KEY_INSERT = 106,
    KEY_DELETE = 119,
    KEY_HOME = 97,
    KEY_END = 103,
    KEY_PAGE_UP = 112,
    KEY_PAGE_DOWN = 117,

    KEY_UP = 111,
    KEY_DOWN = 116,
    KEY_LEFT = 113,
    KEY_RIGHT = 114,

    KEY_NUMPAD0 = 90,
    KEY_NUMPAD1 = 87,
    KEY_NUMPAD2 = 88,
    KEY_NUMPAD3 = 89,
    KEY_NUMPAD4 = 83,
    KEY_NUMPAD5 = 84,
    KEY_NUMPAD6 = 85,
    KEY_NUMPAD7 = 79,
    KEY_NUMPAD8 = 80,
    KEY_NUMPAD9 = 81,

    KEY_F1 = 67,
    KEY_F2 = 68,
    KEY_F3 = 69,
    KEY_F4 = 70,
    KEY_F5 = 71,
    KEY_F6 = 72,
    KEY_F7 = 73,
    KEY_F8 = 74,
    KEY_F9 = 75,
    KEY_F10 = 76,
    KEY_F11 = 95,
    KEY_F12 = 96,

    KEY_PERIOD = 60,
    KEY_COMMA = 59,
    KEY_PLUS = 21,
    KEY_MINUS = 20,

    KEY_COLON = 47,
    KEY_QUESTION = 61,
    KEY_SQUIGGLE = 49,
    KEY_LEFT_BRACKET = 34,
    KEY_BACK_SLASH = 51,
    KEY_RIGHT_BRACKET = 35,
    KEY_QUOTE = 48,

    KEY_DECIMAL = 91
};

#else // All other platforms

enum KeyCode
{
    KEY_BACK = 10,
    KEY_ESCAPE = 27,

    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,

    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,

    KEY_SPACE = 32,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,

    KEY_SHIFT_L = 16,
    KEY_CONTROL_L = 17,
    KEY_ALT_L = 255,
    KEY_SHIFT_R = 16,
    KEY_CONTROL_R = 17,
    KEY_ALT_R = 255,

    KEY_INSERT = 255,
    KEY_DELETE = 255,
    KEY_HOME = 255,
    KEY_END = 255,
    KEY_PAGE_UP = 33,
    KEY_PAGE_DOWN = 34,

    KEY_UP = 38,
    KEY_DOWN = 40,
    KEY_LEFT = 37,
    KEY_RIGHT = 39,

    KEY_NUMPAD0 = 96,
    KEY_NUMPAD1 = 97,
    KEY_NUMPAD2 = 98,
    KEY_NUMPAD3 = 99,
    KEY_NUMPAD4 = 100,
    KEY_NUMPAD5 = 101,
    KEY_NUMPAD6 = 102,
    KEY_NUMPAD7 = 103,
    KEY_NUMPAD8 = 104,
    KEY_NUMPAD9 = 105,

    KEY_F1 = 112,
    KEY_F2 = 113,
    KEY_F3 = 114,
    KEY_F4 = 115,
    KEY_F5 = 116,
    KEY_F6 = 117,
    KEY_F7 = 118,
    KEY_F8 = 119,
    KEY_F9 = 120,
    KEY_F10 = 121,
    KEY_F11 = 122,
    KEY_F12 = 123,

    KEY_PERIOD = 0xBE,
    KEY_COMMA = 0xBC,
    KEY_PLUS = 0xBB,
    KEY_MINUS = 0xBD,

    KEY_COLON = 0xBA,
    KEY_QUESTION = 0xBF,
    KEY_SQUIGGLE = 0xC0,
    KEY_LEFT_BRACKET = 0xDB,
    KEY_BACK_SLASH = 0xDC,
    KEY_RIGHT_BRACKET = 0xDD,
    KEY_QUOTE = 0xDE,

    KEY_DECIMAL = 0x6E
};

#endif