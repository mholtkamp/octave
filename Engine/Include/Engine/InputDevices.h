#pragma once

#include "Input/Input.h"

#include <string.h>
#include <vector>

bool IsKeyDown(int32_t key);
bool IsKeyJustDownRepeat(int32_t key);
bool IsKeyJustDown(int32_t key);
bool IsKeyJustUp(int32_t key);

bool IsControlDown();
bool IsShiftDown();
void ClearControlDown();
void ClearShiftDown();

bool IsMouseButtonDown(int32_t button);
bool IsMouseButtonJustDown(int32_t button);
bool IsMouseButtonJustUp(int32_t button);

void GetMousePosition(int32_t& mouseX, int32_t& mouseY);

int32_t GetScrollWheelDelta();
bool IsTouchDown(int32_t touch);

bool IsPointerJustUp(int32_t pointer = 0);
bool IsPointerJustDown(int32_t pointer = 0);
bool IsPointerDown(int32_t pointer = 0);

void GetTouchPosition(int32_t& touchX, int32_t& touchY, int32_t touch);
void GetTouchPositionNormalized(float& fTouchX, float& fTouchY, int32_t touch);
void GetPointerPosition(int32_t& pointerX, int32_t& pointerY, int32_t pointer = 0);
void GetPointerPositionNormalized(float& fPointerX, float& fPointerY, int32_t  pointer = 0);

bool IsGamepadButtonDown(int32_t gamepadButton, int32_t gamepadIndex);
bool IsGamepadButtonJustDown(int32_t gamepadButton, int32_t gamepadIndex);
bool IsGamepadButtonJustUp(int32_t gamepadButton, int32_t gamepadIndex);
float GetGamepadAxisValue(int32_t gamepadAxis, int32_t gamepadIndex);
GamepadType GetGamepadType(int32_t gamepadIndex);
bool IsGamepadConnected(int32_t gamepadIndex);
