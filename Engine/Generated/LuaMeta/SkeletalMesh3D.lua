--- @meta

---@class SkeletalMesh3D : Mesh3D
local SkeletalMesh3D = {}

---@param arg1? SkeletalMesh
function SkeletalMesh3D:SetSkeletalMesh(arg1) end

---@return any
function SkeletalMesh3D:GetSkeletalMesh() end

---@param animName string
---@param arg2? integer
---@param arg3? boolean
---@param arg4? number
---@param arg5? number
function SkeletalMesh3D:PlayAnimation(animName, arg2, arg3, arg4, arg5) end

---@param animName string
---@param arg2? boolean
function SkeletalMesh3D:StopAnimation(animName, arg2) end

---@param arg1? boolean
function SkeletalMesh3D:StopAllAnimations(arg1) end

---@param animName string
---@return boolean
function SkeletalMesh3D:IsAnimationPlaying(animName) end

---@param animName string
---@param arg2? string
---@param arg3? integer
---@param arg4? boolean
---@param arg5? number
---@param arg6? number
function SkeletalMesh3D:QueueAnimation(animName, arg2, arg3, arg4, arg5, arg6) end

---@param animName string
function SkeletalMesh3D:CancelQueuedAnimation(animName) end

function SkeletalMesh3D:CancelAllQueuedAnimations() end

---@param value boolean
function SkeletalMesh3D:SetInheritPose(value) end

---@return boolean
function SkeletalMesh3D:IsInheritPoseEnabled() end

function SkeletalMesh3D:ResetAnimation() end

---@return number
function SkeletalMesh3D:GetAnimationSpeed() end

---@param speed number
function SkeletalMesh3D:SetAnimationSpeed(speed) end

---@param value boolean
function SkeletalMesh3D:SetAnimationPaused(value) end

---@return boolean
function SkeletalMesh3D:IsAnimationPaused() end

---@param boneName string
---@return Vector
function SkeletalMesh3D:GetBonePosition(boneName) end

---@param boneName string
---@return Vector
function SkeletalMesh3D:GetBoneRotation(boneName) end

---@param boneName string
---@return Vector
function SkeletalMesh3D:GetBoneScale(boneName) end

---@param boneName string
---@param value Vector
function SkeletalMesh3D:SetBonePosition(boneName, value) end

---@param boneName string
---@param value Vector
function SkeletalMesh3D:SetBoneRotation(boneName, value) end

---@param boneName string
---@param value Vector
function SkeletalMesh3D:SetBoneScale(boneName, value) end

---@return integer
function SkeletalMesh3D:GetNumBones() end

---@param arg1 function
function SkeletalMesh3D:SetAnimEventHandler(arg1) end

---@param radius number
function SkeletalMesh3D:SetBoundsRadiusOverride(radius) end

---@return number
function SkeletalMesh3D:GetBoundsRadiusOverride() end
