#if PLATFORM_WINDOWS

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"

void INP_Initialize()
{

}

void INP_Shutdown()
{

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
        }
        else
        {
            input.mGamepads[i].mConnected = false;
        }
    }

    // Update mouse position
    if (GetEngineState()->mSystem.mWindowHasFocus)
    {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(GetEngineState()->mSystem.mWindow, &point);

        INP_SetMousePosition(point.x, point.y);
    }
}

void INP_ShowCursor(bool show)
{
    ShowCursor(show);
}

void INP_SetCursorPos(int32_t x, int32_t y)
{
    INP_SetMousePosition(x, y);

    POINT screenPoint = { x, y };
    ClientToScreen(GetEngineState()->mSystem.mWindow, &screenPoint);
    
    // Delay the cursor reset until processing mouse events in SYS_Update().
    GetEngineState()->mSystem.mCursorSet = { screenPoint.x, screenPoint.y };
}

#endif