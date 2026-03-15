# Gamepad & Controller Input

Gamepad input is available on **all platforms**. Up to 4 controllers can be connected simultaneously.

## Button Reference

All button constants are on the global `Gamepad` table in Lua, or the `GAMEPAD_` enum in C++.

### Face Buttons

| Lua          | C++          | Notes                              |
|--------------|--------------|------------------------------------|
| `Gamepad.A`  | `GAMEPAD_A`  | Bottom face button                 |
| `Gamepad.B`  | `GAMEPAD_B`  | Right face button                  |
| `Gamepad.C`  | `GAMEPAD_C`  | Nunchuk C button (Wii only)        |
| `Gamepad.X`  | `GAMEPAD_X`  | Left face button                   |
| `Gamepad.Y`  | `GAMEPAD_Y`  | Top face button                    |
| `Gamepad.Z`  | `GAMEPAD_Z`  | Z button (GameCube/Nunchuk)        |

### Shoulder & Trigger Buttons

| Lua          | C++              | Notes                          |
|--------------|------------------|--------------------------------|
| `Gamepad.L1` | `GAMEPAD_L1`     | Left shoulder / bumper         |
| `Gamepad.R1` | `GAMEPAD_R1`     | Right shoulder / bumper        |
| `Gamepad.L2` | `GAMEPAD_L2`     | Left trigger (as button)       |
| `Gamepad.R2` | `GAMEPAD_R2`     | Right trigger (as button)      |
| `Gamepad.L3` | `GAMEPAD_THUMBL` | Left stick click               |
| `Gamepad.R3` | `GAMEPAD_THUMBR` | Right stick click              |

### System Buttons

| Lua              | C++              |
|------------------|------------------|
| `Gamepad.Start`  | `GAMEPAD_START`  |
| `Gamepad.Select` | `GAMEPAD_SELECT` |
| `Gamepad.Home`   | `GAMEPAD_HOME`   |

### D-Pad

| Lua             | C++             |
|-----------------|-----------------|
| `Gamepad.Up`    | `GAMEPAD_UP`    |
| `Gamepad.Down`  | `GAMEPAD_DOWN`  |
| `Gamepad.Left`  | `GAMEPAD_LEFT`  |
| `Gamepad.Right` | `GAMEPAD_RIGHT` |

### Analog Stick as Buttons

When an analog stick axis exceeds a threshold of **0.5** in any direction, it automatically generates a digital button press. This lets you use stick directions in button-check functions.

| Lua               | C++              | Stick   | Direction |
|--------------------|------------------|---------|-----------|
| `Gamepad.LsLeft`  | `GAMEPAD_L_LEFT` | Left    | Left      |
| `Gamepad.LsRight` | `GAMEPAD_L_RIGHT`| Left    | Right     |
| `Gamepad.LsUp`    | `GAMEPAD_L_UP`   | Left    | Up        |
| `Gamepad.LsDown`  | `GAMEPAD_L_DOWN` | Left    | Down      |
| `Gamepad.RsLeft`  | `GAMEPAD_R_LEFT` | Right   | Left      |
| `Gamepad.RsRight` | `GAMEPAD_R_RIGHT`| Right   | Right     |
| `Gamepad.RsUp`    | `GAMEPAD_R_UP`   | Right   | Up        |
| `Gamepad.RsDown`  | `GAMEPAD_R_DOWN` | Right   | Down      |

## Axis Reference

Axes return **float** values. Stick axes range from **-1.0 to 1.0**. Trigger axes range from **0.0 to 1.0**.

| Lua              | C++                      | Range       | Description       |
|------------------|--------------------------|-------------|-------------------|
| `Gamepad.AxisL`  | `GAMEPAD_AXIS_LTRIGGER`  | 0.0 to 1.0  | Left trigger      |
| `Gamepad.AxisR`  | `GAMEPAD_AXIS_RTRIGGER`  | 0.0 to 1.0  | Right trigger     |
| `Gamepad.AxisLX` | `GAMEPAD_AXIS_LTHUMB_X`  | -1.0 to 1.0 | Left stick X      |
| `Gamepad.AxisLY` | `GAMEPAD_AXIS_LTHUMB_Y`  | -1.0 to 1.0 | Left stick Y      |
| `Gamepad.AxisRX` | `GAMEPAD_AXIS_RTHUMB_X`  | -1.0 to 1.0 | Right stick X     |
| `Gamepad.AxisRY` | `GAMEPAD_AXIS_RTHUMB_Y`  | -1.0 to 1.0 | Right stick Y     |

Positive Y is up, negative Y is down. Positive X is right, negative X is left.

## Button State Functions

### Lua

```lua
-- Is button currently held? (second argument is gamepad index, 1-4, defaults to 1)
if Input.IsGamepadDown(Gamepad.A, 1) then
    accelerate()
end

-- Was button pressed this frame?
if Input.IsGamepadPressed(Gamepad.A) then
    jump()
end

-- Was button released this frame?
if Input.IsGamepadReleased(Gamepad.B) then
    stopAction()
end
```

**Function aliases:**
- `IsGamepadDown` = `IsGamepadButtonDown`
- `IsGamepadPressed` = `IsGamepadButtonJustDown`
- `IsGamepadReleased` = `IsGamepadButtonJustUp`

The gamepad index parameter is optional and defaults to **1** (first controller) in Lua.

### C++

```cpp
#include "InputDevices.h"

// C++ uses 0-based index
bool down     = IsGamepadButtonDown(GAMEPAD_A, 0);
bool pressed  = IsGamepadButtonJustDown(GAMEPAD_A, 0);
bool released = IsGamepadButtonJustUp(GAMEPAD_B, 0);
```

## Axis Reading

### Lua

```lua
-- Read left stick X axis (gamepad 1)
local lx = Input.GetGamepadAxis(Gamepad.AxisLX, 1)
local ly = Input.GetGamepadAxis(Gamepad.AxisLY, 1)

-- Read triggers
local lt = Input.GetGamepadAxis(Gamepad.AxisL, 1)
local rt = Input.GetGamepadAxis(Gamepad.AxisR, 1)
```

**Function aliases:**
- `GetGamepadAxis` = `GetGamepadAxisValue`

### C++

```cpp
float lx = GetGamepadAxisValue(GAMEPAD_AXIS_LTHUMB_X, 0);
float ly = GetGamepadAxisValue(GAMEPAD_AXIS_LTHUMB_Y, 0);
float lt = GetGamepadAxisValue(GAMEPAD_AXIS_LTRIGGER, 0);
```

## Connection & Type Detection

### IsGamepadConnected(index)

```lua
if Input.IsGamepadConnected(1) then
    print("Gamepad 1 is connected")
end
```

### GetGamepadType(index)

Returns a string identifying the controller type:

| Return Value   | Controller                    |
|----------------|-------------------------------|
| `"Standard"`   | Generic / XInput controller   |
| `"GameCube"`   | GameCube controller           |
| `"Wiimote"`    | Wii Remote (+ Nunchuk)        |
| `"WiiClassic"` | Wii Classic Controller        |

```lua
local padType = Input.GetGamepadType(1)
if padType == "Wiimote" then
    showWiimoteControls()
elseif padType == "GameCube" then
    showGCControls()
end
```

On Wii, the type auto-detects based on which buttons have been pressed. If a Classic Controller is plugged in and buttons are pressed on it, the type switches to `"WiiClassic"`. If a GameCube controller is used on the same port, it switches to `"GameCube"`.

**C++:**
```cpp
GamepadType type = GetGamepadType(0);
bool connected = IsGamepadConnected(0);
```

## Deadzone Handling

Analog sticks rarely rest at exactly 0.0. Apply a deadzone to prevent drift:

```lua
local deadzone = 0.1
local lx = Input.GetGamepadAxis(Gamepad.AxisLX)
local ly = Input.GetGamepadAxis(Gamepad.AxisLY)

if math.abs(lx) < deadzone then lx = 0.0 end
if math.abs(ly) < deadzone then ly = 0.0 end

-- Use lx, ly for movement
```

This pattern is used in `FirstPersonController.lua`:

```lua
local leftAxisX = Input.GetGamepadAxis(Gamepad.AxisLX)
local leftAxisY = Input.GetGamepadAxis(Gamepad.AxisLY)

if math.abs(leftAxisX) > 0.1 then
    moveDir.x = moveDir.x + leftAxisX
end
if math.abs(leftAxisY) > 0.1 then
    moveDir.z = moveDir.z - leftAxisY
end
```

## Multi-Controller Support

Up to 4 gamepads are supported. In Lua, pass the gamepad index (1-4) as the second argument. In C++, use index 0-3.

```lua
-- Player 1 on gamepad 1, Player 2 on gamepad 2
local p1Jump = Input.IsGamepadPressed(Gamepad.A, 1)
local p2Jump = Input.IsGamepadPressed(Gamepad.A, 2)
```

## Platform Notes

- **Windows**: Uses XInput. Xbox controllers work natively. Up to 4 pads.
- **Linux**: Uses `/dev/input/js*` joystick API. Only tested with Xbox 360 controllers. Up to 4 pads.
- **GameCube**: GameCube controller only. 4 controller ports. `GetGamepadType` returns `"GameCube"`.
- **Wii**: Supports Wiimote, Wiimote + Nunchuk, Classic Controller, and GameCube controllers. See [Platform-Specific Input](PlatformSpecific.md) for details.
- **3DS**: Built-in controls are always connected on gamepad port 1 (Lua) / index 0 (C++). See [Platform-Specific Input](PlatformSpecific.md) for details.
- **Android**: Gamepad button mapping is incomplete (commented out as TODO in source). See [Known Gaps](KnownGaps.md).
