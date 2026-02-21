# Touch & Pointer Input

Touch input is available on **Windows**, **Linux**, **Wii** (IR pointer), **3DS** (touchscreen), and **Android**. It is not available on GameCube.

There are two related APIs:

- **Touch** - Raw touch point data (position and down state per touch index)
- **Pointer** - Unified abstraction that combines mouse and touch into a single interface

## Touch Functions

### IsTouchDown(index)

Returns `true` if the given touch point is active. Index is **1-based** in Lua, **0-based** in C++.

```lua
if Input.IsTouchDown(1) then
    -- First touch is active
end
```

Up to **4 simultaneous touch points** are supported (`INPUT_MAX_TOUCHES = 4`).

### GetTouchPosition(index)

Returns the touch position in **pixel coordinates**. Index defaults to 1 if omitted.

```lua
local x, y = Input.GetTouchPosition(1)
print("Touch at: " .. x .. ", " .. y)
```

### GetTouchPositionNormalized(index)

Returns the touch position in **normalized coordinates** ranging from **-1.0 to 1.0**, where (0, 0) is the screen center, (-1, -1) is the top-left, and (1, 1) is the bottom-right.

```lua
local nx, ny = Input.GetTouchPositionNormalized(1)
```

### C++ Equivalents

```cpp
#include "InputDevices.h"

bool down = IsTouchDown(0);  // 0-based index

int32_t touchX, touchY;
GetTouchPosition(touchX, touchY, 0);

float normX, normY;
GetTouchPositionNormalized(normX, normY, 0);
```

## Pointer Functions

The Pointer API provides a unified way to handle both mouse and touch input. On desktop platforms, pointer events come from the mouse. On touch platforms, they come from the touchscreen. This makes it easy to write cross-platform UI code.

### IsPointerDown(index)

Returns `true` if the pointer is currently down. Index is **1-based** in Lua (defaults to 1 if omitted).

```lua
if Input.IsPointerDown() then
    -- Mouse button or touch is held
end
```

### IsPointerPressed(index) / IsPointerJustDown(index)

Returns `true` only on the frame the pointer goes down.

```lua
if Input.IsPointerPressed() then
    onTapStart()
end
```

**Function aliases:**
- `IsPointerPressed` = `IsPointerJustDown`

### IsPointerReleased(index) / IsPointerJustUp(index)

Returns `true` only on the frame the pointer goes up.

```lua
if Input.IsPointerReleased() then
    onTapEnd()
end
```

**Function aliases:**
- `IsPointerReleased` = `IsPointerJustUp`

### GetPointerPosition(index)

Returns the pointer position in **pixel coordinates**.

```lua
local x, y = Input.GetPointerPosition()
```

### GetPointerPositionNormalized(index)

Returns the pointer position in **normalized coordinates** (-1.0 to 1.0).

```lua
local nx, ny = Input.GetPointerPositionNormalized()
```

### C++ Equivalents

```cpp
#include "InputDevices.h"

bool down     = IsPointerDown(0);
bool pressed  = IsPointerJustDown(0);
bool released = IsPointerJustUp(0);

int32_t px, py;
GetPointerPosition(px, py, 0);

float normX, normY;
GetPointerPositionNormalized(normX, normY, 0);
```

## Cross-Platform UI Example

Using the Pointer API for a button that works on both desktop (mouse) and mobile (touch):

```lua
function CheckButton(buttonX, buttonY, buttonW, buttonH)
    if Input.IsPointerPressed() then
        local px, py = Input.GetPointerPosition()
        if px >= buttonX and px <= buttonX + buttonW and
           py >= buttonY and py <= buttonY + buttonH then
            return true
        end
    end
    return false
end
```

## Platform Notes

- **Windows / Linux**: Touch maps to mouse input. Pointer functions use the mouse.
- **3DS**: Single touch point on the bottom screen only. Use touch index 1 (Lua) / 0 (C++). The top screen is display only.
- **Wii**: Each Wiimote's IR pointer is mapped as a separate touch point. Wiimote on channel 0 sets touch index 0 (Lua index 1), channel 1 sets touch index 1 (Lua index 2), etc. See [Platform-Specific Input](PlatformSpecific.md) for details.
- **Android**: Full multitouch support up to 4 simultaneous touches.
- **GameCube**: No touch support.
