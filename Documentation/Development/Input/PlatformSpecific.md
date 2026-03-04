# Platform-Specific Input

This document covers input details unique to each platform.

## GameCube

### Controller Layout

The GameCube controller has:
- **Face buttons**: A, B, X, Y
- **Shoulder buttons**: L (digital + analog trigger), R (digital + analog trigger), Z
- **Control Stick**: Left analog stick
- **C-Stick**: Right analog stick
- **D-Pad**: Up, Down, Left, Right
- **Start**: Start button (no Select or Home)

### Button Mapping

| Physical Button | Gamepad Constant |
|-----------------|------------------|
| A               | `Gamepad.A`      |
| B               | `Gamepad.B`      |
| X               | `Gamepad.X`      |
| Y               | `Gamepad.Y`      |
| Z               | `Gamepad.Z`      |
| L (digital)     | `Gamepad.L1`     |
| R (digital)     | `Gamepad.R1`     |
| Start           | `Gamepad.Start`  |
| D-Pad Up        | `Gamepad.Up`     |
| D-Pad Down      | `Gamepad.Down`   |
| D-Pad Left      | `Gamepad.Left`   |
| D-Pad Right     | `Gamepad.Right`  |

### Axis Mapping

| Physical Input   | Axis Constant     | Range        |
|------------------|-------------------|--------------|
| Control Stick X  | `Gamepad.AxisLX`  | -1.0 to 1.0 |
| Control Stick Y  | `Gamepad.AxisLY`  | -1.0 to 1.0 |
| C-Stick X        | `Gamepad.AxisRX`  | -1.0 to 1.0 |
| C-Stick Y        | `Gamepad.AxisRY`  | -1.0 to 1.0 |
| L Trigger        | `Gamepad.AxisL`   | 0.0 to 1.0  |
| R Trigger        | `Gamepad.AxisR`   | 0.0 to 1.0  |

Analog sticks are normalized by dividing by 127.0. Triggers are normalized by dividing by 255.0.

### Unsupported Features

- No keyboard, mouse, or touch input
- No Home button (`Gamepad.Home` always reads 0)
- No motion sensors
- `GetGamepadType()` returns `"GameCube"`

---

## Wii

The Wii supports three controller types through the same gamepad API. The active type is auto-detected based on button activity.

### Wiimote (+ Nunchuk)

#### Button Mapping

| Physical Button    | Gamepad Constant  |
|--------------------|-------------------|
| A                  | `Gamepad.A`       |
| B (trigger)        | `Gamepad.B`       |
| 1                  | `Gamepad.X`       |
| 2                  | `Gamepad.Y`       |
| + (Plus)           | `Gamepad.Start`   |
| - (Minus)          | `Gamepad.Select`  |
| Home               | `Gamepad.Home`    |
| D-Pad Up           | `Gamepad.Up`      |
| D-Pad Down         | `Gamepad.Down`    |
| D-Pad Left         | `Gamepad.Left`    |
| D-Pad Right        | `Gamepad.Right`   |
| Nunchuk Z          | `Gamepad.Z`       |
| Nunchuk C          | `Gamepad.C`       |

#### Nunchuk Analog Stick

When a Nunchuk is connected, its analog stick maps to the left stick axes:

| Physical Input     | Axis Constant     |
|--------------------|-------------------|
| Nunchuk Stick X    | `Gamepad.AxisLX`  |
| Nunchuk Stick Y    | `Gamepad.AxisLY`  |

The Nunchuk stick is normalized by dividing by 127.0.

#### IR Pointer

The Wiimote IR pointer is exposed through the **touch** API. Each Wiimote's IR data is set as a separate touch index:

- Wiimote 0 (first controller) -> touch index 0 (Lua: `Input.GetTouchPosition(1)`)
- Wiimote 1 -> touch index 1 (Lua: `Input.GetTouchPosition(2)`)
- Wiimote 2 -> touch index 2
- Wiimote 3 -> touch index 3

The IR resolution is set to **640x480** pixels.

```lua
-- Read IR pointer position for first Wiimote
local irX, irY = Input.GetTouchPosition(1)
```

> **Note**: The IR `valid` flag is not checked in the current implementation. If the IR sensor loses tracking, stale position data may be returned. See [Known Gaps](KnownGaps.md).

#### Orientation

The Wiimote provides orientation data derived from its accelerometer:

```lua
-- Returns a Vector with (pitch, yaw, roll) in degrees
local orient = Input.GetGamepadOrientation(1)
print("Pitch: " .. orient.x)
print("Yaw: " .. orient.y)
print("Roll: " .. orient.z)
```

Note: Yaw is negated from the raw `WPAD_Orientation` value.

**C++:**
```cpp
float pitch, yaw, roll;
INP_GetGamepadOrientation(pitch, yaw, roll, 0);
```

#### Accelerometer

Raw accelerometer data from the Wiimote:

```lua
-- Returns a Vector with raw (x, y, z) accelerometer values
local accel = Input.GetGamepadAcceleration(1)
```

The values are raw (`WPAD_Accel` data divided by 1.0, i.e., unscaled).

**C++:**
```cpp
float ax, ay, az;
INP_GetGamepadAcceleration(ax, ay, az, 0);
```

#### Gyroscope

**Not available on Wii.** `GetGamepadGyro()` returns zeros. The Wii implementation does not read gyroscope data. True gyroscope would require MotionPlus support, which is not implemented. See [Known Gaps](KnownGaps.md).

`GetGamepadType()` returns `"Wiimote"`.

### Wii Classic Controller

#### Button Mapping

| Physical Button   | Gamepad Constant  |
|-------------------|-------------------|
| A                 | `Gamepad.A`       |
| B                 | `Gamepad.B`       |
| X                 | `Gamepad.X`       |
| Y                 | `Gamepad.Y`       |
| ZL                | `Gamepad.Z`       |
| L (full press)    | `Gamepad.L1`      |
| R (full press)    | `Gamepad.R1`      |
| + (Plus)          | `Gamepad.Start`   |
| - (Minus)         | `Gamepad.Select`  |
| Home              | `Gamepad.Home`    |
| D-Pad Up          | `Gamepad.Up`      |
| D-Pad Down        | `Gamepad.Down`    |
| D-Pad Left        | `Gamepad.Left`    |
| D-Pad Right       | `Gamepad.Right`   |

#### Axis Mapping

| Physical Input     | Axis Constant     | Normalization |
|--------------------|-------------------|---------------|
| Left Stick X       | `Gamepad.AxisLX`  | / 31.0        |
| Left Stick Y       | `Gamepad.AxisLY`  | / 31.0        |
| Right Stick X      | `Gamepad.AxisRX`  | / 15.0        |
| Right Stick Y      | `Gamepad.AxisRY`  | / 15.0        |
| L Trigger          | `Gamepad.AxisL`   | Raw shoulder value |
| R Trigger          | `Gamepad.AxisR`   | Raw shoulder value |

`GetGamepadType()` returns `"WiiClassic"`.

### GameCube Controller on Wii

GameCube controllers can be plugged into the Wii. When a GameCube pad button is pressed, the gamepad type for that port switches to `"GameCube"`. The button and axis mappings are the same as the [GameCube section](#gamecube) above.

### Controller Type Detection

On Wii, the controller type for each port is determined dynamically:

1. All ports start as `Wiimote` type.
2. If Classic Controller buttons are pressed, the port switches to `WiiClassic`.
3. If GameCube pad buttons are pressed, the port switches to `GameCube`.
4. The type persists until a different controller type's buttons are pressed.

```lua
local padType = Input.GetGamepadType(1)
-- Returns "Wiimote", "WiiClassic", or "GameCube"
```

### Soft Keyboard

Not available on Wii. `ShowSoftKeyboard()` returns `nil`.

---

## 3DS

### Built-in Controls

The 3DS has built-in controls that are always connected on gamepad port **1** (Lua) / index **0** (C++).

#### Button Mapping

| Physical Button | Gamepad Constant  | Notes               |
|-----------------|-------------------|---------------------|
| A               | `Gamepad.A`       |                     |
| B               | `Gamepad.B`       |                     |
| X               | `Gamepad.X`       |                     |
| Y               | `Gamepad.Y`       |                     |
| L               | `Gamepad.L1`      |                     |
| R               | `Gamepad.R1`      |                     |
| ZL              | `Gamepad.L2`      | New 3DS only        |
| ZR              | `Gamepad.R2`      | New 3DS only        |
| Start           | `Gamepad.Start`   |                     |
| Select          | `Gamepad.Select`  |                     |
| D-Pad Up        | `Gamepad.Up`      |                     |
| D-Pad Down      | `Gamepad.Down`    |                     |
| D-Pad Left      | `Gamepad.Left`    |                     |
| D-Pad Right     | `Gamepad.Right`   |                     |

Note: `Gamepad.Z` is always 0 on 3DS (no Z button).

#### Axis Mapping

| Physical Input | Axis Constant     | Range        | Normalization | Notes           |
|----------------|-------------------|--------------|---------------|-----------------|
| Circle Pad X   | `Gamepad.AxisLX`  | -1.0 to 1.0 | / 155.0       |                 |
| Circle Pad Y   | `Gamepad.AxisLY`  | -1.0 to 1.0 | / 155.0       |                 |
| C-Stick X      | `Gamepad.AxisRX`  | -1.0 to 1.0 | / 145.0       | New 3DS only    |
| C-Stick Y      | `Gamepad.AxisRY`  | -1.0 to 1.0 | / 145.0       | New 3DS only    |
| L Trigger      | `Gamepad.AxisL`   | 0.0 or 1.0  | Digital        | L or ZL pressed |
| R Trigger      | `Gamepad.AxisR`   | 0.0 or 1.0  | Digital        | R or ZR pressed |

The L and R trigger axes are **digital** on 3DS - they read 1.0 when L/ZL or R/ZR is pressed, 0.0 otherwise.

### Touchscreen

The 3DS bottom screen is a single-point touchscreen. Use the touch API:

```lua
if Input.IsTouchDown(1) then
    local x, y = Input.GetTouchPosition(1)
    -- x, y are pixel coordinates on the bottom screen
end
```

Only single-touch is supported (touch index 1 only).

### Accelerometer

```lua
-- Returns a Vector with (x, y, z) accelerometer values
-- Raw values from hidAccelRead() are scaled by 1/512
local accel = Input.GetGamepadAcceleration(1)
```

**C++:**
```cpp
float ax, ay, az;
INP_GetGamepadAcceleration(ax, ay, az, 0);
```

### Gyroscope

The 3DS has a true gyroscope (unlike Wii):

```lua
-- Returns a Vector with (x, y, z) angular rate values
-- Raw values from hidGyroRead() are scaled by 1/1024
local gyro = Input.GetGamepadGyro(1)
```

**C++:**
```cpp
float gx, gy, gz;
INP_GetGamepadGyro(gx, gy, gz, 0);
```

### Soft Keyboard

The 3DS supports an on-screen keyboard that works **synchronously**. When called, it blocks until the user confirms or cancels:

```lua
local text = Input.ShowSoftKeyboard(true)
if text then
    print("User entered: " .. text)
else
    print("User cancelled")
end
```

The keyboard uses a 256-byte buffer. `IsSoftKeyboardShown()` always returns `false` on 3DS because the keyboard is modal/synchronous.

### Key Table Conflicts

On 3DS, the `Key` table entries `Key.A`, `Key.B`, `Key.L`, `Key.R`, `Key.X`, `Key.Y`, `Key.Up`, `Key.Down`, `Key.Left`, and `Key.Right` all resolve to **0** due to naming conflicts with 3DS SDK constants. Do not use keyboard input on 3DS.

---

## Android

### Touch Input

Android supports multitouch with up to 4 simultaneous touch points:

```lua
for i = 1, 4 do
    if Input.IsTouchDown(i) then
        local x, y = Input.GetTouchPosition(i)
        -- Handle touch point i
    end
end
```

### Keyboard

Android supports both hardware keyboards and the soft keyboard:

```lua
-- Show the on-screen keyboard
Input.ShowSoftKeyboard(true)

-- Check if it's currently shown
if Input.IsSoftKeyboardShown() then
    -- Keyboard is visible
end
```

### Gamepad

Android gamepad support exists in the codebase but the button mapping is **incomplete** (commented out as TODO). See [Known Gaps](KnownGaps.md).

---

## Windows

### Keyboard & Mouse

Full keyboard and mouse support with all features (cursor lock, trap, show/hide, all 5 mouse buttons, scroll wheel).

### Gamepad

Windows uses **XInput** for gamepad support. Xbox controllers (360, One, Series) work natively. Up to 4 controllers.

`GetGamepadType()` returns `"Standard"` for all Windows gamepads.

---

## Linux

### Keyboard & Mouse

Full keyboard and mouse support. Key codes use **XCB/X11** scancodes (different values than Windows but the `Key` table abstracts this). Cursor control uses XCB pointer grab/warp.

### Gamepad

Linux reads gamepads from `/dev/input/js0` through `/dev/input/js3`. The implementation opens device files in non-blocking mode and reads joystick events.

Current button mapping is based on **Xbox 360 controllers** only. Other controllers may have incorrect mappings. See [Known Gaps](KnownGaps.md).

`GetGamepadType()` returns `"Standard"` for all Linux gamepads.
