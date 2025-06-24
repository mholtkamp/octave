#if PLATFORM_WINDOWS

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"

void INP_Initialize()
{
    RAWINPUTDEVICE Rid;

    // Mouse
    Rid.usUsagePage = 1;
    Rid.usUsage = 2;
    Rid.dwFlags = 0;
    Rid.hwndTarget = NULL;

    if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)))
    {
        LogError("Failed to register RawInput device");
    }

    InputInit();
}

void INP_Shutdown()
{
    InputShutdown();
}

void INP_Update()
{
    InputAdvanceFrame();

    InputState& input = GetEngineState()->mInput;
    memset(input.mXinputStates, 0, sizeof(XINPUT_STATE) * INPUT_MAX_GAMEPADS);

    for (int32_t i = 0; i < XUSER_MAX_COUNT && i < INPUT_MAX_GAMEPADS; i++)
    {
        if (!XInputGetState(i, &input.mXinputStates[i]))
        {
            input.mGamepads[i].mConnected = true;

            // Buttons
            input.mGamepads[i].mButtons[GAMEPAD_A] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_A;
            input.mGamepads[i].mButtons[GAMEPAD_B] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_B;
            input.mGamepads[i].mButtons[GAMEPAD_X] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_X;
            input.mGamepads[i].mButtons[GAMEPAD_Y] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y;
            input.mGamepads[i].mButtons[GAMEPAD_L1] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
            input.mGamepads[i].mButtons[GAMEPAD_R1] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
            input.mGamepads[i].mButtons[GAMEPAD_THUMBL] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
            input.mGamepads[i].mButtons[GAMEPAD_THUMBR] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
            input.mGamepads[i].mButtons[GAMEPAD_START] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_START;
            input.mGamepads[i].mButtons[GAMEPAD_SELECT] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
            input.mGamepads[i].mButtons[GAMEPAD_LEFT] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
            input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
            input.mGamepads[i].mButtons[GAMEPAD_UP] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
            input.mGamepads[i].mButtons[GAMEPAD_DOWN] = input.mXinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;

            // Axes
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] = (float)input.mXinputStates[i].Gamepad.bLeftTrigger / 255;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] = (float)input.mXinputStates[i].Gamepad.bRightTrigger / 255;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = (float)input.mXinputStates[i].Gamepad.sThumbLX / 32767;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = (float)input.mXinputStates[i].Gamepad.sThumbLY / 32767;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] = (float)input.mXinputStates[i].Gamepad.sThumbRX / 32767;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = (float)input.mXinputStates[i].Gamepad.sThumbRY / 32767;

            // Set digital inputs for analog triggers
            input.mGamepads[i].mButtons[GAMEPAD_L2] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] > 0.2f;
            input.mGamepads[i].mButtons[GAMEPAD_R2] = input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] > 0.2f;
        }
        else
        {
            input.mGamepads[i].mConnected = false;
        }
    }

    InputPostUpdate();
}

void INP_SetCursorPos(int32_t x, int32_t y)
{
    INP_SetMousePosition(x, y);

    POINT screenPoint = { x, y };
    ClientToScreen(GetEngineState()->mSystem.mWindow, &screenPoint);

    SetCursorPos(screenPoint.x, screenPoint.y);
}

void INP_ShowCursor(bool show)
{
    SystemState& system = GetEngineState()->mSystem;

    // Calling ShowCursor(true) twice will cause a sort of ref counting problem,
    // and then future ShowCursor(false) calls won't work.
    if (system.mWindowHasFocus && show != GetEngineState()->mInput.mCursorShown)
    {
        ShowCursor(show);
    }

    GetEngineState()->mInput.mCursorShown = show;
}

void INP_LockCursor(bool lock)
{
    SystemState& system = GetEngineState()->mSystem;
    InputState& input = GetEngineState()->mInput;
    input.mCursorLocked = lock;
}

void INP_TrapCursor(bool trap)
{
    SystemState& system = GetEngineState()->mSystem;
    InputState& input = GetEngineState()->mInput;

    if (system.mWindowHasFocus)
    {
        if (trap)
        {
            RECT rect;
            GetClientRect(system.mWindow, &rect);

            POINT tl;
            tl.x = rect.left;
            tl.y = rect.top;

            POINT br;
            br.x = rect.right;
            br.y = rect.bottom;

            MapWindowPoints(system.mWindow, nullptr, &tl, 1);
            MapWindowPoints(system.mWindow, nullptr, &br, 1);

            rect.left = tl.x;
            rect.top = tl.y;
            rect.right = br.x;
            rect.bottom = br.y;

            ClipCursor(&rect);
        }
        else
        {
            ClipCursor(nullptr);
        }
    }

    input.mCursorTrapped = trap;
}

void INP_ShowSoftKeyboard(bool show)
{

}

bool INP_IsSoftKeyboardShown()
{
    return false;
}


#endif