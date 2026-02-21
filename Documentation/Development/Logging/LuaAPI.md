# Logging - Lua API

All logging functions are on the global `Log` table.

## Log.Debug(message)

Logs a debug-level message. Alias: `Log.Info(message)`.

```lua
Log.Debug("Game started")
Log.Debug("Score: " .. tostring(score))
Log.Info("This is the same as Debug")
```

Messages appear in **green** in the in-game console and editor Debug Log.

## Log.Warning(message)

Logs a warning-level message.

```lua
Log.Warning("Texture not found, using placeholder")
Log.Warning("Frame rate dropped below 30 FPS")
```

Messages appear in **yellow**.

## Log.Error(message)

Logs an error-level message.

```lua
Log.Error("Failed to load scene: " .. sceneName)
Log.Error("Null reference in Update")
```

Messages appear in **red**.

## Log.Console(message, color)

Logs a message to the in-game console with a custom color. This does **not** go through the severity system or trigger callbacks - it only writes to the Console widget.

The color is a `Vec4` with RGBA components (0.0 to 1.0).

```lua
-- Cyan message
Log.Console("Custom colored message", Vec(0, 1, 1, 1))

-- White message
Log.Console("Hello world", Vec(1, 1, 1, 1))
```

> **Note**: The Lua parameter order is `(message, color)`, which differs from the C++ `LogConsole(color, format)`.

## Log.Enable(enabled)

Enables or disables logging at runtime. When disabled, all `Log.Debug`, `Log.Warning`, and `Log.Error` calls are silently ignored.

```lua
Log.Enable(false)  -- Suppress all log output
Log.Enable(true)   -- Resume logging
```

This only works when `LOGGING_ENABLED` is 1 at compile time.

## Log.IsEnabled()

Returns `true` if logging is currently enabled.

```lua
if Log.IsEnabled() then
    Log.Debug("Verbose status: " .. computeExpensiveStatus())
end
```

## String Formatting Tips

Lua's `Log` functions take a single string argument (no format specifiers like C's `printf`). Use Lua string concatenation or `string.format`:

```lua
-- Concatenation
Log.Debug("Player HP: " .. tostring(hp) .. "/" .. tostring(maxHp))

-- string.format
Log.Debug(string.format("Position: (%.2f, %.2f, %.2f)", x, y, z))

-- tostring for non-string types
Log.Debug("Object: " .. tostring(myNode))
```

## Complete Example

```lua
function MyScript:Start()
    Log.Debug("MyScript started on " .. self:GetName())
end

function MyScript:Tick(deltaTime)
    if self.health <= 0 then
        Log.Warning("Entity " .. self:GetName() .. " has zero health")
    end

    if not self.target then
        Log.Error("No target assigned to " .. self:GetName())
    end
end

function MyScript:OnDamage(amount, source)
    Log.Debug(string.format("%s took %d damage from %s",
        self:GetName(), amount, source:GetName()))
end
```
