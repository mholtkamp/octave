#if PLATFORM_LINUX

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"

#include <xcb/xcb.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

bool gWarpCursor = false;
int32_t gWarpCursorX = -1;
int32_t gWarpCursorY = -1;

static int32_t jsHandles[INPUT_MAX_GAMEPADS] = {-1, -1, -1, -1};
static const char* jsDevicePaths[INPUT_MAX_GAMEPADS] = 
{
    "/dev/input/js0",
    "/dev/input/js1",
    "/dev/input/js2",
    "/dev/input/js3"
};

int32_t ReadJoystickEvent(int32_t fd, struct js_event* event)
{
    ssize_t bytes = 0;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
    {
        return 0;
    }

    return -1;
}

void HandleJoystickButtonEvent(int32_t gamepadIndex, int32_t number, int32_t value)
{
    InputState& input = GetEngineState()->mInput;

    int32_t code = -1;

    // TODO: Handle other controllers besides my third party xbox 360 controller.
    // I would hope there is some button code standard but... might have to look at what SDL does.
    switch(number)
    {
        case 0: code = GAMEPAD_A; break;
        case 1: code = GAMEPAD_B; break;
        case 2: code = GAMEPAD_X; break;
        case 3: code = GAMEPAD_Y; break;
        case 4: code = GAMEPAD_L1; break;
        case 5: code = GAMEPAD_R1; break;
        case 6: code = GAMEPAD_SELECT; break;
        case 7: code = GAMEPAD_START; break;
        //case 8: code = GAMEPAD_HOME; break;
        //case 9: code = GAMEPAD_L3; break;
        //case 10: code = GAMEPAD_R3; break;
        default: break;
    }

    if (code != -1)
    {
        input.mGamepads[gamepadIndex].mButtons[code] = value;
    }
}

void HandleJoystickAxisEvent(int32_t gamepadIndex, int32_t number, int32_t value)
{
    InputState& input = GetEngineState()->mInput;

    int32_t code = -1;

    // TODO: Handle other controllers besides my third party xbox 360 controller.
    // I would hope there is some button code standard but... might have to look at what SDL does.
    switch(number)
    {
        case 0: code = GAMEPAD_AXIS_LTHUMB_X; break;
        case 1: code = GAMEPAD_AXIS_LTHUMB_Y; break;
        case 2: code = GAMEPAD_AXIS_LTRIGGER; break;
        case 3: code = GAMEPAD_AXIS_RTHUMB_X; break;
        case 4: code = GAMEPAD_AXIS_RTHUMB_Y; break;
        case 5: code = GAMEPAD_AXIS_RTRIGGER; break;
        default: break;
    }

    // DPad is an axis for some reason, need to conver it to button presses.
    if (number == 6)
    {
        // 6 = left-right dpad
        input.mGamepads[gamepadIndex].mButtons[GAMEPAD_LEFT] = value < 0;
        input.mGamepads[gamepadIndex].mButtons[GAMEPAD_RIGHT] = value > 0;
    }
    else if (number == 7)
    {
        // 7 = up-down dpad
        input.mGamepads[gamepadIndex].mButtons[GAMEPAD_UP] = value < 0;
        input.mGamepads[gamepadIndex].mButtons[GAMEPAD_DOWN] = value > 0;
    }

    // Set and clear trigger buttons based on axis value
    if (code == GAMEPAD_AXIS_LTRIGGER ||
        code == GAMEPAD_AXIS_RTRIGGER)
    {
        int32_t buttonCode = (code == GAMEPAD_AXIS_LTRIGGER) ? GAMEPAD_L2 : GAMEPAD_R2;
        input.mGamepads[gamepadIndex].mButtons[buttonCode] = value > 0;
    }

    // Invert analog vertical axes
    if (code == GAMEPAD_AXIS_LTHUMB_Y ||
        code == GAMEPAD_AXIS_RTHUMB_Y)
    {
        value *= -1;
    }

    if (code != -1)
    {
        input.mGamepads[gamepadIndex].mAxes[code] = glm::clamp(value / 32767.0f, -1.0f, 1.0f);
    }

}

void INP_Initialize()
{

}

void INP_Shutdown()
{

}

void INP_Update()
{
    InputAdvanceFrame();

    struct js_event event;

    // Update connected gamepads. -1 indicates no gamepad connected.
    for (int32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        if (jsHandles[i] == -1)
        {
            // Attempt to open the controller
            jsHandles[i] = open(jsDevicePaths[i], O_RDONLY | O_NONBLOCK);
        }
        else
        {
            while (ReadJoystickEvent(jsHandles[i], &event) == 0)
            {
                switch (event.type)
                {
                    case JS_EVENT_BUTTON:
                    {
                        //printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
                        HandleJoystickButtonEvent(i, event.number, event.value);
                        break;
                    }
                    case JS_EVENT_AXIS:
                    {
                        // printf("Axis %zu = %6d\n", event.number, event.value);
                        HandleJoystickAxisEvent(i, event.number, event.value);
                            
                        break;
                    }
                    default:
                        /* Ignore init events. */
                        break;
                }
                
                fflush(stdout);
            }
        }
    }

    InputPostUpdate();
}

void INP_SetCursorPos(int32_t x, int32_t y)
{
    gWarpCursor = true;
    gWarpCursorX = x;
    gWarpCursorY = y;

    INP_SetMousePosition(x, y);
}

void INP_ShowCursor(bool show)
{
    SystemState& system = GetEngineState()->mSystem;
    uint32_t mask = XCB_CW_CURSOR;
    uint32_t valueList = show ? XCB_NONE : system.mNullCursor;
    xcb_change_window_attributes (system.mXcbConnection, system.mXcbWindow, mask, &valueList);
    xcb_flush(system.mXcbConnection);
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
            xcb_grab_pointer_cookie_t grabCookie;
            xcb_grab_pointer_reply_t* grabReply = nullptr;
            
            grabCookie = xcb_grab_pointer(
                system.mXcbConnection,
                1,
                system.mXcbWindow,
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE |
                XCB_EVENT_MASK_POINTER_MOTION,
                XCB_GRAB_MODE_ASYNC,
                XCB_GRAB_MODE_ASYNC,
                system.mXcbWindow,
                XCB_NONE,
                XCB_CURRENT_TIME
            );

            grabReply = xcb_grab_pointer_reply(system.mXcbConnection, grabCookie, nullptr); 
            free(grabReply);
        }
        else
        {
            xcb_ungrab_pointer(system.mXcbConnection, XCB_CURRENT_TIME);
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