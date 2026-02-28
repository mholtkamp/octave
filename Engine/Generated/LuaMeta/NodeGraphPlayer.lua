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

---@return Asset
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

---@param name string
---@param value number
---@return boolean
function NodeGraphPlayer:SetInputByte(name, value) end

---@param name string
---@param value? Asset
---@return boolean
function NodeGraphPlayer:SetInputAsset(name, value) end

---@param name string
---@param value Vector
---@return boolean
function NodeGraphPlayer:SetInputVector2D(name, value) end

---@param name string
---@param value number
---@return boolean
function NodeGraphPlayer:SetInputShort(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputNode(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputNode3D(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputWidget(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputText(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputQuad(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputAudio3D(name, value) end

---@param name string
---@param value? Asset
---@return boolean
function NodeGraphPlayer:SetInputScene(name, value) end

---@param name string
---@param value? Node
---@return boolean
function NodeGraphPlayer:SetInputSpline3D(name, value) end

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
---@return string
function NodeGraphPlayer:GetOutputString(pinIndex) end

---@param pinIndex number
---@return Vector
function NodeGraphPlayer:GetOutputVector2D(pinIndex) end

---@param pinIndex number
---@return Vector
function NodeGraphPlayer:GetOutputVector(pinIndex) end

---@param pinIndex number
---@return Vector
function NodeGraphPlayer:GetOutputColor(pinIndex) end

---@param pinIndex number
---@return integer
function NodeGraphPlayer:GetOutputByte(pinIndex) end

---@param pinIndex number
---@return integer
function NodeGraphPlayer:GetOutputShort(pinIndex) end

---@param pinIndex number
---@return Asset
function NodeGraphPlayer:GetOutputAsset(pinIndex) end

---@param pinIndex number
---@return Node
function NodeGraphPlayer:GetOutputNode(pinIndex) end

---@param playOnStart boolean
function NodeGraphPlayer:SetPlayOnStart(playOnStart) end

---@return boolean
function NodeGraphPlayer:GetPlayOnStart() end
