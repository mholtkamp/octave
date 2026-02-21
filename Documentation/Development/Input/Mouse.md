# Mouse Input

Mouse input is available on **Windows**, **Linux**, **Wii** (IR pointer), and **Android**. It is not available on GameCube or 3DS.

## Mouse Button Reference

| Lua            | Lua (numeric) | C++            |
|----------------|---------------|----------------|
| `Mouse.Left`   | `Mouse[1]`    | `MOUSE_LEFT`   |
| `Mouse.Right`  | `Mouse[2]`    | `MOUSE_RIGHT`  |
| `Mouse.Middle` | `Mouse[3]`    | `MOUSE_MIDDLE` |
| `Mouse.X1`     | `Mouse[4]`    | `MOUSE_X1`     |
| `Mouse.X2`     | `Mouse[5]`    | `MOUSE_X2`     |

## Button State Functions

### Lua

```lua
-- Is mouse button currently held?
if Input.IsMouseDown(Mouse.Left) then
    fireContinuous()
end

-- Was mouse button pressed this frame?
if Input.IsMousePressed(Mouse.Left) then
    fireSingle()
end

-- Was mouse button released this frame?
if Input.IsMouseReleased(Mouse.Left) then
    stopFiring()
end
```

**Function aliases:**
- `IsMouseDown` = `IsMouseButtonDown`
- `IsMousePressed` = `IsMouseButtonJustDown`
- `IsMouseReleased` = `IsMouseButtonJustUp`

### C++

```cpp
#include "InputDevices.h"

bool down     = IsMouseButtonDown(MOUSE_LEFT);
bool pressed  = IsMouseButtonJustDown(MOUSE_LEFT);
bool released = IsMouseButtonJustUp(MOUSE_LEFT);
```

## Position

### GetMousePosition()

Returns the mouse cursor position in **pixel coordinates** relative to the window.

```lua
local x, y = Input.GetMousePosition()
print("Mouse at: " .. x .. ", " .. y)
```

**C++:**
```cpp
int32_t mouseX, mouseY;
GetMousePosition(mouseX, mouseY);
```

### GetMouseDelta()

Returns how many pixels the mouse moved since the last frame. Essential for mouse-look camera controls.

```lua
local dx, dy = Input.GetMouseDelta()
```

**C++:**
```cpp
int32_t deltaX, deltaY;
INP_GetMouseDelta(deltaX, deltaY);
```

## Scroll Wheel

### GetScrollWheelDelta()

Returns the scroll wheel delta for this frame. Typically +1 for scroll up, -1 for scroll down.

```lua
local scroll = Input.GetScrollWheelDelta()
if scroll > 0 then
    zoomIn()
elseif scroll < 0 then
    zoomOut()
end
```

**C++:**
```cpp
int32_t scroll = GetScrollWheelDelta();
```

## Cursor Control

### ShowCursor(show)

Shows or hides the operating system cursor.

```lua
Input.ShowCursor(false) -- Hide cursor
Input.ShowCursor(true)  -- Show cursor
```

### LockCursor(lock)

Locks the cursor to the center of the window. When locked, the cursor is reset to the window center each frame. Use `GetMouseDelta()` to read movement while locked. This is essential for FPS-style mouse look.

```lua
Input.LockCursor(true)  -- Lock to center
Input.LockCursor(false) -- Unlock
```

### TrapCursor(trap)

Prevents the cursor from leaving the window bounds but does not lock it to center.

```lua
Input.TrapCursor(true)  -- Trap inside window
Input.TrapCursor(false) -- Allow leaving window
```

### SetCursorPosition(x, y)

Warps the cursor to the given pixel coordinates.

```lua
Input.SetCursorPosition(400, 300)
```

**C++:**
```cpp
INP_ShowCursor(false);
INP_LockCursor(true);
INP_TrapCursor(true);
INP_SetCursorPos(400, 300);
```

## Practical Example: FPS Mouse Look

This pattern is taken from `FirstPersonController.lua`:

```lua
function MyController:Start()
    Input.LockCursor(true)
    Input.TrapCursor(true)
    Input.ShowCursor(false)
end

function MyController:Stop()
    Input.LockCursor(false)
    Input.TrapCursor(false)
    Input.ShowCursor(true)
end

function MyController:Tick(deltaTime)
    local dx, dy = Input.GetMouseDelta()
    dx = dx * self.mouseSensitivity
    dy = dy * self.mouseSensitivity

    -- Rotate yaw based on horizontal mouse movement
    local rot = self.node:GetRotation()
    rot.y = rot.y - dx * self.lookSpeed * deltaTime
    self.node:SetRotation(rot)

    -- Rotate camera pitch based on vertical mouse movement
    local camRot = self.camera:GetRotation()
    camRot.x = camRot.x - dy * self.lookSpeed * deltaTime
    camRot.x = Math.Clamp(camRot.x, -89.9, 89.9)
    self.camera:SetRotation(camRot)
end
```

## Platform Notes

- **GameCube**: No mouse support.
- **3DS**: No mouse support. Use touch input instead.
- **Wii**: The Wii IR pointer is exposed through the **touch** API, not the mouse API. See [Touch & Pointer Input](Touch.md) and [Platform-Specific Input](PlatformSpecific.md) for details. `INPUT_MOUSE_SUPPORT` is 1 on Wii to enable the mouse position tracking infrastructure.
- **Windows / Linux**: Full mouse support with cursor lock, trap, and all 5 buttons.
