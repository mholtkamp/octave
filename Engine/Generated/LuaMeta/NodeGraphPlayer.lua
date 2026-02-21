--- @meta

---@class NodeGraphPlayer : Node
NodeGraphPlayer = {}

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
---@param value number
---@return boolean
function NodeGraphPlayer:SetInputInt(name, value) end

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

---@param pinIndex number
---@return number
function NodeGraphPlayer:GetOutputFloat(pinIndex) end

---@param pinIndex number
---@return integer
function NodeGraphPlayer:GetOutputInt(pinIndex) end

---@param pinIndex number
---@return boolean
function NodeGraphPlayer:GetOutputBool(pinIndex) end

---@param pinIndex number
---@return Vector
function NodeGraphPlayer:GetOutputVector(pinIndex) end

---@param pinIndex number
---@return Vector
function NodeGraphPlayer:GetOutputColor(pinIndex) end

---@param playOnStart boolean
function NodeGraphPlayer:SetPlayOnStart(playOnStart) end

---@return boolean
function NodeGraphPlayer:GetPlayOnStart() end
