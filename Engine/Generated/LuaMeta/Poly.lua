--- @meta

---@class Poly : Widget
Poly = {}

---@param pos Vector
---@param color? Vector
---@param uv? Vector
function Poly:AddVertex(pos, color, uv) end

function Poly:ClearVertices() end

---@return integer
function Poly:GetNumVertices() end

---@param index integer
---@return table
function Poly:GetVertex(index) end

---@param texture Texture
function Poly:SetTexture(texture) end

---@return any
function Poly:GetTexture() end

---@return number
function Poly:GetLineWidth() end

---@param value number
function Poly:SetLineWidth(value) end
