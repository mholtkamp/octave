--- @meta

---@class ParticleSystem : Asset
ParticleSystem = {}

---@param value? Material
function ParticleSystem:SetMaterial(value) end

---@return Asset
function ParticleSystem:GetMaterial() end

---@param value number
function ParticleSystem:SetDuration(value) end

---@return number
function ParticleSystem:GetDuration() end

---@param value number
function ParticleSystem:SetSpawnRate(value) end

---@return number
function ParticleSystem:GetSpawnRate() end

---@param value integer
function ParticleSystem:SetBurstCount(value) end

---@return integer
function ParticleSystem:GetBurstCount() end

---@param value number
function ParticleSystem:SetBurstWindow(value) end

---@return number
function ParticleSystem:GetBurstWindow() end

---@param value integer
function ParticleSystem:SetMaxParticles(value) end

---@return integer
function ParticleSystem:GetMaxParticles() end

---@param value integer
function ParticleSystem:SetLoops(value) end

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
---@param maxVal? number
function ParticleSystem:SetLifetime(minVal, maxVal) end

---@return number
function ParticleSystem:GetLifetimeMin() end

---@return number
function ParticleSystem:GetLifetimeMax() end

---@param minVal Vector
---@param maxVal? Vector
function ParticleSystem:SetPosition(minVal, maxVal) end

---@return Vector
function ParticleSystem:GetPositionMin() end

---@return Vector
function ParticleSystem:GetPositionMax() end

---@param minVal Vector
---@param maxVal? Vector
function ParticleSystem:SetVelocity(minVal, maxVal) end

---@return Vector
function ParticleSystem:GetVelocityMin() end

---@return Vector
function ParticleSystem:GetVelocityMax() end

---@param minVal Vector
---@param maxVal? Vector
function ParticleSystem:SetSize(minVal, maxVal) end

---@return Vector
function ParticleSystem:GetSizeMin() end

---@return Vector
function ParticleSystem:GetSizeMax() end

---@param minVal number
---@param maxVal? number
function ParticleSystem:SetRotation(minVal, maxVal) end

---@return number
function ParticleSystem:GetRotationMin() end

---@return number
function ParticleSystem:GetRotationMax() end

---@param minVal number
---@param maxVal? number
function ParticleSystem:SetRotationSpeed(minVal, maxVal) end

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
