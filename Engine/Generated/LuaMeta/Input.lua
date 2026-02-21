--- @meta

---@class InputModule
Input = {}

---@param key integer
---@return boolean
function Input.IsKeyDown(key) end

---@param key integer
---@return boolean
function Input.IsKeyJustDownRepeat(key) end

---@param key integer
---@return boolean
function Input.IsKeyJustDown(key) end

---@param key integer
---@return boolean
function Input.IsKeyJustUp(key) end

---@return boolean
function Input.IsControlDown() end

---@return boolean
function Input.IsShiftDown() end

---@param mouse integer
---@return boolean
function Input.IsMouseButtonDown(mouse) end

---@param mouse integer
---@return boolean
function Input.IsMouseButtonJustDown(mouse) end

---@param mouse integer
---@return boolean
function Input.IsMouseButtonJustUp(mouse) end

---@return integer, integer
function Input.GetMousePosition() end

---@return integer
function Input.GetScrollWheelDelta() end

---@return integer, integer
function Input.GetMouseDelta() end

---@param index integer
---@return boolean
function Input.IsTouchDown(index) end

---@param index? integer
---@return boolean
function Input.IsPointerJustUp(index) end

---@param index? integer
---@return boolean
function Input.IsPointerJustDown(index) end

---@param index? integer
---@return boolean
function Input.IsPointerDown(index) end

---@param index? integer
---@return integer, integer
function Input.GetTouchPosition(index) end

---@param index? integer
---@return number, number
function Input.GetTouchPositionNormalized(index) end

---@param index? integer
---@return integer, integer
function Input.GetPointerPosition(index) end

---@param index? integer
---@return number, number
function Input.GetPointerPositionNormalized(index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadButtonDown(button, index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadButtonJustDown(button, index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadButtonJustUp(button, index) end

---@param axis integer
---@param index? integer
---@return number
function Input.GetGamepadAxisValue(axis, index) end

---@param index? integer
---@return string
function Input.GetGamepadType(index) end

---@param index? integer
---@return boolean
function Input.IsGamepadConnected(index) end

---@param index? integer
function Input.GetGamepadGyro(index) end

---@param index? integer
function Input.GetGamepadAcceleration(index) end

---@param index? integer
function Input.GetGamepadOrientation(index) end

---@param show boolean
function Input.ShowCursor(show) end

---@param value boolean
function Input.TrapCursor(value) end

---@param value boolean
function Input.LockCursor(value) end

---@param x integer
---@param y integer
function Input.SetCursorPosition(x, y) end

---@return table
function Input.GetKeysJustDown() end

---@return boolean
function Input.IsAnyKeyJustDown() end

---@param keyCode integer
---@return string
function Input.ConvertKeyCodeToChar(keyCode) end

---@param show boolean
---@return nil
function Input.ShowSoftKeyboard(show) end

---@return boolean
function Input.IsSoftKeyboardShown() end

---@param key integer
---@return boolean
function Input.IsKeyPressedRepeat(key) end

---@param key integer
---@return boolean
function Input.IsKeyPressed(key) end

---@param key integer
---@return boolean
function Input.IsKeyReleased(key) end

---@param mouse integer
---@return boolean
function Input.IsMouseDown(mouse) end

---@param mouse integer
---@return boolean
function Input.IsMousePressed(mouse) end

---@param mouse integer
---@return boolean
function Input.IsMouseReleased(mouse) end

---@param index? integer
---@return boolean
function Input.IsPointerReleased(index) end

---@param index? integer
---@return boolean
function Input.IsPointerPressed(index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadDown(button, index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadPressed(button, index) end

---@param button integer
---@param index? integer
---@return boolean
function Input.IsGamepadReleased(button, index) end

---@param axis integer
---@param index? integer
---@return number
function Input.GetGamepadAxis(axis, index) end

---@return table
function Input.GetKeysPressed() end
