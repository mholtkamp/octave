--- @meta

---@class NodeGraphPlayer : Node
local NodeGraphPlayer = {}

function NodeGraphPlayer:Play() end

function NodeGraphPlayer:Pause() end

function NodeGraphPlayer:Stop() end

function NodeGraphPlayer:Reset() end

---@return boolean
function NodeGraphPlayer:IsPlaying() end

---@return boolean
function NodeGraphPlayer:IsPaused() end

---@param rawAsset? Asset
function NodeGraphPlayer:SetNodeGraphAsset(rawAsset) end

---@return any
function NodeGraphPlayer:GetNodeGraphAsset() end

---@param name string
---@param value number
---@return boolean
function NodeGraphPlayer:SetInputFloat(name, value) end

---@param name string
---@param arg2 number
---@return boolean
function NodeGraphPlayer:SetInputInt(name, arg2) end

---@param name string
---@param value boolean
---@return boolean
function NodeGraphPlayer:SetInputBool(name, value) end

---@param name string
---@param value string
---@return boolean
function NodeGraphPlayer:SetInputString(name, value) end

---@param name string
---@param value Vector
---@return boolean
function NodeGraphPlayer:SetInputVector(name, value) end

---@param name string
---@param value Vector
---@return boolean
function NodeGraphPlayer:SetInputColor(name, value) end

---@param arg1 number
---@return number
function NodeGraphPlayer:GetOutputFloat(arg1) end

---@param arg1 number
---@return integer
function NodeGraphPlayer:GetOutputInt(arg1) end

---@param arg1 number
---@return boolean
function NodeGraphPlayer:GetOutputBool(arg1) end

---@param arg1 number
---@return Vector
function NodeGraphPlayer:GetOutputVector(arg1) end

---@param arg1 number
---@return Vector
function NodeGraphPlayer:GetOutputColor(arg1) end

---@param playOnStart boolean
function NodeGraphPlayer:SetPlayOnStart(playOnStart) end

---@return boolean
function NodeGraphPlayer:GetPlayOnStart() end
