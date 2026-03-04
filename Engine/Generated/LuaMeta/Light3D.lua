--- @meta

---@class Light3D : Node3D
Light3D = {}

---@param value Vector
function Light3D:SetColor(value) end

---@return Vector
function Light3D:GetColor() end

---@param value number
function Light3D:SetIntensity(value) end

---@return number
function Light3D:GetIntensity() end

---@param value boolean
function Light3D:SetCastShadows(value) end

---@return boolean
function Light3D:ShouldCastShadows() end

---@return integer
function Light3D:GetDomain() end

---@return integer
function Light3D:GetLightingChannels() end

---@param value integer
function Light3D:SetLightingChannels(value) end
