--- @meta

---@class StaticMesh : Asset
local StaticMesh = {}

---@return any
function StaticMesh:GetMaterial() end

---@param arg1? Material
function StaticMesh:SetMaterial(arg1) end

---@return integer
function StaticMesh:GetNumIndices() end

---@return integer
function StaticMesh:GetNumFaces() end

---@return integer
function StaticMesh:GetNumVertices() end

---@return boolean
function StaticMesh:HasVertexColor() end

---@return integer
function StaticMesh:GetVertices() end

---@return integer
function StaticMesh:GetIndices() end

---@return boolean
function StaticMesh:HasTriangleMeshCollision() end

---@param value boolean
function StaticMesh:EnableTriangleMeshCollision(value) end
