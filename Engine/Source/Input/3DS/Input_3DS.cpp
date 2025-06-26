#if PLATFORM_3DS

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"
#include "Maths.h"
#include "Signals.h"
#include "System/System.h"

#include <3ds.h>

//Ref
ThreadObject* SYS_CreateThread(ThreadFuncFP func, void* arg);
void SYS_JoinThread(ThreadObject* thread);
void SYS_DestroyThread(ThreadObject* thread);
MutexObject* SYS_CreateMutex();
void SYS_LockMutex(MutexObject* mutex);
void SYS_UnlockMutex(MutexObject* mutex);
void SYS_DestroyMutex(MutexObject* mutex);

enum class SwkbdStatus : uint8_t
{
    Closed,
    Open,
    Finished,

    Count
};

#define SWKBD_BUFFER_SIZE 256
static ThreadObject* sSwkbdThread = nullptr;
static MutexObject* sSwkbdMutex = nullptr;
static SwkbdStatus sSwkbdStatus = SwkbdStatus::Closed;
static std::string sSwkbdText;

static ThreadFuncRet SwkbdThreadFunc(void* in)
{
    static char sTextBuffer[SWKBD_BUFFER_SIZE] = {};

    SwkbdState swkbd;
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 3, -1);
    swkbdSetHintText(&swkbd, "Enter Name");
    SwkbdButton button = swkbdInputText(&swkbd, sTextBuffer, sizeof(sTextBuffer));

    LogDebug("SWKBD Text = %s", sTextBuffer);

    {
        SCOPED_LOCK(sSwkbdMutex);
        sSwkbdText = sTextBuffer;
        sSwkbdStatus = SwkbdStatus::Finished;
    }
}

void INP_Initialize()
{
    InputInit();

    sSwkbdMutex = SYS_CreateMutex();
}

void INP_Shutdown()
{
    if (sSwkbdThread != nullptr)
    {
        // Will this destroy the thread properly?
        SYS_DestroyThread(sSwkbdThread);
        sSwkbdThread = nullptr;
    }

    SYS_DestroyMutex(sSwkbdMutex);
    sSwkbdMutex = nullptr;

    InputShutdown();
}

void INP_Update()
{
    InputAdvanceFrame();
    InputState& input = GetEngineState()->mInput;

    hidScanInput();
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

    if (down & KEY_CSTICK_RIGHT)
        input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_X] = 1.0f;
    if (down & KEY_CSTICK_LEFT)
        input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_X] = -1.0f;
    if (down & KEY_CSTICK_UP)
        input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = 1.0f;
    if (down & KEY_CSTICK_DOWN)
        input.mGamepads[0].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = -1.0f;

    // Touch
    if (down & KEY_TOUCH)
    {
        INP_SetTouch(0);

        touchPosition touch;
        hidTouchRead(&touch);
        INP_SetTouchPosition(touch.px, touch.py, 0);

        //LogDebug("Touch: %d, %d", touch.px, touch.py);
    }
    else
    {
        INP_ClearTouch(0);
    }

    {
        SCOPED_LOCK(sSwkbdMutex);

        if (sSwkbdStatus == SwkbdStatus::Finished)
        {
            InputState& input = GetEngineState()->mInput;
            if (input.mSoftwareKeyboardEntrySignal)
            {
                std::vector<Datum> args = { sSwkbdText };
                input.mSoftwareKeyboardEntrySignal->Emit(args);
            }

            sSwkbdStatus = SwkbdStatus::Closed;
            SYS_JoinThread(sSwkbdThread);
            SYS_DestroyThread(sSwkbdThread);
            sSwkbdThread = nullptr;
        }
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
    SCOPED_LOCK(sSwkbdMutex);

    if (sSwkbdStatus != SwkbdStatus::Closed)
    {
        return;
    }

    // I don't think we can externally close the keyboard if it's up.
    if (show)
    {
        if (sSwkbdThread != nullptr)
        {
            LogError("Keyboard thread is already running!");
        }

        sSwkbdThread = SYS_CreateThread(SwkbdThreadFunc, nullptr);
        sSwkbdStatus = SwkbdStatus::Open;
    }
}

bool INP_IsSoftKeyboardShown()
{
    return false;
}


#endif