#if PLATFORM_LINUX

// This is a custom XCB backend written for Octave, based on the official Win32 backend.
// Right now it is coupled to some engine functionality for time/deltaTime.
// And it also converts keycodes using definitions in Input.h

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_xcb.h"

#include "Input/Input.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Utilities.h"

struct ImGui_ImplXcb_Data
{
    xcb_window_t                mWindow;

    ImGui_ImplXcb_Data()      { memset((void*)this, 0, sizeof(*this)); }
};

static ImGui_ImplXcb_Data* ImGui_ImplXcb_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplXcb_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

// Functions
static bool ImGui_ImplXcb_InitEx(xcb_window_t window, bool platform_has_own_dc)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplXcb_Data* bd = IM_NEW(ImGui_ImplXcb_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_xcb";

    bd->mWindow = window;
    //bd->TicksPerSecond = perf_frequency;
    //bd->Time = perf_counter;

    // Set platform dependent data in viewport
    ImGui::GetMainViewport()->PlatformHandleRaw = (void*)window;
    IM_UNUSED(platform_has_own_dc); // Used in 'docking' branch

    return true;
}

IMGUI_IMPL_API bool     ImGui_ImplXcb_Init(xcb_window_t window)
{
    return ImGui_ImplXcb_InitEx(window, false);
}

void    ImGui_ImplXcb_Shutdown()
{
    ImGui_ImplXcb_Data* bd = ImGui_ImplXcb_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad);
    IM_DELETE(bd);
}

static void ImGui_ImplXcb_AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(key, down);
    io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
    IM_UNUSED(native_scancode);
}

static void ImGui_ImplXcb_UpdateKeyModifiers()
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl,  IsControlDown());
    io.AddKeyEvent(ImGuiMod_Shift, IsShiftDown());
    io.AddKeyEvent(ImGuiMod_Alt, IsAltDown());
    io.AddKeyEvent(ImGuiMod_Super, false);
}

static void ImGui_ImplXcb_UpdateMouseData()
{
    ImGui_ImplXcb_Data* bd = ImGui_ImplXcb_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(bd->mWindow != 0);
}

void    ImGui_ImplXcb_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplXcb_Data* bd = ImGui_ImplXcb_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplXcb_Init()?");

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)GetEngineState()->mWindowWidth, (float)GetEngineState()->mWindowHeight);

    // Setup time step
    io.DeltaTime = glm::max(GetEngineState()->mRealDeltaTime, 0.0001f);

    // Update OS mouse position
    ImGui_ImplXcb_UpdateMouseData();

    ImGui_ImplXcb_UpdateKeyModifiers();
}

// Map VK_xxx to ImGuiKey_xxx.
static ImGuiKey ImGui_ImplXcb_VirtualKeyToImGuiKey(xcb_keycode_t keyCode)
{
    switch (keyCode)
    {
        case OCTAVE_KEY_TAB: return ImGuiKey_Tab;
        case OCTAVE_KEY_LEFT: return ImGuiKey_LeftArrow;
        case OCTAVE_KEY_RIGHT: return ImGuiKey_RightArrow;
        case OCTAVE_KEY_UP: return ImGuiKey_UpArrow;
        case OCTAVE_KEY_DOWN: return ImGuiKey_DownArrow;
        case OCTAVE_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case OCTAVE_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case OCTAVE_KEY_HOME: return ImGuiKey_Home;
        case OCTAVE_KEY_END: return ImGuiKey_End;
        case OCTAVE_KEY_INSERT: return ImGuiKey_Insert;
        case OCTAVE_KEY_DELETE: return ImGuiKey_Delete;
        case OCTAVE_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case OCTAVE_KEY_SPACE: return ImGuiKey_Space;
        case OCTAVE_KEY_ENTER: return ImGuiKey_Enter;
        case OCTAVE_KEY_ESCAPE: return ImGuiKey_Escape;
        case OCTAVE_KEY_QUOTE: return ImGuiKey_Apostrophe;
        case OCTAVE_KEY_COMMA: return ImGuiKey_Comma;
        case OCTAVE_KEY_MINUS: return ImGuiKey_Minus;
        case OCTAVE_KEY_PERIOD: return ImGuiKey_Period;
        case OCTAVE_KEY_QUESTION: return ImGuiKey_Slash;
        case OCTAVE_KEY_COLON: return ImGuiKey_Semicolon;
        case OCTAVE_KEY_PLUS: return ImGuiKey_Equal;
        case OCTAVE_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case OCTAVE_KEY_BACK_SLASH: return ImGuiKey_Backslash;
        case OCTAVE_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case OCTAVE_KEY_SQUIGGLE: return ImGuiKey_GraveAccent;
        //case OCTAVE_KEY_???: return ImGuiKey_CapsLock;
        //case OCTAVE_KEY_???: return ImGuiKey_ScrollLock;
        //case OCTAVE_KEY_???: return ImGuiKey_NumLock;
        //case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
        //case VK_PAUSE: return ImGuiKey_Pause;
        case OCTAVE_KEY_NUMPAD0: return ImGuiKey_Keypad0;
        case OCTAVE_KEY_NUMPAD1: return ImGuiKey_Keypad1;
        case OCTAVE_KEY_NUMPAD2: return ImGuiKey_Keypad2;
        case OCTAVE_KEY_NUMPAD3: return ImGuiKey_Keypad3;
        case OCTAVE_KEY_NUMPAD4: return ImGuiKey_Keypad4;
        case OCTAVE_KEY_NUMPAD5: return ImGuiKey_Keypad5;
        case OCTAVE_KEY_NUMPAD6: return ImGuiKey_Keypad6;
        case OCTAVE_KEY_NUMPAD7: return ImGuiKey_Keypad7;
        case OCTAVE_KEY_NUMPAD8: return ImGuiKey_Keypad8;
        case OCTAVE_KEY_NUMPAD9: return ImGuiKey_Keypad9;
        case OCTAVE_KEY_DECIMAL: return ImGuiKey_KeypadDecimal;
        //case VK_DIVIDE: return ImGuiKey_KeypadDivide;
        //case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        //case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        //case VK_ADD: return ImGuiKey_KeypadAdd;
        //case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
        case OCTAVE_KEY_SHIFT_L: return ImGuiKey_LeftShift;
        case OCTAVE_KEY_CONTROL_L: return ImGuiKey_LeftCtrl;
        case OCTAVE_KEY_ALT_L: return ImGuiKey_LeftAlt;
        //case VK_LWIN: return ImGuiKey_LeftSuper;
        case OCTAVE_KEY_SHIFT_R: return ImGuiKey_RightShift;
        case OCTAVE_KEY_CONTROL_R: return ImGuiKey_RightCtrl;
        
        // TODO: Why is ALT R the same as one of the arrow keys??
        //case OCTAVE_KEY_ALT_R: return ImGuiKey_RightAlt;
        
        //case VK_RWIN: return ImGuiKey_RightSuper;
        //case VK_APPS: return ImGuiKey_Menu;
        case OCTAVE_KEY_0: return ImGuiKey_0;
        case OCTAVE_KEY_1: return ImGuiKey_1;
        case OCTAVE_KEY_2: return ImGuiKey_2;
        case OCTAVE_KEY_3: return ImGuiKey_3;
        case OCTAVE_KEY_4: return ImGuiKey_4;
        case OCTAVE_KEY_5: return ImGuiKey_5;
        case OCTAVE_KEY_6: return ImGuiKey_6;
        case OCTAVE_KEY_7: return ImGuiKey_7;
        case OCTAVE_KEY_8: return ImGuiKey_8;
        case OCTAVE_KEY_9: return ImGuiKey_9;
        case OCTAVE_KEY_A: return ImGuiKey_A;
        case OCTAVE_KEY_B: return ImGuiKey_B;
        case OCTAVE_KEY_C: return ImGuiKey_C;
        case OCTAVE_KEY_D: return ImGuiKey_D;
        case OCTAVE_KEY_E: return ImGuiKey_E;
        case OCTAVE_KEY_F: return ImGuiKey_F;
        case OCTAVE_KEY_G: return ImGuiKey_G;
        case OCTAVE_KEY_H: return ImGuiKey_H;
        case OCTAVE_KEY_I: return ImGuiKey_I;
        case OCTAVE_KEY_J: return ImGuiKey_J;
        case OCTAVE_KEY_K: return ImGuiKey_K;
        case OCTAVE_KEY_L: return ImGuiKey_L;
        case OCTAVE_KEY_M: return ImGuiKey_M;
        case OCTAVE_KEY_N: return ImGuiKey_N;
        case OCTAVE_KEY_O: return ImGuiKey_O;
        case OCTAVE_KEY_P: return ImGuiKey_P;
        case OCTAVE_KEY_Q: return ImGuiKey_Q;
        case OCTAVE_KEY_R: return ImGuiKey_R;
        case OCTAVE_KEY_S: return ImGuiKey_S;
        case OCTAVE_KEY_T: return ImGuiKey_T;
        case OCTAVE_KEY_U: return ImGuiKey_U;
        case OCTAVE_KEY_V: return ImGuiKey_V;
        case OCTAVE_KEY_W: return ImGuiKey_W;
        case OCTAVE_KEY_X: return ImGuiKey_X;
        case OCTAVE_KEY_Y: return ImGuiKey_Y;
        case OCTAVE_KEY_Z: return ImGuiKey_Z;
        case OCTAVE_KEY_F1: return ImGuiKey_F1;
        case OCTAVE_KEY_F2: return ImGuiKey_F2;
        case OCTAVE_KEY_F3: return ImGuiKey_F3;
        case OCTAVE_KEY_F4: return ImGuiKey_F4;
        case OCTAVE_KEY_F5: return ImGuiKey_F5;
        case OCTAVE_KEY_F6: return ImGuiKey_F6;
        case OCTAVE_KEY_F7: return ImGuiKey_F7;
        case OCTAVE_KEY_F8: return ImGuiKey_F8;
        case OCTAVE_KEY_F9: return ImGuiKey_F9;
        case OCTAVE_KEY_F10: return ImGuiKey_F10;
        case OCTAVE_KEY_F11: return ImGuiKey_F11;
        case OCTAVE_KEY_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

IMGUI_IMPL_API int32_t ImGui_ImplXcb_EventHandler(xcb_generic_event_t* event)
{
    if (ImGui::GetCurrentContext() == nullptr)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplXcb_Data* bd = ImGui_ImplXcb_GetBackendData();

    switch (event->response_type & 0x7f)
    {
    case XCB_MOTION_NOTIFY:
    {
        xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*) event;
        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        io.AddMousePosEvent((float)motion->event_x, (float)motion->event_y);
        break;
    }
    case XCB_BUTTON_PRESS:
    {
        xcb_button_press_event_t* press = (xcb_button_press_event_t*) event;

        int button = -1;
        if (press->detail == XCB_BUTTON_INDEX_1) { button = 0; }
        if (press->detail == XCB_BUTTON_INDEX_3) { button = 1; }
        if (press->detail == XCB_BUTTON_INDEX_2) { button = 2; }

        if (press->detail == XCB_BUTTON_INDEX_4)
        { 
            io.AddMouseWheelEvent(0.0f, 1.0f);
        }
        if (press->detail == XCB_BUTTON_INDEX_5)
        {
            io.AddMouseWheelEvent(0.0f, -1.0f);
        }

        if (button != -1)
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(button, true);
        }
        return 0;
    }
    case XCB_BUTTON_RELEASE:
    {
        xcb_button_press_event_t* release = (xcb_button_press_event_t*) event;

        int button = -1;
        if (release->detail == XCB_BUTTON_INDEX_1) { button = 0; }
        if (release->detail == XCB_BUTTON_INDEX_3) { button = 1; }
        if (release->detail == XCB_BUTTON_INDEX_2) { button = 2; }

        if (button != -1)
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(button, false);
        }
        return 0;
    }
    case XCB_KEY_PRESS:
    {
        const xcb_key_press_event_t* keyEvent = (const xcb_key_press_event_t*)event;
        xcb_keycode_t keyCode = keyEvent->detail;
        
        // Submit key event
        const ImGuiKey key = ImGui_ImplXcb_VirtualKeyToImGuiKey(keyCode);
        const int scancode = -1; //(int)LOBYTE(HIWORD(lParam));
        if (key != ImGuiKey_None)
            ImGui_ImplXcb_AddKeyEvent(key, true, keyCode, scancode);

        uint8_t charVal = ConvertKeyCodeToChar((uint8_t)keyCode, IsShiftDown());
        if (charVal != 0)
        {
            io.AddInputCharacter(charVal);
        }

        // Win32 implementation for reference.
#if 0
        const bool is_key_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        if (wParam < 256)
        {
            // Submit modifiers
            ImGui_ImplWin32_UpdateKeyModifiers();

            // Obtain virtual key code
            // (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
            int vk = (int)wParam;
            if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
                vk = IM_VK_KEYPAD_ENTER;

            // Submit key event
            const ImGuiKey key = ImGui_ImplWin32_VirtualKeyToImGuiKey(vk);
            const int scancode = (int)LOBYTE(HIWORD(lParam));
            if (key != ImGuiKey_None)
                ImGui_ImplWin32_AddKeyEvent(key, is_key_down, vk, scancode);

            // Submit individual left/right modifier events
            if (vk == VK_SHIFT)
            {
                // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
                if (IsVkDown(VK_LSHIFT) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
                if (IsVkDown(VK_RSHIFT) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
            }
            else if (vk == VK_CONTROL)
            {
                if (IsVkDown(VK_LCONTROL) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
                if (IsVkDown(VK_RCONTROL) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
            }
            else if (vk == VK_MENU)
            {
                if (IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
                if (IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
            }
        }
#endif

        return 0;
    }
    case XCB_KEY_RELEASE:
    {
        const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*) event;

        xcb_keycode_t keyCode = keyEvent->detail;
        
        // Submit key event
        const ImGuiKey key = ImGui_ImplXcb_VirtualKeyToImGuiKey(keyCode);
        const int scancode = -1;
        if (key != ImGuiKey_None)
            ImGui_ImplXcb_AddKeyEvent(key, false, keyCode, scancode);

        return 0;
    }
    case XCB_FOCUS_IN:
        io.AddFocusEvent(true);
        return 0;
    case XCB_FOCUS_OUT:
        io.AddFocusEvent(false);
        return 0;
    }
    return 0;
}

#endif // #ifndef IMGUI_DISABLE

#endif