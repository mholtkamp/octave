# Input

System for querying and controlling input devices.

---
### IsKeyDown
Check if a keyboard key is down.

See [Key](../Misc/Enums.md#key)

Sig: `down = Input.IsKeyDown(key)`
 - Arg: `Key(integer) key` Key to check
 - Ret: `boolean down` Key is held down
---
### IsKeyPressed
Check if a keyboard key was just pressed down this frame.

See [Key](../Misc/Enums.md#key)

Sig: `pressed = Input.IsKeyPressed(key)`
 - Arg: `Key(integer) key` Key to check
 - Ret: `boolean pressed` Key was just pressed down
---
### IsKeyPressedRepeat
Check if a keyboard key was pressed down, allowing for repeated pressing as if holding down a key in a text field.

See [Key](../Misc/Enums.md#key)

Sig: `pressed = Input.IsKeyPressedRepeat(key)`
 - Arg: `Key(integer) key` Key to check
 - Ret: `boolean pressed` Key was just pressed down
---
### IsKeyReleased
Check if a keyboard key was just released this frame.

See [Key](../Misc/Enums.md#key)

Sig: `released = Input.IsKeyReleased(key)`
 - Arg: `Key(integer) key` Key to check
 - Ret: `boolean released` Key was just released
---
### IsControlDown
Check if either the left or right control button is down.

Sig: `down = Input.IsControlDown()`
 - Ret: `boolean down` Control button is held down
---
### IsShiftDown
Check if either the left or right shift button is down.

Sig: `down = Input.IsShiftDown()`
 - Ret: `boolean down` Shift button is held down
---
### IsMouseDown
Check if a mouse button is held down.

See [Mouse](../Misc/Enums.md#mouse)

Sig: `down = Input.IsMouseDown(button)`
 - Arg: `Mouse(integer) button` Mouse button to check
 - Ret: `boolean down` Is button down
---
### IsMousePressed
Check if a mouse button was just pressed this frame.

See [Mouse](../Misc/Enums.md#mouse)

Sig: `pressed = Input.IsMousePressed(button)`
 - Arg: `Mouse(integer) button` Mouse button to check
 - Ret: `boolean pressed` Is button pressed
---
### IsMouseReleased
Check if a mouse button was just released this frame.

See [Mouse](../Misc/Enums.md#mouse)

Sig: `released = Input.IsMouseReleased(button)`
 - Arg: `Mouse(integer) button` Mouse button to check
 - Ret: `boolean released` Is button released
---
### GetMousePosition
Get the mouse position in screen space (pixels).

Sig: `x, y = Input.GetMousePosition()`
 - Ret: `integer x` Mouse position x
 - Ret: `integer y` Mouse position y
---
### GetScrollWheelDelta
Get the change scroll wheel position from the previous frame.

Sig: `delta = Input.GetScrollWheelDelta()`
 - Ret: `integer delta` Scroll wheel delta
---
### GetMouseDelta
Get the change in mouse position from last frame to this frame in screen space (pixels).

Sig: `deltaX, deltaY = Input.GetMouseDelta()`
 - Ret: `integer deltaX` Mouse position delta x
 - Ret: `integer deltaY` Mouse position delta y
---
### IsTouchDown
Check if a touch (finger/stylus) is held down.

Sig: `down = Input.IsTouchDown(index)`
 - Arg: `integer index` Touch index
 - Ret: `boolean down` Is touch down
---
### IsPointerReleased
Check if a pointer (mouse/finger/stylus) is released.

Sig: `released = Input.IsPointerReleased(index=1)`
 - Arg: `integer index` Pointer index
 - Ret: `boolean released` Is pointer released
---
### IsPointerPressed
Check if a pointer (mouse/finger/stylus) is pressed.

Sig: `pressed = Input.IsPointerPressed(index=1)`
 - Arg: `integer index` Pointer index
 - Ret: `boolean pressed` Is pointer pressed
---
### IsPointerDown
Check if a pointer (mouse/finger/stylus) is held down.

Sig: `down = Input.IsPointerDown(index=1)`
 - Arg: `integer index` Pointer index
 - Ret: `boolean down` Is pointer down
---
### GetTouchPosition
Get a touch position in screen space.

Sig: `x, y = Input.GetTouchPosition(index=1)`
 - Arg: `integer index` Touch index
 - Ret: `integer x` X position
 - Ret: `integer y` Y position
---
### GetTouchPositionNormalized
Get a touch position in normalized screen space (0 to 1 instead of pixels).

Sig: `x, y = Input.GetTouchPositionNormalized(index=1)`
 - Arg: `integer index` Touch index
 - Ret: `number x` Normalized x position (0 - 1)
 - Ret: `number y` Normalized y position (0 - 1)
---
### GetPointerPosition
Get a pointer position in screen space.

Sig: `x, y = Input.GetPointerPosition(index=1)`
 - Arg: `integer index` Pointer index
 - Ret: `integer x` X position
 - Ret: `integer y` Y position
---
### GetPointerPositionNormalized
Get a pointer position in normalized screen space (0 to 1 instead of pixels).

Sig: `x, y = Input.GetPointerPositionNormalized(index=1)`
 - Arg: `integer index` Pointer index
 - Ret: `number x` Normalized x position (0 - 1)
 - Ret: `number y` Normalized y position (0 - 1)
---
### IsGamepadDown
Check if a gamepad button is held down.

See [Gamepad](../Misc/Enums.md#gamepad)

Sig: `down = Input.IsGamepadDown(button, index=1)`
 - Arg: `Gamepad(integer) button` Gamepad button
 - Arg: `integer index` Gamepad index/port
 - Ret: `boolean down` Is button down
---
### IsGamepadPressed
Check if a gamepad button is pressed.

See [Gamepad](../Misc/Enums.md#gamepad)

Sig: `pressed = Input.IsGamepadPressed(button, index=1)`
 - Arg: `Gamepad(integer) button` Gamepad button
 - Arg: `integer index` Gamepad index/port
 - Ret: `boolean pressed` Is button pressed
---
### IsGamepadReleased
Check if a gamepad button is released.

See [Gamepad](../Misc/Enums.md#gamepad)

Sig: `released = Input.IsGamepadReleased(button, index=1)`
 - Arg: `Gamepad(integer) button` Gamepad button
 - Arg: `integer index` Gamepad index/port
 - Ret: `boolean released` Is button released
---
### GetGamepadAxis
Get a gamepad axis value (for instance: left stick X).

See [Gamepad](../Misc/Enums.md#gamepad)
Axis IDs are at the bottom.

Sig: `value = Input.GetGamepadAxis(axis, index=1)`
 - Arg: `Gamepad(integer) axis` Gamepad axis
 - Arg: `integer index` Gamepad index/port
 - Ret: `number value` Axis value (0 to 1)
---
### GetGamepadType
Check what type of gamepad is connected. This is really only useful on Wii where you might have a Wiimote, Wii classic, or GameCube controller.

Sig: `type = Input.GetGamepadType(index=1)`
 - Arg: `integer index` Gamepad index/port
 - Ret: `string type` The controller type
---
### IsGamepadConnected
Check if a gamepad is connected on a given port.

Sig: `connected = Input.IsGamepadConnected(index=1)`
 - Arg: `integer index` Gamepad index/port
 - Ret: `boolean connected` Is gamepad connected
---
### ShowCursor
Show/hide the mouse cursor.

Sig: `Input.ShowCursor(show)`
 - Arg: `boolean show` Whether to show cursor
---
### SetCursorPosition
Set the mouse cursor position.

Sig: `Input.SetCursorPosition(x, y)`
 - Arg: `integer x` X position in screen space
 - Arg: `integer y` Y position in screen space
---
### GetKeysPressed
Get a list of keys that where just pressed this frame.

See [Key](../Misc/Enums.md#key)

Sig: `pressedKeys = Input.GetKeysPressed()`
 - Ret: `table pressedKeys` An array of pressed keys
---
### IsAnyKeyPressed
Check if any key was just pressed this frame.

Sig: `pressed = Input.IsAnyKeyPressed()`
 - Ret: `boolean pressed` Was any key pressed
---
### ConvertKeyCodeToChar
Convert a Key code/ID to an ascii character returned as a string.

See [Key](../Misc/Enums.md#key)

Sig: `char = Input.ConvertKeyCodeToChar(code)`
 - Arg: `Key(integer) code` Key ID to convert
 - Ret: `string char` The converted character as a string
---
### ShowSoftKeyboard
Show/hide the software keyboard if one is available for the platform. Currently, only Android soft keyboard is supported.
TODO: Support for 3DS and Wii

Sig: `Input.ShowSoftKeyboard(show)`
 - Arg: `boolean show` Whether to show soft keyboard
---
### IsSoftKeyboardShown
Check if the software keyboard is currently shown.

Sig: `shown = Input.IsSoftKeyboardShown()`
 - Ret: `boolean shown` Whether the software keyboard is shown
---
