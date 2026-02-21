--- @meta

---@class Rect
---@field x number
---@field y number
---@field w number
---@field h number
---@field width number
---@field height number
Rect = {}

---@return any
function Rect:Create() end

---@param x number
---@param y number
---@return boolean
function Rect:ContainsPoint(x, y) end

---@param b Rect
---@return boolean
function Rect:OverlapsRect(b) end

---@param b Rect
function Rect:Clamp(b) end

---@return number
function Rect:Top() end

---@return number
function Rect:Bottom() end

---@return number
function Rect:Left() end

---@return number
function Rect:Right() end
