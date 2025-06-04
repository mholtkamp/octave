#if PLATFORM_DOLPHIN

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"
#include "Maths.h"

#if PLATFORM_WII
#include <wiiuse/wpad.h>
#endif

void INP_Initialize()
{
#if PLATFORM_WII
    PAD_Init();
    WPAD_Init();

    WPAD_SetVRes(0, 640, 480);
    
    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        WPAD_SetDataFormat(WPAD_CHAN_0 + i, WPAD_FMT_BTNS_ACC_IR);

        InputState& input = GetEngineState()->mInput;
        input.mGamepads[i].mType = GamepadType::Wiimote;
    }

#else
    PAD_Init();
#endif
}

void INP_Shutdown()
{

}

void INP_Update()
{
    InputAdvanceFrame();

    InputState& input = GetEngineState()->mInput;

#if PLATFORM_WII
    WPAD_ScanPads();

    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        WPADData* data = WPAD_Data(i);

        input.mGamepads[i].mConnected = (data->err == WPAD_ERR_NONE);

        struct expansion_t expData;
        WPAD_Expansion(i, &expData); // Get expansion info from the first wiimote

        if (expData.type == WPAD_EXP_CLASSIC)
        {
            int32_t held = expData.classic.btns_held;

            if (held != 0)
            {
                input.mGamepads[i].mType = GamepadType::WiiClassic;
            }

            if (input.mGamepads[i].mType != GamepadType::WiiClassic)
            {
                continue;
            }

            // Buttons
            input.mGamepads[i].mButtons[GAMEPAD_A] = held & CLASSIC_CTRL_BUTTON_A;
            input.mGamepads[i].mButtons[GAMEPAD_B] = held & CLASSIC_CTRL_BUTTON_B;
            input.mGamepads[i].mButtons[GAMEPAD_X] = held & CLASSIC_CTRL_BUTTON_X;
            input.mGamepads[i].mButtons[GAMEPAD_Y] = held & CLASSIC_CTRL_BUTTON_Y;
            input.mGamepads[i].mButtons[GAMEPAD_Z] = held & CLASSIC_CTRL_BUTTON_ZL;
            input.mGamepads[i].mButtons[GAMEPAD_R1] = held & CLASSIC_CTRL_BUTTON_FULL_R;
            input.mGamepads[i].mButtons[GAMEPAD_L1] = held & CLASSIC_CTRL_BUTTON_FULL_L;
            input.mGamepads[i].mButtons[GAMEPAD_LEFT] = held & CLASSIC_CTRL_BUTTON_LEFT;
            input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = held & CLASSIC_CTRL_BUTTON_RIGHT;
            input.mGamepads[i].mButtons[GAMEPAD_DOWN] = held & CLASSIC_CTRL_BUTTON_DOWN;
            input.mGamepads[i].mButtons[GAMEPAD_UP] = held & CLASSIC_CTRL_BUTTON_UP;
            input.mGamepads[i].mButtons[GAMEPAD_START] = held & CLASSIC_CTRL_BUTTON_PLUS;
            input.mGamepads[i].mButtons[GAMEPAD_SELECT] = held & CLASSIC_CTRL_BUTTON_MINUS;
            input.mGamepads[i].mButtons[GAMEPAD_HOME] = held & CLASSIC_CTRL_BUTTON_HOME;

            // Axes
            int32_t leftStickX = int32_t(expData.classic.ljs.pos.x - expData.classic.ljs.center.x);
            int32_t leftStickY = int32_t(expData.classic.ljs.pos.y - expData.classic.ljs.center.y);
            int32_t rightStickX = int32_t(expData.classic.rjs.pos.x - expData.classic.rjs.center.x);
            int32_t rightStickY = int32_t(expData.classic.rjs.pos.y - expData.classic.rjs.center.y);
            float leftTrigger = expData.classic.l_shoulder;
            float rightTrigger = expData.classic.r_shoulder;

            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(leftStickX / 31.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(leftStickY / 31.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] = glm::clamp(rightStickX / 15.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = glm::clamp(rightStickY / 15.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] = leftTrigger;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] = rightTrigger;
        }
        else
        {
            int32_t held = WPAD_ButtonsHeld(i);

            if (held != 0)
            {
                input.mGamepads[i].mType = GamepadType::Wiimote;
            }

            if (input.mGamepads[i].mType != GamepadType::Wiimote)
            {
                continue;
            }

            // Buttons
            input.mGamepads[i].mButtons[GAMEPAD_A] = held & WPAD_BUTTON_A;
            input.mGamepads[i].mButtons[GAMEPAD_B] = held & WPAD_BUTTON_B;
            input.mGamepads[i].mButtons[GAMEPAD_X] = held & WPAD_BUTTON_1;
            input.mGamepads[i].mButtons[GAMEPAD_Y] = held & WPAD_BUTTON_2;
            input.mGamepads[i].mButtons[GAMEPAD_Z] = held & WPAD_NUNCHUK_BUTTON_Z;
            input.mGamepads[i].mButtons[GAMEPAD_C] = held & WPAD_NUNCHUK_BUTTON_C;
            input.mGamepads[i].mButtons[GAMEPAD_LEFT] = held & WPAD_BUTTON_LEFT;
            input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = held & WPAD_BUTTON_RIGHT;
            input.mGamepads[i].mButtons[GAMEPAD_DOWN] = held & WPAD_BUTTON_DOWN;
            input.mGamepads[i].mButtons[GAMEPAD_UP] = held & WPAD_BUTTON_UP;
            input.mGamepads[i].mButtons[GAMEPAD_START] = held & WPAD_BUTTON_PLUS;
            input.mGamepads[i].mButtons[GAMEPAD_SELECT] = held & WPAD_BUTTON_MINUS;
            input.mGamepads[i].mButtons[GAMEPAD_HOME] = held & WPAD_BUTTON_HOME;

            struct expansion_t expData;
            WPAD_Expansion(i, &expData); // Get expansion info from the first wiimote

            if (expData.type == WPAD_EXP_NUNCHUK)
            {
                int32_t stickX = int32_t(expData.nunchuk.js.pos.x - expData.nunchuk.js.center.x);
                int32_t stickY = int32_t(expData.nunchuk.js.pos.y - expData.nunchuk.js.center.y);

                // Axes
                input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(stickX / 127.0f, -1.0f, 1.0f);
                input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(stickY / 127.0f, -1.0f, 1.0f);
            }

            // IR support
            struct ir_t irData;
            WPAD_IR(i, &irData);
            INP_SetTouchPosition((int32_t)irData.x, (int32_t)irData.y, i);
        }
    }

#endif

    PADStatus pads[4];
    //PAD_ScanPads();
    PAD_Read(pads);

    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        input.mGamepads[i].mConnected = (pads[i].err == PAD_ERR_NONE);

#if PLATFORM_WII
        if (pads[i].button != 0)
        {
            input.mGamepads[i].mType = GamepadType::GameCube;
        }

        if (input.mGamepads[i].mType != GamepadType::GameCube)
        {
            continue;
        }
#endif

        // Buttons
        input.mGamepads[i].mButtons[GAMEPAD_A] = pads[i].button & PAD_BUTTON_A;
        input.mGamepads[i].mButtons[GAMEPAD_B] = pads[i].button & PAD_BUTTON_B;
        input.mGamepads[i].mButtons[GAMEPAD_X] = pads[i].button & PAD_BUTTON_X;
        input.mGamepads[i].mButtons[GAMEPAD_Y] = pads[i].button & PAD_BUTTON_Y;
        input.mGamepads[i].mButtons[GAMEPAD_Z] = pads[i].button & PAD_TRIGGER_Z;
        input.mGamepads[i].mButtons[GAMEPAD_R1] = pads[i].button & PAD_TRIGGER_R;
        input.mGamepads[i].mButtons[GAMEPAD_L1] = pads[i].button & PAD_TRIGGER_L;
        input.mGamepads[i].mButtons[GAMEPAD_LEFT] = pads[i].button & PAD_BUTTON_LEFT;
        input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = pads[i].button & PAD_BUTTON_RIGHT;
        input.mGamepads[i].mButtons[GAMEPAD_DOWN] = pads[i].button & PAD_BUTTON_DOWN;
        input.mGamepads[i].mButtons[GAMEPAD_UP] = pads[i].button & PAD_BUTTON_UP;
        input.mGamepads[i].mButtons[GAMEPAD_START] = pads[i].button & PAD_BUTTON_START;
        input.mGamepads[i].mButtons[GAMEPAD_HOME] = 0;

        // Axes
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(pads[i].stickX / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(pads[i].stickY / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] = glm::clamp(pads[i].substickX / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = glm::clamp(pads[i].substickY / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] = pads[i].triggerL / 255.0f;
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] = pads[i].triggerR / 255.0f;
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

void INP_ShowSoftKeyboard(bool show)
{

}

bool INP_IsSoftKeyboardShown()
{
    return false;
}


#endif