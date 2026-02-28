--- @meta

---@class Mesh3D : Primitive3D
Mesh3D = {}

---@return Asset
function Mesh3D:GetMaterial() end

---@return Asset
function Mesh3D:GetMaterialOverride() end

---@param material? Material
function Mesh3D:SetMaterialOverride(material) end

---@return Asset
function Mesh3D:InstantiateMaterial() end

---@return boolean
function Mesh3D:IsBillboard() end

---@param value boolean
function Mesh3D:SetBillboard(value) end
