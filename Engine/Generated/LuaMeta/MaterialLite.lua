--- @meta

---@class MaterialLite : Material
MaterialLite = {}

---@param slot integer
---@param texture? Texture
function MaterialLite:SetTexture(slot, texture) end

---@param slot integer
---@return any
function MaterialLite:GetTexture(slot) end

---@return integer
function MaterialLite:GetShadingModel() end

---@param value integer
function MaterialLite:SetShadingModel(value) end

---@param value integer
function MaterialLite:SetBlendMode(value) end

---@param uvIndex? integer
---@return Vector
function MaterialLite:GetUvOffset(uvIndex) end

---@param value Vector
---@param uvIndex? integer
function MaterialLite:SetUvOffset(value, uvIndex) end

---@param uvIndex? integer
---@return Vector
function MaterialLite:GetUvScale(uvIndex) end

---@param value Vector
---@param uvIndex? integer
function MaterialLite:SetUvScale(value, uvIndex) end

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

---@param value integer
function MaterialLite:SetCullMode(value) end

---@return any
function MaterialLite:Create() end
