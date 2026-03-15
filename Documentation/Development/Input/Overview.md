# Input System Overview

The Octave Engine input system provides a unified API for reading player input across all supported platforms. It covers keyboard, mouse, touch, gamepad, and motion sensors. Both **Lua** and **C++** interfaces are available.

## Platform Support Matrix

| Feature       | Windows | Linux | GameCube | Wii | 3DS | Android |
|---------------|---------|-------|----------|-----|-----|---------|
| Keyboard      | Yes     | Yes   | No       | No  | No  | Yes     |
| Mouse         | Yes     | Yes   | No       | Yes*| No  | Yes     |
| Touch/Pointer | Yes     | Yes   | No       | Yes*| Yes | Yes     |
| Gamepad       | Yes     | Yes   | Yes      | Yes | Yes | Yes**   |
| Accelerometer | No      | No    | No       | Yes | Yes | No      |
| Gyroscope     | No      | No    | No       | No  | Yes | No      |
| Orientation   | No      | No    | No       | Yes | No  | No      |

\* Wii IR pointer is exposed through the mouse/touch APIs.
\** Android gamepad support exists but button mapping is incomplete.

## Key Concepts

### Polling-Based Input

Input in Octave is **polling-based**. You check input state each frame (typically in your `Tick` function). There are no event callbacks.

### State Checks

Every input type supports three state checks:

- **Down** - Is the button/key currently held down this frame?
- **Pressed / Just Down** - Was the button/key pressed this exact frame (was up last frame, down now)?
- **Released / Just Up** - Was the button/key released this exact frame (was down last frame, up now)?

```lua
-- "Down" = held continuously
if Input.IsKeyDown(Key.W) then
    -- Runs every frame the key is held
end

-- "Pressed" = single frame on press
if Input.IsKeyPressed(Key.Space) then
    -- Runs only the frame the key first goes down
end

-- "Released" = single frame on release
if Input.IsKeyReleased(Key.Space) then
    -- Runs only the frame the key is let go
end
```

### Lua vs C++ Index Convention

**Lua indices are 1-based.** Gamepad port 1 is the first controller, touch index 1 is the first touch point.

**C++ indices are 0-based.** Gamepad index 0 is the first controller, touch index 0 is the first touch point.

The Lua bindings handle this conversion automatically.

## Lua API

All input functions live on the global `Input` table. Constants are on separate global tables:

| Table      | Purpose                              |
|------------|--------------------------------------|
| `Input`    | All input functions                  |
| `Key`      | Keyboard key code constants          |
| `Mouse`    | Mouse button constants               |
| `Gamepad`  | Gamepad button and axis constants    |

```lua
-- Example: Check if the A key is pressed
if Input.IsKeyPressed(Key.A) then
    print("A was pressed!")
end

-- Example: Check if left mouse button is down
if Input.IsMouseDown(Mouse.Left) then
    print("Shooting!")
end

-- Example: Read left stick X axis on gamepad 1
local axisX = Input.GetGamepadAxis(Gamepad.AxisLX, 1)
```

## C++ API

There are two levels of C++ API:

### High-Level (InputDevices.h)

```cpp
#include "InputDevices.h"

// These mirror the Lua API closely
bool down = IsKeyDown(KEY_W);
bool pressed = IsKeyJustDown(KEY_SPACE);
bool released = IsKeyJustUp(KEY_SPACE);

bool ctrl = IsControlDown();
bool shift = IsShiftDown();

bool mouseDown = IsMouseButtonDown(MOUSE_LEFT);
bool padDown = IsGamepadButtonDown(GAMEPAD_A, 0);
float axis = GetGamepadAxisValue(GAMEPAD_AXIS_LTHUMB_X, 0);
```

### Low-Level (Input/Input.h)

```cpp
#include "Input/Input.h"

// INP_ prefixed functions, same functionality
bool down = INP_IsKeyDown(KEY_W);
bool pressed = INP_IsKeyJustDown(KEY_SPACE);
INP_ShowCursor(false);
INP_LockCursor(true);
```

## Limits

| Constant            | Value | Description                  |
|---------------------|-------|------------------------------|
| `INPUT_MAX_KEYS`    | 256   | Maximum tracked key codes    |
| `INPUT_MAX_TOUCHES` | 4     | Maximum simultaneous touches |
| `INPUT_MAX_GAMEPADS`| 4     | Maximum connected gamepads   |

## Further Reading

- [Keyboard Input](Keyboard.md)
- [Mouse Input](Mouse.md)
- [Gamepad & Controller Input](Gamepad.md)
- [Touch & Pointer Input](Touch.md)
- [Platform-Specific Input](PlatformSpecific.md)
- [Known Gaps & Missing Features](KnownGaps.md)
