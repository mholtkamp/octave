--- @meta

---@class MaterialInstance : Material
MaterialInstance = {}

---@return Asset
function MaterialInstance:GetBaseMaterial() end

---@param base MaterialBase
function MaterialInstance:SetBaseMaterial(base) end

---@return Asset
function MaterialInstance:Create() end
