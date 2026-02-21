--- @meta

---@class Node3D : Node
local Node3D = {}

---@param newParent SkeletalMesh3D
---@param boneName string
---@param arg3? boolean
---@param arg4? integer
function Node3D:AttachToBone(newParent, boneName, arg3, arg4) end

function Node3D:UpdateTransform() end

---@return Vector
function Node3D:GetPosition() end

---@return Vector
function Node3D:GetRotationEuler() end

---@return Vector
function Node3D:GetRotationQuat() end

---@return Vector
function Node3D:GetScale() end

---@param pos Vector
function Node3D:SetPosition(pos) end

---@param rotEuler Vector
function Node3D:SetRotationEuler(rotEuler) end

---@param rotQuat Vector
function Node3D:SetRotationQuat(rotQuat) end

---@param scale Vector
function Node3D:SetScale(scale) end

---@param pivot Vector
---@param axis Vector
---@param degrees number
function Node3D:RotateAround(pivot, axis, degrees) end

---@return Vector
function Node3D:GetWorldPosition() end

---@return Vector
function Node3D:GetWorldRotationEuler() end

---@return Vector
function Node3D:GetWorldRotationQuat() end

---@return Vector
function Node3D:GetWorldScale() end

---@param pos Vector
function Node3D:SetWorldPosition(pos) end

---@param rotEuler Vector
function Node3D:SetWorldRotationEuler(rotEuler) end

---@param rotQuat Vector
function Node3D:SetWorldRotationQuat(rotQuat) end

---@param scale Vector
function Node3D:SetWorldScale(scale) end

---@param deltaDegrees Vector
function Node3D:AddRotationEuler(deltaDegrees) end

---@param deltaVec Vector
function Node3D:AddRotationQuat(deltaVec) end

---@param deltaDegrees Vector
function Node3D:AddWorldRotationEuler(deltaDegrees) end

---@param deltaVec Vector
function Node3D:AddWorldRotationQuat(deltaVec) end

---@param worldPos Vector
---@param arg2 Vector
function Node3D:LookAt(worldPos, arg2) end

---@return Vector
function Node3D:GetForwardVector() end

---@return Vector
function Node3D:GetRightVector() end

---@return Vector
function Node3D:GetUpVector() end

---@return boolean
function Node3D:GetInheritTransform() end

---@param value boolean
function Node3D:SetInheritTransform(value) end

---@return Vector
function Node3D:GetRotation() end

---@param rotEuler Vector
function Node3D:SetRotation(rotEuler) end

---@return Vector
function Node3D:GetAbsolutePosition() end

---@return Vector
function Node3D:GetWorldRotation() end

---@return Vector
function Node3D:GetAbsoluteRotation() end

---@return Vector
function Node3D:GetAbsoluteRotationQuat() end

---@return Vector
function Node3D:GetAbsoluteScale() end

---@param pos Vector
function Node3D:SetAbsolutePosition(pos) end

---@param rotEuler Vector
function Node3D:SetWorldRotation(rotEuler) end

---@param rotEuler Vector
function Node3D:SetAbsoluteRotation(rotEuler) end

---@param rotQuat Vector
function Node3D:SetAbsoluteRotationQuat(rotQuat) end

---@param scale Vector
function Node3D:SetAbsoluteScale(scale) end

---@param deltaDegrees Vector
function Node3D:AddRotation(deltaDegrees) end

---@param deltaDegrees Vector
function Node3D:AddWorldRotation(deltaDegrees) end

---@param deltaDegrees Vector
function Node3D:AddAbsoluteRotation(deltaDegrees) end

---@param deltaVec Vector
function Node3D:AddAbsoluteRotationQuat(deltaVec) end
