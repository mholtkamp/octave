#include "Input/Input.h"

#include "Engine.h"
#include "Log.h"

#include "Assertion.h"

// Platform Agnostic
void INP_SetKey(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    InputState& input = GetEngineState()->mInput;
    input.mKeys[key] = true;
    input.mRepeatKeys[key] = true;
    input.mJustDownKeys.push_back(key);
#endif
}

void INP_ClearKey(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    InputState& input = GetEngineState()->mInput;
    input.mKeys[key] = false;
#endif
}

void INP_ClearAllKeys()
{
#if INPUT_KEYBOARD_SUPPORT
    InputState& input = GetEngineState()->mInput;
    int32_t i = 0;

    // Do not clear hardware keys
    // I think this is for Android?? not sure...
    int32_t back = input.mKeys[KEY_BACK];

    for (i = 0; i < INPUT_MAX_KEYS; i++)
    {
        input.mKeys[i] = false;
    }

    // Restore hardware keys
    input.mKeys[KEY_BACK] = back;
#endif
}

bool INP_IsKeyDown(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    return GetEngineState()->mInput.mKeys[key];
#else
    return false;
#endif
}

bool INP_IsKeyJustDownRepeat(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    return GetEngineState()->mInput.mRepeatKeys[key];
#else
    return false;
#endif
}

bool INP_IsKeyJustDown(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    InputState& input = GetEngineState()->mInput;
    return input.mKeys[key] && !input.mPrevKeys[key];
#else
    return false;
#endif
}

bool INP_IsKeyJustUp(int32_t key)
{
#if INPUT_KEYBOARD_SUPPORT
    OCT_ASSERT(key >= 0 && key < INPUT_MAX_KEYS);
    InputState& input = GetEngineState()->mInput;
    return input.mPrevKeys[key] && !input.mKeys[key];
#else
    return false;
#endif
}

char INP_ConvertKeyCodeToChar(int32_t key)
{
    char retChar = 0;

    switch (key)
    {
    case KEY_0: retChar = '0'; break;
    case KEY_1: retChar = '1'; break;
    case KEY_2: retChar = '2'; break;
    case KEY_3: retChar = '3'; break;
    case KEY_4: retChar = '4'; break;
    case KEY_5: retChar = '5'; break;
    case KEY_6: retChar = '6'; break;
    case KEY_7: retChar = '7'; break;
    case KEY_8: retChar = '8'; break;
    case KEY_9: retChar = '9'; break;

    case KEY_A: retChar = 'A'; break;
    case KEY_B: retChar = 'B'; break;
    case KEY_C: retChar = 'C'; break;
    case KEY_D: retChar = 'D'; break;
    case KEY_E: retChar = 'E'; break;
    case KEY_F: retChar = 'F'; break;
    case KEY_G: retChar = 'G'; break;
    case KEY_H: retChar = 'H'; break;
    case KEY_I: retChar = 'I'; break;
    case KEY_J: retChar = 'J'; break;
    case KEY_K: retChar = 'K'; break;
    case KEY_L: retChar = 'L'; break;
    case KEY_M: retChar = 'M'; break;
    case KEY_N: retChar = 'N'; break;
    case KEY_O: retChar = 'O'; break;
    case KEY_P: retChar = 'P'; break;
    case KEY_Q: retChar = 'Q'; break;
    case KEY_R: retChar = 'R'; break;
    case KEY_S: retChar = 'S'; break;
    case KEY_T: retChar = 'T'; break;
    case KEY_U: retChar = 'U'; break;
    case KEY_V: retChar = 'V'; break;
    case KEY_W: retChar = 'W'; break;
    case KEY_X: retChar = 'X'; break;
    case KEY_Y: retChar = 'Y'; break;
    case KEY_Z: retChar = 'Z'; break;

    case KEY_SPACE: retChar = ' '; break;
    case KEY_ENTER: retChar = '\n'; break;

    case KEY_NUMPAD0: retChar = '0'; break;
    case KEY_NUMPAD1: retChar = '1'; break;
    case KEY_NUMPAD2: retChar = '2'; break;
    case KEY_NUMPAD3: retChar = '3'; break;
    case KEY_NUMPAD4: retChar = '4'; break;
    case KEY_NUMPAD5: retChar = '5'; break;
    case KEY_NUMPAD6: retChar = '6'; break;
    case KEY_NUMPAD7: retChar = '7'; break;
    case KEY_NUMPAD8: retChar = '8'; break;
    case KEY_NUMPAD9: retChar = '9'; break;

    case KEY_PERIOD: retChar = '.'; break;
    case KEY_COMMA: retChar = ','; break;
    case KEY_PLUS: retChar = '='; break;
    case KEY_MINUS: retChar = '-'; break;
    case KEY_COLON: retChar = ';'; break;
    case KEY_QUESTION: retChar = '/'; break;
    case KEY_SQUIGGLE: retChar = '`'; break;
    case KEY_LEFT_BRACKET: retChar = '['; break;
    case KEY_BACK_SLASH: retChar = '\\'; break;
    case KEY_RIGHT_BRACKET: retChar = ']'; break;
    case KEY_QUOTE: retChar = '\''; break;
    case KEY_DECIMAL: retChar = '.'; break;

    default: break;
    }

    return retChar;
}

void INP_SetMouseButton(int32_t button)
{
    OCT_ASSERT(button >= 0 && button < INPUT_MAX_MOUSE_BUTTONS);
    InputState& input = GetEngineState()->mInput;
    input.mMouseButtons[button] = true;

    if (button == MOUSE_LEFT)
    {
        input.mTouches[0] = true;
    }
}

void INP_ClearMouseButton(int32_t button)
{
    OCT_ASSERT(button >= 0 && button < INPUT_MAX_MOUSE_BUTTONS);
    InputState& input = GetEngineState()->mInput;
    input.mMouseButtons[button] = false;

    if (button == MOUSE_LEFT)
    {
        input.mTouches[0] = false;
    }
}

void INP_SetScrollWheelDelta(int32_t delta)
{
    GetEngineState()->mInput.mScrollWheelDelta = delta;
}

bool INP_IsMouseButtonDown(int32_t button)
{
    OCT_ASSERT(button >= 0 && button < INPUT_MAX_MOUSE_BUTTONS);
    return GetEngineState()->mInput.mMouseButtons[button];
}

bool INP_IsMouseButtonJustDown(int32_t button)
{
    OCT_ASSERT(button >= 0 && button < INPUT_MAX_MOUSE_BUTTONS);
    InputState& input = GetEngineState()->mInput;
    return input.mMouseButtons[button] && !input.mPrevMouseButtons[button];
}

bool INP_IsMouseButtonJustUp(int32_t button)
{
    OCT_ASSERT(button >= 0 && button < INPUT_MAX_MOUSE_BUTTONS);
    InputState& input = GetEngineState()->mInput;
    return !input.mMouseButtons[button] && input.mPrevMouseButtons[button];
}

int32_t INP_GetScrollWheelDelta()
{
    return GetEngineState()->mInput.mScrollWheelDelta;
}

void INP_ClearAllMouseButtons()
{
#if INPUT_MOUSE_SUPPORT
    InputState& input = GetEngineState()->mInput;

    for (uint32_t i = 0; i < INPUT_MAX_MOUSE_BUTTONS; i++)
    {
        input.mMouseButtons[i] = false;
    }
#endif
}

void INP_SetTouch(int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    GetEngineState()->mInput.mTouches[touch] = true;
}

void INP_ClearTouch(int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    GetEngineState()->mInput.mTouches[touch] = false;
}

bool INP_IsTouchDown(int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    return GetEngineState()->mInput.mTouches[touch];
}

bool INP_IsPointerDown(int32_t pointer)
{
    // If either the left mouse button is down or the specified
    // touch index is down, then return 1.
    OCT_ASSERT(pointer >= 0 && pointer < INPUT_MAX_TOUCHES);
    InputState& input = GetEngineState()->mInput;
    return input.mTouches[pointer];
}

bool INP_IsPointerJustUp(int32_t pointer)
{
    // If either the left mouse button is down or the specified
    // touch index is down, then return 1.
    OCT_ASSERT(pointer >= 0 && pointer < INPUT_MAX_TOUCHES);
    InputState& input = GetEngineState()->mInput;
    return (!input.mTouches[pointer] && input.mPrevTouches[pointer]);
}

bool INP_IsPointerJustDown(int32_t pointer)
{
    // If either the left mouse button is down or the specified
    // touch index is down, then return 1.
    OCT_ASSERT(pointer >= 0 && pointer < INPUT_MAX_TOUCHES);
    InputState& input = GetEngineState()->mInput;
    return (input.mTouches[pointer] && !input.mPrevTouches[pointer]);
}

void INP_SetMousePosition(int32_t mouseX, int32_t mouseY)
{
    // First index in pointer array is mouse position.
    InputState& input = GetEngineState()->mInput;
    input.mPointerX[0] = mouseX;
    input.mPointerY[0] = mouseY;
}

void INP_SetTouchPosition(int32_t touchX, int32_t touchY, int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    InputState& input = GetEngineState()->mInput;
    input.mPointerX[touch] = touchX;
    input.mPointerY[touch] = touchY;
}

void INP_GetMousePosition(int32_t& mouseX, int32_t& mouseY)
{
    // First pointer location is for mouse.
    InputState& input = GetEngineState()->mInput;
    mouseX = input.mPointerX[0];
    mouseY = input.mPointerY[0];
}

void INP_GetTouchPosition(int32_t& touchX, int32_t& touchY, int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    InputState& input = GetEngineState()->mInput;
    touchX = input.mPointerX[touch];
    touchY = input.mPointerY[touch];
}

void INP_GetTouchPositionNormalized(float& touchX, float& touchY, int32_t touch)
{
    OCT_ASSERT(touch >= 0 && touch < INPUT_MAX_TOUCHES);
    const EngineState* engineState = GetEngineState();
    const InputState* inputState = &engineState->mInput;
    touchX = (inputState->mPointerX[touch] - (engineState->mWindowWidth / 2.0f)) / (engineState->mWindowWidth / 2.0f);
    touchY = (inputState->mPointerY[touch] - (engineState->mWindowHeight / 2.0f)) / (engineState->mWindowHeight / 2.0f);
}

void INP_GetPointerPosition(int32_t& pointerX, int32_t& pointerY, int32_t pointer)
{
    INP_GetTouchPosition(pointerX,
        pointerY,
        pointer);
}

void INP_GetPointerPositionNormalized(float& pointerX, float& pointerY, int32_t  pointer)
{
    INP_GetTouchPositionNormalized(pointerX,
        pointerY,
        pointer);
}

void INP_GetMouseDelta(int32_t& deltaX, int32_t& deltaY)
{
    deltaX = GetEngineState()->mInput.mMouseDeltaX;
    deltaY = GetEngineState()->mInput.mMouseDeltaY;
}

bool INP_IsGamepadButtonDown(int32_t gamepadButton, int32_t gamepadIndex)
{
    InputState& input = GetEngineState()->mInput;
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    OCT_ASSERT(gamepadButton >= 0 && gamepadButton < INPUT_MAX_GAMEPAD_BUTTONS);
    return input.mGamepads[gamepadIndex].mButtons[gamepadButton];
}

bool INP_IsGamepadButtonJustDown(int32_t gamepadButton, int32_t gamepadIndex)
{
    InputState& input = GetEngineState()->mInput;
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    OCT_ASSERT(gamepadButton >= 0 && gamepadButton < INPUT_MAX_GAMEPAD_BUTTONS);
    return input.mGamepads[gamepadIndex].mButtons[gamepadButton] &&
        !input.mPrevGamepads[gamepadIndex].mButtons[gamepadButton];
}

bool INP_IsGamepadButtonJustUp(int32_t gamepadButton, int32_t gamepadIndex)
{
    InputState& input = GetEngineState()->mInput;
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    OCT_ASSERT(gamepadButton >= 0 && gamepadButton < INPUT_MAX_GAMEPAD_BUTTONS);
    return !input.mGamepads[gamepadIndex].mButtons[gamepadButton] &&
        input.mPrevGamepads[gamepadIndex].mButtons[gamepadButton];
}

float INP_GetGamepadAxisValue(int32_t gamepadAxis, int32_t gamepadIndex)
{
    InputState& input = GetEngineState()->mInput;
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    OCT_ASSERT(gamepadAxis >= 0 && gamepadAxis < INPUT_MAX_GAMEPAD_AXES);
    return input.mGamepads[gamepadIndex].mAxes[gamepadAxis];
}

GamepadType INP_GetGamepadType(int32_t gamepadIndex)
{
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    InputState& input = GetEngineState()->mInput;
    return input.mGamepads[gamepadIndex].mType;
}

bool INP_IsGamepadConnected(int32_t gamepadIndex)
{
    OCT_ASSERT(gamepadIndex >= 0 && gamepadIndex < INPUT_MAX_GAMEPADS);
    InputState& input = GetEngineState()->mInput;
    return input.mGamepads[gamepadIndex].mConnected;
}

int32_t INP_GetGamepadIndex(int32_t inputDevice)
{
#if 0
    // This is really only used for Android right now? Possibly move into Input_Android.cpp
    int32_t i = 0;
    InputState& input = GetEngineState()->mInput;

    for (i = 0; i < INPUT_MAX_GAMEPADS; i++)
    {
        if (input.mGamepads[i].mDevice == inputDevice)
        {
            return i;
        }
        else if (input.mGamepads[i].mDevice == -1)
        {
            // We didn't find the gamepad, and this slot isn't used, so assign it.
            // On android, I'm currently doing a simple implementation where once a controller is
            // detected for the first time, it is always connected.
            input.mGamepads[i].mDevice = inputDevice;
            input.mGamepads[i].mConnected = true;
            return i;
        }
    }
#else
    // Possibly only support 1 gamepad on android. Since unplugging / plugging in controll
    // will switch to a new device id. Only allowing one gamepad will simplify the experience.
    // Don't need to restart game if you plug in a diff controller.
    return 0;
#endif

    return -1;
}

void INP_SetGamepadAxisValue(GamepadAxisCode axisCode, float axisValue, int32_t gamepadIndex)
{
    if (gamepadIndex >= 0 &&
        gamepadIndex < INPUT_MAX_GAMEPADS)
    {
        InputState& input = GetEngineState()->mInput;
        input.mGamepads[gamepadIndex].mAxes[(int32_t)axisCode] = axisValue;
    }
}

void INP_SetGamepadButton(GamepadButtonCode buttonCode, int32_t gamepadIndex)
{
    if (gamepadIndex >= 0 &&
        gamepadIndex < INPUT_MAX_GAMEPADS)
    {
        InputState& input = GetEngineState()->mInput;
        input.mGamepads[gamepadIndex].mButtons[(int32_t)buttonCode] = 1;
    }
}

void INP_ClearGamepadButton(GamepadButtonCode buttonCode, int32_t gamepadIndex)
{
    if (gamepadIndex >= 0 &&
        gamepadIndex < INPUT_MAX_GAMEPADS)
    {
        InputState& input = GetEngineState()->mInput;
        input.mGamepads[gamepadIndex].mButtons[(int32_t)buttonCode] = 0;
    }
}
