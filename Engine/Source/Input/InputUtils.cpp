#include "Input/InputUtils.h"
#include "Input/InputConstants.h"
#include "Input/Input.h"
#include "System/System.h"

#include "Engine.h"

#include <stdlib.h>
#include <string.h>

void InputAdvanceFrame()
{
    InputState& input = GetEngineState()->mInput;

#if INPUT_KEYBOARD_SUPPORT
    memcpy(input.mPrevKeys, input.mKeys, INPUT_MAX_KEYS * sizeof(bool));
    memset(input.mRepeatKeys, 0, INPUT_MAX_KEYS * sizeof(bool));
    input.mJustDownKeys.clear();
#endif

#if INPUT_MOUSE_SUPPORT
    memcpy(input.mPrevMouseButtons, input.mMouseButtons, INPUT_MAX_MOUSE_BUTTONS * sizeof(bool));

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