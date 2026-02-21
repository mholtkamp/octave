--- @meta

---@class Mesh3D : Primitive3D
local Mesh3D = {}

---@return any
function Mesh3D:GetMaterial() end

---@return any
function Mesh3D:GetMaterialOverride() end

---@param arg1? Material
function Mesh3D:SetMaterialOverride(arg1) end

---@return any
function Mesh3D:InstantiateMaterial() end

---@return boolean
function Mesh3D:IsBillboard() end

---@param value boolean
function Mesh3D:SetBillboard(value) end
