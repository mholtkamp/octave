#include "InputDevices.h"
#include "Log.h"
#include "Engine.h"
#include <string.h>
#include <ctype.h>
#include "Assertion.h"
#include <glm/glm.hpp>

#include "Input/Input.h"

bool IsKeyDown(int32_t key)
{
    return INP_IsKeyDown(key);
}

bool IsKeyJustDownRepeat(int32_t key)
{
    return INP_IsKeyJustDownRepeat(key);
}

bool IsKeyJustDown(int32_t key)
{
    return INP_IsKeyJustDown(key);
}

bool IsKeyJustUp(int32_t key)
{
    return INP_IsKeyJustUp(key);
}

bool IsControlDown()
{
    return (IsKeyDown(KEY_CONTROL_L) || IsKeyDown(KEY_CONTROL_R));
}

bool IsShiftDown()
{
    return (IsKeyDown(KEY_SHIFT_L) || IsKeyDown(KEY_SHIFT_R));
}

bool IsAltDown()
{
    return (IsKeyDown(KEY_ALT_L) || IsKeyDown(KEY_ALT_R));
}

void ClearControlDown()
{
    INP_ClearKey(KEY_CONTROL_L);
    INP_ClearKey(KEY_CONTROL_R);
}

void ClearShiftDown()
{
    INP_ClearKey(KEY_SHIFT_L);
    INP_ClearKey(KEY_SHIFT_R);
}

void ClearAltDown()
{
    INP_ClearKey(KEY_ALT_L);
    INP_ClearKey(KEY_ALT_R);
}

bool IsMouseButtonDown(int32_t button)
{
    return INP_IsMouseButtonDown(button);
}

bool IsMouseButtonJustDown(int32_t button)
{
    return INP_IsMouseButtonJustDown(button);
}

bool IsMouseButtonJustUp(int32_t button)
{
    return INP_IsMouseButtonJustUp(button);
}

void GetMousePosition(int32_t& mouseX, int32_t& mouseY)
{
    INP_GetMousePosition(mouseX, mouseY);
}

int32_t GetScrollWheelDelta()
{
    return INP_GetScrollWheelDelta();
}

bool IsTouchDown(int32_t touch)
{
    return INP_IsTouchDown(touch);
}

bool IsPointerDown(int32_t pointer)
{
    return INP_IsPointerDown(pointer);
}

bool IsPointerJustUp(int32_t pointer)
{
    return INP_IsPointerJustUp(pointer);
}

bool IsPointerJustDown(int32_t pointer)
{
    return INP_IsPointerJustDown(pointer);
}

void GetTouchPosition(int32_t& touchX,
    int32_t& touchY,
    int32_t touch)
{
    INP_GetTouchPosition(touchX, touchY, touch);
}

void GetTouchPositionNormalized(float& touchX,
    float& touchY,
    int32_t touch)
{
    INP_GetTouchPositionNormalized(touchX, touchY, touch);
}

void GetPointerPosition(int32_t& pointerX,
    int32_t& pointerY,
    int32_t pointer)
{
    INP_GetPointerPosition(pointerX, pointerY, pointer);
}

void GetPointerPositionNormalized(float& pointerX,
    float& pointerY,
    int32_t pointer)
{
    INP_GetPointerPositionNormalized(pointerX, pointerY, pointer);
}

bool IsGamepadButtonDown(int32_t gamepadButton,
    int32_t gamepadIndex)
{
    return INP_IsGamepadButtonDown(gamepadButton, gamepadIndex);
}

bool IsGamepadButtonJustDown(int32_t gamepadButton,
    int32_t gamepadIndex)
{
    return INP_IsGamepadButtonJustDown(gamepadButton, gamepadIndex);
}

bool IsGamepadButtonJustUp(int32_t gamepadButton, int32_t gamepadIndex)
{
    return INP_IsGamepadButtonJustUp(gamepadButton, gamepadIndex);
}

float GetGamepadAxisValue(int32_t gamepadAxis,
    int32_t gamepadIndex)
{
    return INP_GetGamepadAxisValue(gamepadAxis, gamepadIndex);
}

GamepadType GetGamepadType(int32_t gamepadIndex)
{
    return INP_GetGamepadType(gamepadIndex);
}

bool IsGamepadConnected(int32_t gamepadIndex)
{
    return INP_IsGamepadConnected(gamepadIndex);
}

