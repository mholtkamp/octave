--- @meta

---@class Particle3D : Primitive3D
local Particle3D = {}

function Particle3D:Reset() end

---@param value boolean
function Particle3D:EnableEmission(value) end

---@return boolean
function Particle3D:IsEmissionEnabled() end

---@param value boolean
function Particle3D:EnableSimulation(value) end

---@return boolean
function Particle3D:IsSimulationEnabled() end

---@return number
function Particle3D:GetElapsedTime() end

---@param arg1? ParticleSystem
function Particle3D:SetParticleSystem(arg1) end

---@return any
function Particle3D:GetParticleSystem() end

---@param arg1? Material
function Particle3D:SetMaterialOverride(arg1) end

---@return any
function Particle3D:GetMaterial() end

---@return any
function Particle3D:GetMaterialOverride() end

---@param value number
function Particle3D:SetTimeMultiplier(value) end

---@return number
function Particle3D:GetTimeMultiplier() end

---@param value boolean
function Particle3D:SetUseLocalSpace(value) end

---@return boolean
function Particle3D:GetUseLocalSpace() end

---@return integer
function Particle3D:GetNumParticles() end

---@param index integer
---@return nil
function Particle3D:GetParticleData(index) end

---@param index integer
function Particle3D:SetParticleData(index) end

---@param value boolean
function Particle3D:EnableAutoEmit(value) end

---@return boolean
function Particle3D:IsAutoEmitEnabled() end

---@param arg1 integer
function Particle3D:SetParticleOrientation(arg1) end

---@return integer
function Particle3D:GetParticleOrientation() end

---@param value boolean
function Particle3D:EnableAutoDestroy(value) end

---@return any
function Particle3D:InstantiateParticleSystem() end
