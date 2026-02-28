--- @meta

---@class World
World = {}

---@return Node
function World:GetActiveCamera() end

---@return Node
function World:GetAudioReceiver() end

---@param cameraComp Camera3D
function World:SetActiveCamera(cameraComp) end

---@param transformComp? Node3D
function World:SetAudioReceiver(transformComp) end

---@param nodeClass string
---@param position? Vector
---@return Node
function World:SpawnNode(nodeClass, position) end

---@param scene Scene
---@param position? Vector
---@return Node
function World:SpawnScene(scene, position) end

---@return Node
function World:GetRootNode() end

---@param arg1? Node
function World:SetRootNode(arg1) end

function World:DestroyRootNode() end

---@param name string
---@return Node
function World:FindNode(name) end

---@param tag string
---@return Node
function World:FindNodesWithTag(tag) end

---@param name string
---@return Node
function World:FindNodesWithName(name) end

---@param color Vector
function World:SetAmbientLightColor(color) end

---@return Vector
function World:GetAmbientLightColor() end

---@param color Vector
function World:SetShadowColor(color) end

---@return Vector
function World:GetShadowColor() end

---@param arg1 table
function World:SetFogSettings(arg1) end

---@return number
function World:GetFogSettings() end

---@param value Vector
function World:SetGravity(value) end

---@return Vector
function World:GetGravity() end

---@param start Vector
---@param end Vector
---@param colMask integer
---@param arg4 table
---@param ignorePureOverlap boolean
---@return number
function World:RayTest(start, end, colMask, arg4, ignorePureOverlap) end

---@param start Vector
---@param end Vector
---@param colMask integer
---@param ignorePureOverlap boolean
---@return integer
function World:RayTestMulti(start, end, colMask, ignorePureOverlap) end

---@param primComp Primitive3D
---@param start Vector
---@param end Vector
---@param colMask integer
---@return number
function World:SweepTest(primComp, start, end, colMask) end

---@param newScene string
---@param instant? boolean
function World:LoadScene(newScene, instant) end

---@param newRoot Node
function World:QueueRootNode(newRoot) end

---@param value boolean
function World:EnableInternalEdgeSmoothing(value) end

---@return boolean
function World:IsInternalEdgeSmoothingEnabled() end

---@param particleSys ParticleSystem
---@param pos Vector
---@return Node
function World:SpawnParticle(particleSys, pos) end

---@param start Vector
---@param end Vector
---@return Vector
function World:FindNavPath(start, end) end

---@return Vector
function World:FindRandomNavPoint() end

---@param inPoint Vector
---@return Vector
function World:FindClosestNavPoint(inPoint) end

function World:BuildNavData() end

---@param enable boolean
function World:EnableAutoNavRebuild(enable) end

function World:Clear() end

---@param arg1 table
function World:SetFog(arg1) end

---@return number
function World:GetFog() end
