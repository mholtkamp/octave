--- @meta

---@class Texture : Asset
Texture = {}

---@return boolean
function Texture:IsMipmapped() end

---@return boolean
function Texture:IsRenderTarget() end

---@return integer
function Texture:GetWidth() end

---@return integer
function Texture:GetHeight() end

---@return integer
function Texture:GetMipLevels() end

---@return integer
function Texture:GetLayers() end

---@return integer
function Texture:GetFormat() end

---@return integer
function Texture:GetFilterType() end

---@return integer
function Texture:GetWrapMode() end
