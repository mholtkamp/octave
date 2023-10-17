#pragma once

#include "InputConstants.h"
#include "InputTypes.h"

#include <stdint.h>

// Platform Specific
void INP_Initialize();
void INP_Shutdown();
void INP_Update();
void INP_SetCursorPos(int32_t x, int32_t y);
void INP_ShowCursor(bool show);
void INP_LockCursor(bool lock);
void INP_TrapCursor(bool trap);
void INP_ShowSoftKeyboard(bool show);
bool INP_IsSoftKeyboardShown();

// Platform Agnostic
void INP_SetKey(int32_t key);
void INP_ClearKey(int32_t key);
void INP_ClearAllKeys();
bool INP_IsKeyDown(int32_t key);
bool INP_IsKeyJustDownRepeat(int32_t key);
bool INP_IsKeyJustDown(int32_t key);
bool INP_IsKeyJustUp(int32_t key);
char INP_ConvertKeyCodeToChar(int32_t key);

void INP_SetMouseButton(int32_t button);
void INP_ClearMouseButton(int32_t button);
void INP_SetScrollWheelDelta(int32_t delta);
bool INP_IsMouseButtonDown(int32_t button);
bool INP_IsMouseButtonJustDown(int32_t button);
bool INP_IsMouseButtonJustUp(int32_t button);
int32_t INP_GetScrollWheelDelta();
void INP_ClearAllMouseButtons();

void INP_SetTouch(int32_t touch);
void INP_ClearTouch(int32_t touch);
bool INP_IsTouchDown(int32_t touch);
bool INP_IsPointerJustUp(int32_t pointer = 0);
bool INP_IsPointerJustDown(int32_t pointer = 0);
bool INP_IsPointerDown(int32_t pointer = 0);

void INP_SetMousePosition(int32_t mouseX, int32_t mouseY);
void INP_SetTouchPosition(int32_t touchX, int32_t touchY, int32_t touch = 0);
void INP_GetMousePosition(int32_t& mouseX, int32_t& mouseY);
void INP_GetTouchPosition(int32_t& touchX, int32_t& touchY, int32_t touch);
void INP_GetTouchPositionNormalized(float& touchX, float& touchY, int32_t touch);
void INP_GetPointerPosition(int32_t& pointerX, int32_t& pointerY, int32_t pointer = 0);
void INP_GetPointerPositionNormalized(float& pointerX, float& pointerY, int32_t  pointer = 0);
void INP_GetMouseDelta(int32_t& deltaX, int32_t& deltaY);

bool INP_IsGamepadButtonDown(int32_t gamepadButton, int32_t gamepadIndex);
bool INP_IsGamepadButtonJustDown(int32_t gamepadButton, int32_t gamepadIndex);
bool INP_IsGamepadButtonJustUp(int32_t gamepadButton, int32_t gamepadIndex);
float INP_GetGamepadAxisValue(int32_t gamepadAxis, int32_t gamepadIndex);
GamepadType INP_GetGamepadType(int32_t gamepadIndex);
bool INP_IsGamepadConnected(int32_t gamepadIndex);
int32_t INP_GetGamepadIndex(int32_t inputDevice);
void INP_SetGamepadAxisValue(GamepadAxisCode axisCode, float axisValue, int32_t gamepadIndex);
void INP_SetGamepadButton(GamepadButtonCode buttonCode, int32_t gamepadIndex);
void INP_ClearGamepadButton(GamepadButtonCode buttonCode, int32_t gamepadIndex);

bool INP_IsCursorLocked();
bool INP_IsCursorTrapped();
bool INP_IsCursorShown();
