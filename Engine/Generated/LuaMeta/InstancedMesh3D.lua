--- @meta

---@class InstancedMesh3D : StaticMesh3D
local InstancedMesh3D = {}

---@return integer
function InstancedMesh3D:GetNumInstances() end

---@param index integer
---@return nil
function InstancedMesh3D:GetInstanceData(index) end

---@param index integer
function InstancedMesh3D:SetInstanceData(index) end

function InstancedMesh3D:AddInstanceData() end

---@param index integer
function InstancedMesh3D:RemoveInstanceData(index) end
