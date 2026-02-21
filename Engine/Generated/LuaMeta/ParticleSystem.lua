--- @meta

---@class ParticleSystem : Asset
local ParticleSystem = {}

---@param arg1? Material
function ParticleSystem:SetMaterial(arg1) end

---@return any
function ParticleSystem:GetMaterial() end

---@param value number
function ParticleSystem:SetDuration(value) end

---@return number
function ParticleSystem:GetDuration() end

---@param value number
function ParticleSystem:SetSpawnRate(value) end

---@return number
function ParticleSystem:GetSpawnRate() end

---@param arg1 integer
function ParticleSystem:SetBurstCount(arg1) end

---@return integer
function ParticleSystem:GetBurstCount() end

---@param value number
function ParticleSystem:SetBurstWindow(value) end

---@return number
function ParticleSystem:GetBurstWindow() end

---@param arg1 integer
function ParticleSystem:SetMaxParticles(arg1) end

---@return integer
function ParticleSystem:GetMaxParticles() end

---@param arg1 integer
function ParticleSystem:SetLoops(arg1) end

---@return integer
function ParticleSystem:GetLoops() end

---@param value boolean
function ParticleSystem:SetRadialVelocity(value) end

---@return boolean
function ParticleSystem:IsRadialVelocity() end

---@param value boolean
function ParticleSystem:SetRadialSpawn(value) end

---@return boolean
function ParticleSystem:IsRadialSpawn() end

---@param value boolean
function ParticleSystem:SetLockedRatio(value) end

---@return boolean
function ParticleSystem:IsRatioLocked() end

---@param minVal number
---@param arg2? number
function ParticleSystem:SetLifetime(minVal, arg2) end

---@return number
function ParticleSystem:GetLifetimeMin() end

---@return number
function ParticleSystem:GetLifetimeMax() end

---@param minVal Vector
---@param arg2? Vector
function ParticleSystem:SetPosition(minVal, arg2) end

---@return Vector
function ParticleSystem:GetPositionMin() end

---@return Vector
function ParticleSystem:GetPositionMax() end

---@param minVal Vector
---@param arg2? Vector
function ParticleSystem:SetVelocity(minVal, arg2) end

---@return Vector
function ParticleSystem:GetVelocityMin() end

---@return Vector
function ParticleSystem:GetVelocityMax() end

---@param minVal Vector
---@param arg2? Vector
function ParticleSystem:SetSize(minVal, arg2) end

---@return Vector
function ParticleSystem:GetSizeMin() end

---@return Vector
function ParticleSystem:GetSizeMax() end

---@param minVal number
---@param arg2? number
function ParticleSystem:SetRotation(minVal, arg2) end

---@return number
function ParticleSystem:GetRotationMin() end

---@return number
function ParticleSystem:GetRotationMax() end

---@param minVal number
---@param arg2? number
function ParticleSystem:SetRotationSpeed(minVal, arg2) end

---@return number
function ParticleSystem:GetRotationSpeedMin() end

---@return number
function ParticleSystem:GetRotationSpeedMax() end

---@param value Vector
function ParticleSystem:SetAcceleration(value) end

---@return Vector
function ParticleSystem:GetAcceleration() end

---@param value number
function ParticleSystem:SetAlphaEase(value) end

---@return number
function ParticleSystem:GetAlphaEase() end

---@param value number
function ParticleSystem:SetScaleEase(value) end

---@return number
function ParticleSystem:GetScaleEase() end

---@param value Vector
function ParticleSystem:SetColorStart(value) end

---@return Vector
function ParticleSystem:GetColorStart() end

---@param value Vector
function ParticleSystem:SetColorEnd(value) end

---@return Vector
function ParticleSystem:GetColorEnd() end
