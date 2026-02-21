--- @meta

---@class Quad : Widget
Quad = {}

---@param texture? Texture
function Quad:SetTexture(texture) end

---@return any
function Quad:GetTexture() end

---@param tl Vector
---@param tr Vector
---@param bl Vector
---@param br Vector
function Quad:SetVertexColors(tl, tr, bl, br) end

---@param value Vector
function Quad:SetUvScale(value) end

---@return Vector
function Quad:GetUvScale() end

---@param value Vector
function Quad:SetUvOffset(value) end

---@return Vector
function Quad:GetUvOffset() end
