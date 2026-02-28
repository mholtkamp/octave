# Keyboard Input

Keyboard input is available on **Windows**, **Linux**, and **Android**. It is not available on GameCube, Wii, or 3DS (see [Platform Notes](#platform-notes) below).

## Key Table Reference

All key constants are accessed via the global `Key` table in Lua, or the `KEY_` enum in C++.

### Letters

| Lua         | C++       |
|-------------|-----------|
| `Key.A`     | `KEY_A`   |
| `Key.B`     | `KEY_B`   |
| `Key.C`     | `KEY_C`   |
| ...         | ...       |
| `Key.Z`     | `KEY_Z`   |

All 26 letters A-Z are available.

### Numbers

Numbers have two naming styles in Lua:

| Lua (short) | Lua (word)  | C++       |
|-------------|-------------|-----------|
| `Key.N0`    | `Key.Zero`  | `KEY_0`   |
| `Key.N1`    | `Key.One`   | `KEY_1`   |
| `Key.N2`    | `Key.Two`   | `KEY_2`   |
| `Key.N3`    | `Key.Three` | `KEY_3`   |
| `Key.N4`    | `Key.Four`  | `KEY_4`   |
| `Key.N5`    | `Key.Five`  | `KEY_5`   |
| `Key.N6`    | `Key.Six`   | `KEY_6`   |
| `Key.N7`    | `Key.Seven` | `KEY_7`   |
| `Key.N8`    | `Key.Eight` | `KEY_8`   |
| `Key.N9`    | `Key.Nine`  | `KEY_9`   |

### Function Keys

| Lua        | C++        |
|------------|------------|
| `Key.F1`   | `KEY_F1`   |
| `Key.F2`   | `KEY_F2`   |
| ...        | ...        |
| `Key.F12`  | `KEY_F12`  |

### Common Keys

| Lua             | C++              | Description       |
|-----------------|------------------|-------------------|
| `Key.Space`     | `KEY_SPACE`      | Spacebar          |
| `Key.Enter`     | `KEY_ENTER`      | Enter / Return    |
| `Key.Backspace` | `KEY_BACKSPACE`  | Backspace         |
| `Key.Tab`       | `KEY_TAB`        | Tab               |
| `Key.Escape`    | `KEY_ESCAPE`     | Escape            |
| `Key.Back`      | `KEY_BACK`       | Back (Android)    |

### Modifier Keys

| Lua            | C++             |
|----------------|-----------------|
| `Key.ShiftL`   | `KEY_SHIFT_L`   |
| `Key.ShiftR`   | `KEY_SHIFT_R`   |
| `Key.ControlL` | `KEY_CONTROL_L` |
| `Key.ControlR` | `KEY_CONTROL_R` |
| `Key.AltL`     | `KEY_ALT_L`     |
| `Key.AltR`     | `KEY_ALT_R`     |

### Arrow Keys

| Lua         | C++         |
|-------------|-------------|
| `Key.Up`    | `KEY_UP`    |
| `Key.Down`  | `KEY_DOWN`  |
| `Key.Left`  | `KEY_LEFT`  |
| `Key.Right` | `KEY_RIGHT` |

### Navigation Keys

| Lua            | C++             |
|----------------|-----------------|
| `Key.Insert`   | `KEY_INSERT`    |
| `Key.Delete`   | `KEY_DELETE`    |
| `Key.Home`     | `KEY_HOME`      |
| `Key.End`      | `KEY_END`       |
| `Key.PageUp`   | `KEY_PAGE_UP`   |
| `Key.PageDown` | `KEY_PAGE_DOWN` |

### Numpad

| Lua            | C++            |
|----------------|----------------|
| `Key.Numpad0`  | `KEY_NUMPAD0`  |
| `Key.Numpad1`  | `KEY_NUMPAD1`  |
| ...            | ...            |
| `Key.Numpad9`  | `KEY_NUMPAD9`  |
| `Key.Decimal`  | `KEY_DECIMAL`  |

### Punctuation

| Lua               | C++                  | Character |
|--------------------|----------------------|-----------|
| `Key.Period`       | `KEY_PERIOD`         | `.`       |
| `Key.Comma`        | `KEY_COMMA`          | `,`       |
| `Key.Plus`         | `KEY_PLUS`           | `=`/`+`   |
| `Key.Minus`        | `KEY_MINUS`          | `-`/`_`   |
| `Key.Colon`        | `KEY_COLON`          | `;`/`:`   |
| `Key.Question`     | `KEY_QUESTION`       | `/`/`?`   |
| `Key.Squiggle`     | `KEY_SQUIGGLE`       | `` ` ``/`~` |
| `Key.LeftBracket`  | `KEY_LEFT_BRACKET`   | `[`/`{`   |
| `Key.RightBracket` | `KEY_RIGHT_BRACKET`  | `]`/`}`   |
| `Key.BackSlash`    | `KEY_BACK_SLASH`     | `\`/`\|`  |
| `Key.Quote`        | `KEY_QUOTE`          | `'`/`"`   |

## Lua Functions

### IsKeyDown(key)

Returns `true` if the key is currently held down.

```lua
if Input.IsKeyDown(Key.W) then
    -- Move forward every frame the key is held
    moveForward()
end
```

### IsKeyPressed(key) / IsKeyJustDown(key)

Returns `true` only on the frame the key is first pressed. Both names are aliases for the same function.

```lua
if Input.IsKeyPressed(Key.Space) then
    jump()
end
```

### IsKeyReleased(key) / IsKeyJustUp(key)

Returns `true` only on the frame the key is released. Both names are aliases for the same function.

```lua
if Input.IsKeyReleased(Key.E) then
    finishInteraction()
end
```

### IsKeyPressedRepeat(key) / IsKeyJustDownRepeat(key)

Returns `true` on the first press and then repeatedly while held, following the OS key repeat rate. Useful for text input or menu navigation.

```lua
if Input.IsKeyPressedRepeat(Key.Down) then
    moveMenuCursor(1)
end
```

### IsControlDown() / IsShiftDown()

Convenience helpers that check if either left or right modifier is held.

```lua
if Input.IsControlDown() and Input.IsKeyPressed(Key.S) then
    save()
end
```

### GetKeysPressed() / GetKeysJustDown()

Returns a table (array) of key codes that were just pressed this frame. Useful for text input.

```lua
local keys = Input.GetKeysPressed()
for i, keyCode in ipairs(keys) do
    local char = Input.ConvertKeyCodeToChar(keyCode)
    textBuffer = textBuffer .. char
end
```

### IsAnyKeyJustDown()

Returns `true` if any key was pressed this frame.

```lua
if Input.IsAnyKeyJustDown() then
    dismissSplashScreen()
end
```

### ConvertKeyCodeToChar(keyCode)

Converts a key code integer to a character string. Respects shift state automatically.

```lua
local char = Input.ConvertKeyCodeToChar(Key.A) -- returns "a" or "A" depending on shift
```

## C++ Equivalents

```cpp
#include "InputDevices.h"

// State checks
bool down    = IsKeyDown(KEY_W);
bool pressed = IsKeyJustDown(KEY_SPACE);
bool repeat  = IsKeyJustDownRepeat(KEY_DOWN);
bool up      = IsKeyJustUp(KEY_E);

// Modifiers
bool ctrl  = IsControlDown();
bool shift = IsShiftDown();
bool alt   = IsAltDown();

// Low-level (Input/Input.h)
char ch = INP_ConvertKeyCodeToChar(KEY_A);
```

## Platform Notes

- **GameCube / Wii**: No keyboard support. `INPUT_KEYBOARD_SUPPORT` is 0 on these platforms.
- **3DS**: No physical keyboard. The `Key` table still exists in Lua but several entries conflict with 3DS gamepad button names. Specifically, `Key.A`, `Key.B`, `Key.L`, `Key.R`, `Key.X`, `Key.Y`, `Key.Up`, `Key.Down`, `Key.Left`, and `Key.Right` all resolve to `0` on 3DS. Do not use keyboard input on 3DS; use gamepad functions instead.
- **Android**: Keyboard input is supported (hardware or software keyboards). Use `Input.ShowSoftKeyboard(true)` to bring up the on-screen keyboard.
