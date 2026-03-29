# WindowManager

Global module for managing Window widgets by ID. Windows with a Window Id property automatically register with the WindowManager when they start.

---
### FindWindow
Find a window by its ID.

Sig: `window = WindowManager.FindWindow(id)`
 - Arg: `string id` Window identifier
 - Ret: `Window window` Window widget or nil if not found
---
### HasWindow
Check if a window with the given ID exists.

Sig: `exists = WindowManager.HasWindow(id)`
 - Arg: `string id` Window identifier
 - Ret: `boolean exists` True if window exists
---
### ShowWindow
Show a window by its ID.

Sig: `WindowManager.ShowWindow(id)`
 - Arg: `string id` Window identifier
---
### HideWindow
Hide a window by its ID.

Sig: `WindowManager.HideWindow(id)`
 - Arg: `string id` Window identifier
---
### CloseWindow
Close a window by its ID (emits Close signal then hides).

Sig: `WindowManager.CloseWindow(id)`
 - Arg: `string id` Window identifier
---
### BringToFront
Bring a window to the front of its parent's children.

Sig: `WindowManager.BringToFront(id)`
 - Arg: `string id` Window identifier

## Example

```lua
-- Check if a window exists
if WindowManager.HasWindow("settings") then
    -- Toggle visibility
    local win = WindowManager.FindWindow("settings")
    if win:IsVisible() then
        WindowManager.HideWindow("settings")
    else
        WindowManager.ShowWindow("settings")
    end
end

-- Show multiple windows
WindowManager.ShowWindow("inventory")
WindowManager.ShowWindow("map")

-- Bring chat window to front when a message arrives
function OnMessageReceived()
    WindowManager.ShowWindow("chat")
    WindowManager.BringToFront("chat")
end

-- Close all windows
local windows = {"inventory", "settings", "map", "chat"}
for _, id in ipairs(windows) do
    if WindowManager.HasWindow(id) then
        WindowManager.CloseWindow(id)
    end
end
```
