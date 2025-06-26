#include "Input/InputUtils.h"
#include "Input/InputConstants.h"
#include "Input/Input.h"
#include "System/System.h"

#include "Engine.h"
#include "Signals.h"

#include <stdlib.h>
#include <string.h>

void InputInit()
{

}

void InputShutdown()
{

}

void InputAdvanceFrame()
{
    InputState& input = GetEngineState()->mInput;

#if INPUT_KEYBOARD_SUPPORT
    memcpy(input.mPrevKeys, input.mKeys, INPUT_MAX_KEYS * sizeof(bool));
    memset(input.mRepeatKeys, 0, INPUT_MAX_KEYS * sizeof(bool));
    input.mJustDownKeys.clear();
#endif

#if INPUT_MOUSE_SUPPORT
    memcpy(input.mPrevMouseButtons, input.mMouseButtons, MOUSE_BUTTON_COUNT * sizeof(bool));

    if (INP_IsCursorLocked() && SYS_DoesWindowHaveFocus())
    {
        // Reset mouse to center if locked
        int32_t x = GetEngineState()->mWindowWidth / 2;
        int32_t y = GetEngineState()->mWindowHeight / 2;
        INP_SetCursorPos(x, y);
    }

    input.mScrollWheelDelta = 0;
    input.mMouseDeltaX = 0;
    input.mMouseDeltaY = 0;
#endif

#if INPUT_TOUCH_SUPPORT
    memcpy(input.mPrevTouches, input.mTouches, INPUT_MAX_TOUCHES * sizeof(bool));
#endif

#if INPUT_GAMEPAD_SUPPORT
    memcpy(input.mPrevGamepads, input.mGamepads, INPUT_MAX_GAMEPADS * sizeof(GamepadState));
#endif
}

void InputPostUpdate()
{
    InputState& input = GetEngineState()->mInput;

    // Update axes as buttons
    for (int32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        input.mGamepads[i].mButtons[GAMEPAD_L_LEFT] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] < -0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_L_RIGHT] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] > 0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_L_DOWN] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] < -0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_L_UP] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] > 0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_R_LEFT] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] < -0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_R_RIGHT] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] > 0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_R_DOWN] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] < -0.5f;
        input.mGamepads[i].mButtons[GAMEPAD_R_UP] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] > 0.5f;
    }
}
