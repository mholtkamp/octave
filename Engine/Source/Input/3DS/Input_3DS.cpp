#if PLATFORM_3DS

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"
#include "Maths.h"
#include "Signals.h"
#include "System/System.h"

#include <3ds.h>

#define SWKBD_BUFFER_SIZE 256

void INP_Initialize()
{
    InputInit();

    irrstInit();
}

void INP_Shutdown()
{
    irrstExit();

    InputShutdown();
}

void INP_Update()
{
    InputAdvanceFrame();
    InputState& input = GetEngineState()->mInput;

    hidScanInput();
    irrstScanInput();

    uint32_t down = hidKeysHeld();

    // Gamepad 0 is always connected on 3DS
    input.mGamepads[0].mConnected = true;

    // Buttons
    input.mGamepads[0].mButtons[GAMEPAD_A] = down & KEY_A;
    input.mGamepads[0].mButtons[GAMEPAD_B] = down & KEY_B;
    input.mGamepads[0].mButtons[GAMEPAD_X] = down & KEY_X;
    input.mGamepads[0].mButtons[GAMEPAD_Y] = down & KEY_Y;
    input.mGamepads[0].mButtons[GAMEPAD_Z] = 0;
    input.mGamepads[0].mButtons[GAMEPAD_R1] = down & KEY_R;
    input.mGamepads[0].mButtons[GAMEPAD_L1] = down & KEY_L;
    input.mGamepads[0].mButtons[GAMEPAD_R2] = down & KEY_ZR;
    input.mGamepads[0].mButtons[GAMEPAD_L2] = down & KEY_ZL;
    input.mGamepads[0].mButtons[GAMEPAD_LEFT] = down & KEY_DLEFT;
    input.mGamepads[0].mButtons[GAMEPAD_RIGHT] = down & KEY_DRIGHT;
    input.mGamepads[0].mButtons[GAMEPAD_DOWN] = down & KEY_DDOWN;
    input.mGamepads[0].mButtons[GAMEPAD_UP] = down & KEY_DUP;
    input.mGamepads[0].mButtons[GAMEPAD_START] = down & KEY_START;
    input.mGamepads[0].mButtons[GAMEPAD_SELECT] = down & KEY_SELECT;

    // Axes
    circlePosition pos;
    hidCircleRead(&pos);

    input.mGamepads[0].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(pos.dx / 155.0f, -1.0f, 1.0f);
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(pos.dy / 155.0f, -1.0f, 1.0f);
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_X] = 0.0f;
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = 0.0f;
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_LTRIGGER] = (down & KEY_L || down & KEY_ZL) ? 1.0 : 0.0;
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTRIGGER] = (down & KEY_R || down & KEY_ZR) ? 1.0 : 0.0;

    // New 3DS Right-Stick
    circlePosition rsPos;
    irrstCstickRead(&rsPos);

    input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_X] = glm::clamp(rsPos.dx / 145.0f, -1.0f, 1.0f);
    input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = glm::clamp(rsPos.dy / 145.0f, -1.0f, 1.0f);

    // Touch
    if (down & KEY_TOUCH)
    {
        INP_SetTouch(0);

        touchPosition touch;
        hidTouchRead(&touch);
        INP_SetTouchPosition(touch.px, touch.py, 0);
    }
    else
    {
        INP_ClearTouch(0);
    }

    InputPostUpdate();
}

void INP_SetCursorPos(int32_t x, int32_t y)
{

}

void INP_ShowCursor(bool show)
{

}

void INP_LockCursor(bool lock)
{

}

void INP_TrapCursor(bool trap)
{

}

const char* INP_ShowSoftKeyboard(bool show)
{
    if (show)
    {
        static char sTextBuffer[SWKBD_BUFFER_SIZE] = {};

        SwkbdState swkbd;
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
        SwkbdButton button = swkbdInputText(&swkbd, sTextBuffer, SWKBD_BUFFER_SIZE);

        if (button == SWKBD_BUTTON_CONFIRM)
        {
            return sTextBuffer;
        }
    }

    return nullptr;
}

bool INP_IsSoftKeyboardShown()
{
    return false;
}


#endif