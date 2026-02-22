--- @meta

---@class SkeletalMesh3D : Mesh3D
SkeletalMesh3D = {}

---@param skMesh? SkeletalMesh
function SkeletalMesh3D:SetSkeletalMesh(skMesh) end

---@return Asset
function SkeletalMesh3D:GetSkeletalMesh() end

---@param animName string
---@param slot? integer
---@param loop? boolean
---@param speed? number
---@param weight? number
function SkeletalMesh3D:PlayAnimation(animName, slot, loop, speed, weight) end

---@param animName string
---@param cancelQueued? boolean
function SkeletalMesh3D:StopAnimation(animName, cancelQueued) end

---@param cancelQueued? boolean
function SkeletalMesh3D:StopAllAnimations(cancelQueued) end

---@param animName string
---@return boolean
function SkeletalMesh3D:IsAnimationPlaying(animName) end

---@param animName string
---@param dependentAnimName? string
---@param slot? integer
---@param loop? boolean
---@param speed? number
---@param weight? number
function SkeletalMesh3D:QueueAnimation(animName, dependentAnimName, slot, loop, speed, weight) end

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
