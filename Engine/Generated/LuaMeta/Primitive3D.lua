--- @meta

---@class Primitive3D : Node3D
Primitive3D = {}

---@param enable boolean
function Primitive3D:EnablePhysics(enable) end

---@param enable boolean
function Primitive3D:EnableCollision(enable) end

---@param enable boolean
function Primitive3D:EnableOverlaps(enable) end

---@return boolean
function Primitive3D:IsPhysicsEnabled() end

---@return boolean
function Primitive3D:IsCollisionEnabled() end

---@return boolean
function Primitive3D:AreOverlapsEnabled() end

---@return number
function Primitive3D:GetMass() end

---@return number
function Primitive3D:GetLinearDamping() end

---@return number
function Primitive3D:GetAngularDamping() end

---@return number
function Primitive3D:GetRestitution() end

---@return number
function Primitive3D:GetFriction() end

---@return number
function Primitive3D:GetRollingFriction() end

---@return Vector
function Primitive3D:GetLinearFactor() end

---@return Vector
function Primitive3D:GetAngularFactor() end

---@return integer
function Primitive3D:GetCollisionGroup() end

---@return integer
function Primitive3D:GetCollisionMask() end

---@param value number
function Primitive3D:SetMass(value) end

---@param value number
function Primitive3D:SetLinearDamping(value) end

---@param value number
function Primitive3D:SetAngularDamping(value) end

---@param value number
function Primitive3D:SetRestitution(value) end

---@param value number
function Primitive3D:SetFriction(value) end

---@param value number
function Primitive3D:SetRollingFriction(value) end

---@param value Vector
function Primitive3D:SetLinearFactor(value) end

---@param value Vector
function Primitive3D:SetAngularFactor(value) end

---@param value integer
function Primitive3D:SetCollisionGroup(value) end

---@param value integer
function Primitive3D:SetCollisionMask(value) end

---@return Vector
function Primitive3D:GetLinearVelocity() end

---@return Vector
function Primitive3D:GetAngularVelocity() end

---@param delta Vector
function Primitive3D:AddLinearVelocity(delta) end

---@param delta Vector
function Primitive3D:AddAngularVelocity(delta) end

---@param value Vector
function Primitive3D:SetLinearVelocity(value) end

---@param value Vector
function Primitive3D:SetAngularVelocity(value) end

---@param delta Vector
function Primitive3D:AddForce(delta) end

---@param delta Vector
function Primitive3D:AddImpulse(delta) end

function Primitive3D:ClearForces() end

---@param enable boolean
function Primitive3D:EnableCastShadows(enable) end

---@return boolean
function Primitive3D:ShouldCastShadows() end

---@param enable boolean
function Primitive3D:EnableReceiveShadows(enable) end

---@return boolean
function Primitive3D:ShouldReceiveShadows() end

---@param enable boolean
function Primitive3D:EnableReceiveSimpleShadows(enable) end

---@return boolean
function Primitive3D:ShouldReceiveSimpleShadows() end

---@return integer
function Primitive3D:GetLightingChannels() end

---@param value integer
function Primitive3D:SetLightingChannels(value) end

---@param pos Vector
---@return number
function Primitive3D:SweepToWorldPosition(pos) end

---@return number
function Primitive3D:GetBounds() end

---@param pos Vector
---@return number
function Primitive3D:SweepToPosition(pos) end
