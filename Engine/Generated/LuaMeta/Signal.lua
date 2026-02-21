--- @meta

---@class Signal
local Signal = {}
Signal = Signal

---@return any
function Signal:Create() end

function Signal:Emit() end

---@param listenerNode Node
---@param arg2 function
function Signal:Connect(listenerNode, arg2) end

---@param listenerNode Node
function Signal:Disconnect(listenerNode) end
