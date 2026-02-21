--- @meta

---@class Button : Widget
local Button = {}

function Button:SetSelected() end

---@return Node
function Button:GetSelected() end

function Button:EnableMouseHandling() end

function Button:EnableGamepadHandling() end

function Button:EnableKeyboardHandling() end

---@return boolean
function Button:IsSelected() end

function Button:Activate() end

---@return integer
function Button:GetState() end

function Button:SetLocked() end

---@param value string
function Button:SetTextString(value) end

---@return string
function Button:GetTextString() end

---@param arg1? Texture
---@param arg2? Texture
---@param arg3? Texture
---@param arg4? Texture
function Button:SetStateTextures(arg1, arg2, arg3, arg4) end

---@param normal Vector
---@param hovered Vector
---@param pressed Vector
---@param locked Vector
function Button:SetStateColors(normal, hovered, pressed, locked) end

---@return any, any, any, any
function Button:GetStateTextures() end

---@return Vector, any, any, any
function Button:GetStateColors() end

---@param arg1 Node
---@param arg2 Node
---@param arg3 Node
---@param arg4 Node
function Button:SetNavigation(arg1, arg2, arg3, arg4) end

---@return Node, any, any, any
function Button:GetNavigation() end

---@return Node
function Button:GetText() end

---@return Node
function Button:GetQuad() end
