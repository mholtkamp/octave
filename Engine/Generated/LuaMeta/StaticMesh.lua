--- @meta

---@class StaticMesh : Asset
StaticMesh = {}

---@return Asset
function StaticMesh:GetMaterial() end

---@param material? Material
function StaticMesh:SetMaterial(material) end

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
