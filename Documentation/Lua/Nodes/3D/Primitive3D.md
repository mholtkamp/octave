# Primitive3D

A base class for 3D nodes that are capable of using collision and/or being rendered.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)

---
### EnablePhysics
Whether this node should be physically simulated.

Sig: `Primitive3D:EnablePhysics(enable)`
 - Arg: `boolean enable` Enable physics simulation
---
### EnableCollision
Whether this node should be collidable. Objects that have collision enabled will affect what objects simulating physics will interact with, as well as what objects can be hit when performing Sweep/Ray tests.

Sig: `Primitive3D:EnableCollision(enable)`
 - Arg: `boolean enable` Enable collision
---
### EnableOverlaps
Whether this node should be able to trigger overlap events.

Sig: `Primitive3D:EnableOverlaps(enable)`
 - Arg: `boolean enable` Enable overlap events
---
### IsPhysicsEnabled
Check whether physics simulation is enabled.

Sig: `enabled = Primitive3D:IsPhysicsEnabled()`
 - Ret: `boolean enabled` Is physics simulation enabled
---
### IsCollisionEnabled
Check whether collision is enabled.

Sig: `enabled = Primitive3D:IsCollisionEnabled()`
 - Ret: `boolean enabled` Is collision enabled
---
### AreOverlapsEnabled
Check whether overlap events are enabled.

Sig: `enabled = Primitive3D:AreOverlapsEnabled()`
 - Ret: `boolean enabled` Are overlap events enabled
---
### GetMass
Get the mass of the node. Used for physics simulation only.

Sig: `mass = Primitive3D:GetMass()`
 - Ret: `number mass` Physical mass
---
### GetLinearDamping
Get the linear damping of the node. Used by physics simulation to control how the node's linear velocity should change over time.

Sig: `linearDamping = Primitive3D:GetLinearDamping()`
 - Ret: `number linearDamping` Linear damping
---
### GetAngularDamping
Get the angular damping of the node. Used by physics simulation to control how the node's angular velocity should change over time.

Sig: `angularDamping = Primitive3D:GetAngularDamping()`
 - Ret: `number angularDamping` Angular damping
---
### GetRestitution
Get the node's restitution (i.e. bounciness). 0 restitution = no bounce. 1 restitution = never lose speed after collision. Going above 1 might result in some crazy interactions. Used by physics simulation.

Sig: `restitution = Primitive3D:GetRestitution()`
 - Ret: `number restitution` Restitution (bounciness)
---
### GetFriction
Get the node's friction. Used by physics simulation.

Sig: `friction = Primitive3D:GetFriction()`
 - Ret: `number friction` Friction
---
### GetRollingFriction
Get the node's rolling friction. Used by physics simulation.

Sig: `rollingFriction = Primitive3D:GetRollingFriction()`
 - Ret: `number rollingFriction` Rolling friction
---
### GetLinearFactor
Get the node's linear factor, which controls what axes the node can move along. Used by physics simulation.

Sig: `factor = Primitive3D:GetLinearFactor()`
 - Ret: `Vector factor` Linear factor
---
### GetAngularFactor
Get the node's angular factor, which controls what axes the node can move along. Used by physics simulation.

Sig: `factor = Primitive3D:GetAngularFactor()`
 - Ret: `Vector factor` Angular factor
---
### GetCollisionGroup
Get the collision group of this node. The collision group is used in conjunction with the collision mask to determine which nodes can collide with each other (or overlap). There are only 8 collision groups because they are stored within one byte of data (0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80). It's possible for a node to be assigned to multiple groups by taking the "bitwise or" of multiple groups (e.g. 0x03 = groups 0x01 and 0x02).

Sig: `group = Primitive3D:GetCollisionGroup()`
 - Ret: `integer group` Collision group
---
### GetCollisionMask
Get this node's collision mask. The collision group is used in conjunction with the collision mask to determine which nodes can collide with each other (or overlap). Given two nodes nodeA and nodeB, they will overlap and collide only if `nodeA:GetCollisionMask() & nodeB:GetCollisionGroup()` and `nodeB:GetCollisionMask() & nodeA:GetCollisionGroup()`.

Sig: `mask = Primitive3D:GetCollisionMask()`
 - Ret: `integer mask` Collision mask
---
### SetMass
Set the mass of the node. Used for physics simulation only.

Sig: `Primitive3D:SetMass(mass)`
 - Arg: `number mass` Physical mass
---
### SetLinearDamping
Set the linear damping of the node. Used by physics simulation to control how the node's linear velocity should change over time.

Sig: `Primitive3D:SetLinearDamping(linearDamping)`
 - Arg: `number linearDamping` Linear damping
---
### SetAngularDamping
Set the angular damping of the node. Used by physics simulation to control how the node's angular velocity should change over time.

Sig: `Primitive3D:SetAngularDamping(angularDamping)`
 - Arg: `number angularDamping` Angular damping
---
### SetRestitution
Set the node's restitution (i.e. bounciness). 0 restitution = no bounce. 1 restitution = never lose speed after collision. Going above 1 might result in some crazy interactions. Used by physics simulation.

Sig: `Primitive3D:SetRestitution(restitution)`
 - Arg: `number restitution` Restitution (bounciness)
---
### SetFriction
Set the node's friction. Used by physics simulation.

Sig: `Primitive3D:SetFriction(friction)`
 - Arg: `number friction` Friction
---
### SetRollingFriction
Set the node's rolling friction. Used by physics simulation.

Sig: `Primitive3D:SetRollingFriction(rollingFriction)`
 - Arg: `number rollingFriction` Rolling friction
---
### SetLinearFactor
Set the node's linear factor, which controls what axes the node can move along. Used by physics simulation.

Sig: `Primitive3D:SetLinearFactor(factor)`
 - Arg: `Vector factor` Linear factor
---
### SetAngularFactor
Set the node's angular factor, which controls what axes the node can move along. Used by physics simulation.

Sig: `Primitive3D:SetAngularFactor(factor)`
 - Arg: `Vector factor` Angular factor
---
### SetCollisionGroup
Set the collision group of this node. The collision group is used in conjunction with the collision mask to determine which nodes can collide with each other (or overlap). There are only 8 collision groups because they are stored within one byte of data (0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80). It's possible for a node to be assigned to multiple groups by taking the "bitwise or" of multiple groups (e.g. 0x03 = groups 0x01 and 0x02).

Sig: `Primitive3D:SetCollisionGroup(group)`
 - Arg: `integer group` Collision group
---
### SetCollisionMask
Set this node's collision mask. The collision group is used in conjunction with the collision mask to determine which nodes can collide with each other (or overlap). Given two nodes nodeA and nodeB, they will overlap and collide only if `nodeA:GetCollisionMask() & nodeB:GetCollisionGroup()` and `nodeB:GetCollisionMask() & nodeA:GetCollisionGroup()`.

Sig: `Primitive3D:SetCollisionMask(mask)`
 - Arg: `integer mask` Collision mask
---
### GetLinearVelocity
Get the node's current linear velocity.

Sig: `velocity = Primitive3D:GetLinearVelocity()`
 - Ret: `Vector velocity` Linear velocity
---
### GetAngularVelocity
Get the node's current angular velocity.

Sig: `velocity = Primitive3D:GetAngularVelocity()`
 - Ret: `Vector velocity` Angular velocity
---
### AddLinearVelocity
Add linear velocity. Only used by physics simulation.

Sig: `Primitive3D:AddLinearVelocity(velocity)`
 - Arg: `Vector velocity` Linear velocity
---
### AddAngularVelocity
Add angular velocity. Only used by physics simulation.

Sig: `Primitive3D:AddAngularVelocity(velocity)`
 - Arg: `Vector velocity` Angular velocity
---
### SetLinearVelocity
Set the linear velocity. Only used by physics simulation.

Sig: `Primitive3D:SetLinearVelocity(velocity)`
 - Arg: `Vector velocity` Linear velocity
---
### SetAngularVelocity
Set angular velocity. Only used by physics simulation.

Sig: `Primitive3D:SetAngularVelocity(velocity)`
 - Arg: `Vector velocity` Angular velocity
---
### AddForce
Add a force to this node. Only used by physics simulation.

Sig: `Primitive3D:AddForce(force)`
 - Arg: `Vector force` Force
---
### AddImpulse
Add an impulse to this node. Only used by physics simulation.

Sig: `Primitive3D:AddImpulse(impulse)`
 - Arg: `Vector impulse` Impulse
---
### ClearForces
Clear all forces affecting this node. Only used by physics simulation.

Sig: `Primitive3D:ClearForces()`

---
### EnableCastShadows
Set whether this node should cast shadows. Projected shadows are only supported on Vulkan and aren't implemented yet.

Sig: `Primitive3D:EnableCastShadows(enable)`
 - Arg: `boolean enable` Enable shadow casting
---
### ShouldCastShadows
Check whether this node is set to cast shadows. Projected shadows are only supported on Vulkan and aren't implemented yet.

Sig: `castShadows = Primitive3D:ShouldCastShadows()`
 - Ret: `boolean castShadows` Should cast shadows
---
### EnableReceiveShadows
Set whether this node should be affected by projected shadows. Projected shadows are only supported on Vulkan and aren't implemented yet.

Sig: `Primitive3D:EnableReceiveShadows(enable)`
 - Arg: `boolean enable` Enable shadow receiving
---
### ShouldReceiveShadows
Check whether this node should be affected by projected shadows. Projected shadows are only supported on Vulkan and aren't implemented yet.

Sig: `receiveShadows = Primitive3D:ShouldReceiveShadows()`
 - Ret: `boolean receiveShadows` Should receive shadows
---
### EnableReceiveSimpleShadows
Enable this node to receive simple shadows from ShadowMesh3D nodes. Simple shadows are supported on all platforms.

Sig: `Primitive3D:EnableReceiveSimpleShadows(enable)`
 - Arg: `boolean enable` Enable receiving simple shadows
---
### ShouldReceiveSimpleShadows
Check whether this node should receive simple shadows from ShadowMesh3D nodes. Simple shadows are supported on all platforms.

Sig: `receive = Primitive3D:ShouldReceiveSimpleShadows()`
 - Ret: `boolean receive` Should receive simple shadows
---
### GetLightingChannels
Get this node's lighting channels. Only lights on the same lighting channel will be able to light this node.

Sig: `channels = Primitive3D:GetLightingChannels()`
 - Ret: `integer channels` Lighting channels
---
### SetLightingChannels
Set this node's lighting channels. Only lights on the same lighting channel will be able to light this node.

Sig: `Primitive3D:SetLightingChannels(channels)`
 - Arg: `integer channels` Lighting channels
---
### SweepToWorldPosition
Sweep this node to a new position, stopping at the first collision along the way. Only this node's collision will be used for the sweep (no children).
TODO: Add support for sweeping children as well.

Alias: `SweepToPosition`

Sig: `result = Primitive3D:SweepToWorldPosition(position, mask=0)`
 - Arg: `Vector position` World position
 - Arg: `integer mask` Collision mask to use (if 0, use this node's collision mask)
 - Ret: `table result` Result of the sweep test
   - `Vector start`
   - `Vector end`
   - `Primitive3D hitNode`
   - `Vector hitNormal`
   - `Vector hitPosition`
   - `number hitFraction` (0 to 1, how far along the sweep did we make it?)
---
