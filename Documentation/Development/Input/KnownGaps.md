# Known Gaps & Missing Features

This document tracks known limitations, incomplete implementations, and missing features in the Octave Engine input system.

## Wii

### No Gyroscope Data

The Wii implementation reads **orientation** (pitch/yaw/roll from `WPAD_Orientation`) and **accelerometer** data (`WPAD_Accel`), but does **not** read true gyroscope data. The `mGyro` array is never populated for Wii gamepads.

`Input.GetGamepadGyro()` returns `Vector(0, 0, 0)` on Wii.

True gyroscope data would require **MotionPlus** extension support, which is not currently implemented.

**Source**: `Engine/Source/Input/Dolphin/Input_Dolphin.cpp` - no gyro read calls present.

### Nunchuk Accelerometer Not Read

The Nunchuk has its own independent accelerometer, but only the Wiimote's accelerometer data is currently read. The Nunchuk accelerometer data is ignored.

**Source**: `Engine/Source/Input/Dolphin/Input_Dolphin.cpp` - only `WPAD_Accel()` is called (Wiimote accel), no Nunchuk accel read.

### Soft Keyboard Not Implemented

`ShowSoftKeyboard()` returns `nullptr` on Wii (no on-screen keyboard implementation).

**Source**: `Engine/Source/Input/Dolphin/Input_Dolphin.cpp:238-241`

## 3DS

### Soft Keyboard State Always False

The 3DS soft keyboard works correctly (it shows a modal dialog and returns text), but `IsSoftKeyboardShown()` always returns `false` because the keyboard is synchronous/blocking - it's never "shown" in the background.

**Source**: `Engine/Source/Input/3DS/Input_3DS.cpp:162-164`

### Key Table Conflicts

On 3DS, the names `KEY_A`, `KEY_B`, `KEY_L`, `KEY_R`, `KEY_X`, `KEY_Y`, and arrow key names (`KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`) conflict with 3DS SDK constants used for gamepad buttons. These are commented out in the `KeyCode` enum, and the Lua `Key` table sets them to `0`.

This means `Key.A`, `Key.B`, `Key.L`, `Key.R`, `Key.X`, `Key.Y`, `Key.Up`, `Key.Down`, `Key.Left`, `Key.Right` are all `0` on 3DS and cannot be used for keyboard input.

**Source**: `Engine/Source/Input/InputTypes.h:384-495` and `Engine/Source/LuaBindings/Input_Lua.cpp:611-741`

## Android

### Gamepad Button Mapping Incomplete

The Android gamepad button enum values are defined but commented out as a TODO. The values conflict with the existing `GamepadButtonCode` enum. Gamepad input on Android is non-functional.

**Source**: `Engine/Source/Input/InputTypes.h:348-382` - entire `GamepadCode` enum is in a comment block.

## Linux

### Limited Joystick Compatibility

The Linux joystick implementation has only been tested with **Xbox 360 controllers**. Button and axis mappings are hardcoded based on that specific controller layout. Other controllers may have incorrect or missing mappings.

The code includes a TODO noting this limitation.

**Source**: `Engine/Source/Input/Linux/Input_Linux.cpp:49` and `Engine/Source/Input/Linux/Input_Linux.cpp:79`

## Cross-Platform

### No Input Rebinding / Action Mapping

There is no built-in action mapping or input rebinding system. Users must implement their own mapping layer on top of the raw input functions. For example, there is no way to define "Jump = Space OR Gamepad A" as a named action.

### No Haptic Feedback / Rumble

No vibration, force feedback, or rumble API is exposed for any platform. This includes:
- Xbox controller rumble (Windows)
- Wiimote rumble (Wii)
- HD Rumble (not applicable to current platforms, but noted for completeness)

### No MotionPlus Support

Wii MotionPlus (gyroscope accessory) is not supported. This means no true angular velocity data is available on Wii - only calculated orientation from the accelerometer.

### IsAltDown Not Exposed to Lua

While `IsControlDown()` and `IsShiftDown()` are exposed to Lua, `IsAltDown()` is only available in C++ (`InputDevices.h`). It is not registered in the Lua binding table.

**Source**: `Engine/Source/LuaBindings/Input_Lua.cpp:452-553` - no `IsAltDown` registration.
