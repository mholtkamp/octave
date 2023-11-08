
#if PLATFORM_LINUX

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen/Pen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy VK_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.

#include "imgui.h"
#include "imgui_impl_xcb.h"

#include "Input.h"
#include "InputDevices.h"
#include "Engine.h"

struct ImGui_ImplXcb_Data
{
    xcb_window_t                mWindow;
    //INT64                     Time;
    //INT64                     TicksPerSecond;

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
    io.DeltaTime = GetEngineState()->mRealDeltaTime;

    // Update OS mouse position
    ImGui_ImplXcb_UpdateMouseData();
}

// Map VK_xxx to ImGuiKey_xxx.
static ImGuiKey ImGui_ImplXcb_VirtualKeyToImGuiKey(xcb_keycode_t keyCode)
{
    switch (keyCode)
    {
        case KEY_TAB: return ImGuiKey_Tab;
        case KEY_LEFT: return ImGuiKey_LeftArrow;
        case KEY_RIGHT: return ImGuiKey_RightArrow;
        case KEY_UP: return ImGuiKey_UpArrow;
        case KEY_DOWN: return ImGuiKey_DownArrow;
        case KEY_PAGE_UP: return ImGuiKey_PageUp;
        case KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case KEY_HOME: return ImGuiKey_Home;
        case KEY_END: return ImGuiKey_End;
        case KEY_INSERT: return ImGuiKey_Insert;
        case KEY_DELETE: return ImGuiKey_Delete;
        case KEY_BACKSPACE: return ImGuiKey_Backspace;
        case KEY_SPACE: return ImGuiKey_Space;
        case KEY_ENTER: return ImGuiKey_Enter;
        case KEY_ESCAPE: return ImGuiKey_Escape;
        case KEY_QUOTE: return ImGuiKey_Apostrophe;
        case KEY_COMMA: return ImGuiKey_Comma;
        case KEY_MINUS: return ImGuiKey_Minus;
        case KEY_PERIOD: return ImGuiKey_Period;
        case KEY_QUESTION: return ImGuiKey_Slash;
        case KEY_COLON: return ImGuiKey_Semicolon;
        case KEY_PLUS: return ImGuiKey_Equal;
        case KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case KEY_BACK_SLASH: return ImGuiKey_Backslash;
        case KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case KEY_SQUIGGLE: return ImGuiKey_GraveAccent;
        //case KEY_???: return ImGuiKey_CapsLock;
        //case KEY_???: return ImGuiKey_ScrollLock;
        //case KEY_???: return ImGuiKey_NumLock;
        //case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
        //case VK_PAUSE: return ImGuiKey_Pause;
        case KEY_NUMPAD0: return ImGuiKey_Keypad0;
        case KEY_NUMPAD1: return ImGuiKey_Keypad1;
        case KEY_NUMPAD2: return ImGuiKey_Keypad2;
        case KEY_NUMPAD3: return ImGuiKey_Keypad3;
        case KEY_NUMPAD4: return ImGuiKey_Keypad4;
        case KEY_NUMPAD5: return ImGuiKey_Keypad5;
        case KEY_NUMPAD6: return ImGuiKey_Keypad6;
        case KEY_NUMPAD7: return ImGuiKey_Keypad7;
        case KEY_NUMPAD8: return ImGuiKey_Keypad8;
        case KEY_NUMPAD9: return ImGuiKey_Keypad9;
        case KEY_DECIMAL: return ImGuiKey_KeypadDecimal;
        //case VK_DIVIDE: return ImGuiKey_KeypadDivide;
        //case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        //case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        //case VK_ADD: return ImGuiKey_KeypadAdd;
        //case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
        case KEY_SHIFT_L: return ImGuiKey_LeftShift;
        case KEY_CONTROL_L: return ImGuiKey_LeftCtrl;
        case KEY_ALT_L: return ImGuiKey_LeftAlt;
        //case VK_LWIN: return ImGuiKey_LeftSuper;
        case KEY_SHIFT_R: return ImGuiKey_RightShift;
        case KEY_CONTROL_R: return ImGuiKey_RightCtrl;
        
        // TODO: Why is ALT R the same as one of the arrow keys??
        //case KEY_ALT_R: return ImGuiKey_RightAlt;
        
        //case VK_RWIN: return ImGuiKey_RightSuper;
        //case VK_APPS: return ImGuiKey_Menu;
        case KEY_0: return ImGuiKey_0;
        case KEY_1: return ImGuiKey_1;
        case KEY_2: return ImGuiKey_2;
        case KEY_3: return ImGuiKey_3;
        case KEY_4: return ImGuiKey_4;
        case KEY_5: return ImGuiKey_5;
        case KEY_6: return ImGuiKey_6;
        case KEY_7: return ImGuiKey_7;
        case KEY_8: return ImGuiKey_8;
        case KEY_9: return ImGuiKey_9;
        case KEY_A: return ImGuiKey_A;
        case KEY_B: return ImGuiKey_B;
        case KEY_C: return ImGuiKey_C;
        case KEY_D: return ImGuiKey_D;
        case KEY_E: return ImGuiKey_E;
        case KEY_F: return ImGuiKey_F;
        case KEY_G: return ImGuiKey_G;
        case KEY_H: return ImGuiKey_H;
        case KEY_I: return ImGuiKey_I;
        case KEY_J: return ImGuiKey_J;
        case KEY_K: return ImGuiKey_K;
        case KEY_L: return ImGuiKey_L;
        case KEY_M: return ImGuiKey_M;
        case KEY_N: return ImGuiKey_N;
        case KEY_O: return ImGuiKey_O;
        case KEY_P: return ImGuiKey_P;
        case KEY_Q: return ImGuiKey_Q;
        case KEY_R: return ImGuiKey_R;
        case KEY_S: return ImGuiKey_S;
        case KEY_T: return ImGuiKey_T;
        case KEY_U: return ImGuiKey_U;
        case KEY_V: return ImGuiKey_V;
        case KEY_W: return ImGuiKey_W;
        case KEY_X: return ImGuiKey_X;
        case KEY_Y: return ImGuiKey_Y;
        case KEY_Z: return ImGuiKey_Z;
        case KEY_F1: return ImGuiKey_F1;
        case KEY_F2: return ImGuiKey_F2;
        case KEY_F3: return ImGuiKey_F3;
        case KEY_F4: return ImGuiKey_F4;
        case KEY_F5: return ImGuiKey_F5;
        case KEY_F6: return ImGuiKey_F6;
        case KEY_F7: return ImGuiKey_F7;
        case KEY_F8: return ImGuiKey_F8;
        case KEY_F9: return ImGuiKey_F9;
        case KEY_F10: return ImGuiKey_F10;
        case KEY_F11: return ImGuiKey_F11;
        case KEY_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}


// Win32 message handler (process Win32 mouse/keyboard inputs, etc.)
// Call from your application's message handler. Keep calling your message handler unless this function returns TRUE.
// When implementing your own backend, you can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if Dear ImGui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to Dear ImGui, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinates when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
#if 0
// Copy this line into your .cpp file to forward declare the function.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// See https://learn.microsoft.com/en-us/windows/win32/tablet/system-events-and-mouse-messages
// Prefer to call this at the top of the message handler to avoid the possibility of other Win32 calls interfering with this.
static ImGuiMouseSource GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == nullptr)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();

    switch (msg)
    {
    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
    {
        // We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
        ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
        const int area = (msg == WM_MOUSEMOVE) ? 1 : 2;
        bd->MouseHwnd = hwnd;
        if (bd->MouseTrackedArea != area)
        {
            TRACKMOUSEEVENT tme_cancel = { sizeof(tme_cancel), TME_CANCEL, hwnd, 0 };
            TRACKMOUSEEVENT tme_track = { sizeof(tme_track), (DWORD)((area == 2) ? (TME_LEAVE | TME_NONCLIENT) : TME_LEAVE), hwnd, 0 };
            if (bd->MouseTrackedArea != 0)
                ::TrackMouseEvent(&tme_cancel);
            ::TrackMouseEvent(&tme_track);
            bd->MouseTrackedArea = area;
        }
        POINT mouse_pos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        if (msg == WM_NCMOUSEMOVE && ::ScreenToClient(hwnd, &mouse_pos) == FALSE) // WM_NCMOUSEMOVE are provided in absolute coordinates.
            break;
        io.AddMouseSourceEvent(mouse_source);
        io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
        break;
    }
    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
    {
        const int area = (msg == WM_MOUSELEAVE) ? 1 : 2;
        if (bd->MouseTrackedArea == area)
        {
            if (bd->MouseHwnd == hwnd)
                bd->MouseHwnd = nullptr;
            bd->MouseTrackedArea = 0;
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
        break;
    }
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
        int button = 0;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
        if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        if (bd->MouseButtonsDown == 0 && ::GetCapture() == nullptr)
            ::SetCapture(hwnd);
        bd->MouseButtonsDown |= 1 << button;
        io.AddMouseSourceEvent(mouse_source);
        io.AddMouseButtonEvent(button, true);
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        ImGuiMouseSource mouse_source = GetMouseSourceFromMessageExtraInfo();
        int button = 0;
        if (msg == WM_LBUTTONUP) { button = 0; }
        if (msg == WM_RBUTTONUP) { button = 1; }
        if (msg == WM_MBUTTONUP) { button = 2; }
        if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        bd->MouseButtonsDown &= ~(1 << button);
        if (bd->MouseButtonsDown == 0 && ::GetCapture() == hwnd)
            ::ReleaseCapture();
        io.AddMouseSourceEvent(mouse_source);
        io.AddMouseButtonEvent(button, false);
        return 0;
    }
    case WM_MOUSEWHEEL:
        io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
        return 0;
    case WM_MOUSEHWHEEL:
        io.AddMouseWheelEvent(-(float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
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
        return 0;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        io.AddFocusEvent(msg == WM_SETFOCUS);
        return 0;
    case WM_CHAR:
        if (::IsWindowUnicode(hwnd))
        {
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000)
                io.AddInputCharacterUTF16((unsigned short)wParam);
        }
        else
        {
            wchar_t wch = 0;
            ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
            io.AddInputCharacter(wch);
        }
        return 0;
    case WM_SETCURSOR:
        // This is required to restore cursor when transitioning from e.g resize borders to client area.
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;
    case WM_DEVICECHANGE:
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
        if ((UINT)wParam == DBT_DEVNODES_CHANGED)
            bd->WantUpdateHasGamepad = true;
#endif
        return 0;
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------
// DPI-related helpers (optional)
//--------------------------------------------------------------------------------------------------------
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
//---------------------------------------------------------------------------------------------------------
// This is the scheme successfully used by GLFW (from which we borrowed some of the code) and other apps aiming to be highly portable.
// ImGui_ImplWin32_EnableDpiAwareness() is just a helper called by main.cpp, we don't call it automatically.
// If you are trying to implement your own backend for your own engine, you may ignore that noise.
//---------------------------------------------------------------------------------------------------------

// Perform our own check with RtlVerifyVersionInfo() instead of using functions from <VersionHelpers.h> as they
// require a manifest to be functional for checks above 8.1. See https://github.com/ocornut/imgui/issues/4200
static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
{
    typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
    static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = nullptr;
	if (RtlVerifyVersionInfoFn == nullptr)
		if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
			RtlVerifyVersionInfoFn = (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
    if (RtlVerifyVersionInfoFn == nullptr)
        return FALSE;

    RTL_OSVERSIONINFOEXW versionInfo = { };
    ULONGLONG conditionMask = 0;
    versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    versionInfo.dwMajorVersion = major;
	versionInfo.dwMinorVersion = minor;
	VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0) ? TRUE : FALSE;
}

#define _IsWindowsVistaOrGreater()   _IsWindowsVersionOrGreater(HIBYTE(0x0600), LOBYTE(0x0600), 0) // _WIN32_WINNT_VISTA
#define _IsWindows8OrGreater()       _IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0) // _WIN32_WINNT_WIN8
#define _IsWindows8Point1OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0) // _WIN32_WINNT_WINBLUE
#define _IsWindows10OrGreater()      _IsWindowsVersionOrGreater(HIBYTE(0x0A00), LOBYTE(0x0A00), 0) // _WIN32_WINNT_WINTHRESHOLD / _WIN32_WINNT_WIN10

#ifndef DPI_ENUMS_DECLARED
typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    (DPI_AWARENESS_CONTEXT)-3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT)-4
#endif
typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);                     // Shcore.lib + dll, Windows 8.1+
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);        // Shcore.lib + dll, Windows 8.1+
typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)

// Helper function to enable DPI awareness without setting up a manifest
void ImGui_ImplWin32_EnableDpiAwareness()
{
    if (_IsWindows10OrGreater())
    {
        static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll"); // Reference counted per-process
        if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
        {
            SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
    }
    if (_IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
        {
            SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
            return;
        }
    }
#if _WIN32_WINNT >= 0x0600
    ::SetProcessDPIAware();
#endif
}

#if defined(_MSC_VER) && !defined(NOGDI)
#pragma comment(lib, "gdi32")   // Link with gdi32.lib for GetDeviceCaps(). MinGW will require linking with '-lgdi32'
#endif

float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor)
{
    UINT xdpi = 96, ydpi = 96;
    if (_IsWindows8Point1OrGreater())
    {
		static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
		static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
		if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr)
            GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
		if (GetDpiForMonitorFn != nullptr)
		{
			GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
            IM_ASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
			return xdpi / 96.0f;
		}
    }
#ifndef NOGDI
    const HDC dc = ::GetDC(nullptr);
    xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
    ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
    IM_ASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
    ::ReleaseDC(nullptr, dc);
#endif
    return xdpi / 96.0f;
}

float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd)
{
    HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
    return ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
}

//---------------------------------------------------------------------------------------------------------
// Transparency related helpers (optional)
//--------------------------------------------------------------------------------------------------------

#if defined(_MSC_VER)
#pragma comment(lib, "dwmapi")  // Link with dwmapi.lib. MinGW will require linking with '-ldwmapi'
#endif

// [experimental]
// Borrowed from GLFW's function updateFramebufferTransparency() in src/win32_window.c
// (the Dwm* functions are Vista era functions but we are borrowing logic from GLFW)
void ImGui_ImplWin32_EnableAlphaCompositing(void* hwnd)
{
    if (!_IsWindowsVistaOrGreater())
        return;

    BOOL composition;
    if (FAILED(::DwmIsCompositionEnabled(&composition)) || !composition)
        return;

    BOOL opaque;
    DWORD color;
    if (_IsWindows8OrGreater() || (SUCCEEDED(::DwmGetColorizationColor(&color, &opaque)) && !opaque))
    {
        HRGN region = ::CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;
        ::DwmEnableBlurBehindWindow((HWND)hwnd, &bb);
        ::DeleteObject(region);
    }
    else
    {
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE;
        ::DwmEnableBlurBehindWindow((HWND)hwnd, &bb);
    }
}

//---------------------------------------------------------------------------------------------------------

#endif // #ifndef IMGUI_DISABLE

#endif