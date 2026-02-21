--- @meta

---@class SkeletalMesh : Asset
local SkeletalMesh = {}

---@return any
function SkeletalMesh:GetMaterial() end

---@param arg1? Material
function SkeletalMesh:SetMaterial(arg1) end

---@return integer
function SkeletalMesh:GetNumIndices() end

---@return integer
function SkeletalMesh:GetNumFaces() end

---@return integer
function SkeletalMesh:GetNumVertices() end

---@param name string
---@return integer
function SkeletalMesh:FindBoneIndex(name) end

---@return integer
function SkeletalMesh:GetNumBones() end

---@param index integer
---@return string
function SkeletalMesh:GetAnimationName(index) end

---@return integer
function SkeletalMesh:GetNumAnimations() end

---@param name string
---@return number
function SkeletalMesh:GetAnimationDuration(name) end
