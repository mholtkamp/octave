--- @meta

---@class StaticMesh3D : Mesh3D
local StaticMesh3D = {}

---@param arg1? StaticMesh
function StaticMesh3D:SetStaticMesh(arg1) end

---@return any
function StaticMesh3D:GetStaticMesh() end

---@param value boolean
function StaticMesh3D:SetUseTriangleCollision(value) end

---@return boolean
function StaticMesh3D:GetUseTriangleCollision() end

---@return boolean
function StaticMesh3D:GetBakeLighting() end

---@param value boolean
function StaticMesh3D:EnableTriangleCollision(value) end

---@return boolean
function StaticMesh3D:IsTriangleCollisionEnabled() end
