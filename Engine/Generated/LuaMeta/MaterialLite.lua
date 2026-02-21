--- @meta

---@class MaterialLite : Material
local MaterialLite = {}

---@param slot integer
---@param arg2? Texture
function MaterialLite:SetTexture(slot, arg2) end

---@param slot integer
---@return any
function MaterialLite:GetTexture(slot) end

---@return integer
function MaterialLite:GetShadingModel() end

---@param arg1 integer
function MaterialLite:SetShadingModel(arg1) end

---@param arg1 integer
function MaterialLite:SetBlendMode(arg1) end

---@param arg1? integer
---@return Vector
function MaterialLite:GetUvOffset(arg1) end

---@param value Vector
---@param arg2? integer
function MaterialLite:SetUvOffset(value, arg2) end

---@param arg1? integer
---@return Vector
function MaterialLite:GetUvScale(arg1) end

---@param value Vector
---@param arg2? integer
function MaterialLite:SetUvScale(value, arg2) end

---@return Vector
function MaterialLite:GetColor() end

---@param value Vector
function MaterialLite:SetColor(value) end

---@param enable boolean
function MaterialLite:EnableFresnel(enable) end

---@return boolean
function MaterialLite:IsFresnelEnabled() end

---@return Vector
function MaterialLite:GetFresnelColor() end

---@param value Vector
function MaterialLite:SetFresnelColor(value) end

---@return number
function MaterialLite:GetFresnelPower() end

---@param value number
function MaterialLite:SetFresnelPower(value) end

---@return number
function MaterialLite:GetEmission() end

---@param value number
function MaterialLite:SetEmission(value) end

---@return number
function MaterialLite:GetWrapLighting() end

---@param value number
function MaterialLite:SetWrapLighting(value) end

---@return number
function MaterialLite:GetSpecular() end

---@param value number
function MaterialLite:SetSpecular(value) end

---@return number
function MaterialLite:GetOpacity() end

---@param value number
function MaterialLite:SetOpacity(value) end

---@param value number
function MaterialLite:SetMaskCutoff(value) end

---@param value integer
function MaterialLite:SetSortPriority(value) end

---@param value boolean
function MaterialLite:SetDepthTestDisabled(value) end

---@param slot integer
---@return integer
function MaterialLite:GetUvMap(slot) end

---@param slot integer
---@param uvMapIdx integer
function MaterialLite:SetUvMap(slot, uvMapIdx) end

---@param slot integer
---@return integer
function MaterialLite:GetTevMode(slot) end

---@param slot integer
---@param tevMode integer
function MaterialLite:SetTevMode(slot, tevMode) end

---@param arg1 integer
function MaterialLite:SetCullMode(arg1) end

---@return any
function MaterialLite:Create() end
