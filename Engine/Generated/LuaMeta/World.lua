--- @meta

---@class World
local World = {}
World = World

---@return Node
function World:GetActiveCamera() end

---@return Node
function World:GetAudioReceiver() end

---@param cameraComp Camera3D
function World:SetActiveCamera(cameraComp) end

---@param arg1? Node3D
function World:SetAudioReceiver(arg1) end

---@param nodeClass string
---@param arg2? Vector
---@return Node
function World:SpawnNode(nodeClass, arg2) end

---@param scene Scene
---@param arg2? Vector
---@return Node
function World:SpawnScene(scene, arg2) end

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
---@param arg3 integer
---@param arg4 table
---@param arg5 boolean
---@return number
function World:RayTest(start, end, arg3, arg4, arg5) end

---@param start Vector
---@param end Vector
---@param arg3 integer
---@param arg4 boolean
---@return integer
function World:RayTestMulti(start, end, arg3, arg4) end

---@param primComp Primitive3D
---@param start Vector
---@param end Vector
---@param arg4 integer
---@return number
function World:SweepTest(primComp, start, end, arg4) end

---@param newScene string
---@param arg2? boolean
function World:LoadScene(newScene, arg2) end

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

function World:Clear() end

---@param arg1 table
function World:SetFog(arg1) end

---@return number
function World:GetFog() end
