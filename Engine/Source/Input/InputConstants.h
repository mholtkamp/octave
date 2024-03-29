#pragma once

#define INPUT_MAX_KEYS 256
#define INPUT_MAX_MOUSE_BUTTONS 8
#define INPUT_MAX_TOUCHES 4
#define INPUT_MAX_GAMEPADS 4
#define INPUT_MAX_GAMEPAD_BUTTONS 19
#define INPUT_MAX_GAMEPAD_AXES 6

#if PLATFORM_WINDOWS
#define INPUT_KEYBOARD_SUPPORT 1
#define INPUT_MOUSE_SUPPORT 1
#define INPUT_TOUCH_SUPPORT 1
#define INPUT_GAMEPAD_SUPPORT 1
#elif PLATFORM_LINUX
#define INPUT_KEYBOARD_SUPPORT 1
#define INPUT_MOUSE_SUPPORT 1
#define INPUT_TOUCH_SUPPORT 1
#define INPUT_GAMEPAD_SUPPORT 1
#elif PLATFORM_ANDROID
#define INPUT_KEYBOARD_SUPPORT 1
#define INPUT_MOUSE_SUPPORT 1
#define INPUT_TOUCH_SUPPORT 1
#define INPUT_GAMEPAD_SUPPORT 1
#endif