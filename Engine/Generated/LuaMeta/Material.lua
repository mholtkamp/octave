--- @meta

---@class Material : Asset
Material = {}

---@return boolean
function Material:IsBase() end

---@return boolean
function Material:IsInstance() end

---@return boolean
function Material:IsLite() end

---@param name string
---@param value number
function Material:SetScalarParameter(name, value) end

---@param name string
---@param value Vector
function Material:SetVectorParameter(name, value) end

---@param name string
---@param value Texture
function Material:SetTextureParameter(name, value) end

---@param name string
---@return number
function Material:GetScalarParameter(name) end

---@param name string
---@return Vector
function Material:GetVectorParameter(name) end

---@param name string
---@return any
function Material:GetTextureParameter(name) end

---@return integer
function Material:GetBlendMode() end

---@return number
function Material:GetMaskCutoff() end

---@return integer
function Material:GetSortPriority() end

---@return boolean
function Material:IsDepthTestDisabled() end

---@return boolean
function Material:ShouldApplyFog() end

---@return integer
function Material:GetCullMode() end
