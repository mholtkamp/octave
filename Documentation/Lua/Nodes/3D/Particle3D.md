# Particle3D

A node that will simulate and render particles.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)

---
### Reset
Reset the particle system. Destroys all particles and sets the elapsed time and loop count back to 0.

Sig: `Particle3D:Reset()`

---
### EnableEmission
Enable the particle node to spawn new particles. Disabling emission will also reset the loop count and elapsed time to 0.

Sig: `Particle3D:EnableEmission(emit)`
 - Arg: `boolean emit` Whether to emit new particles
---
### IsEmissionEnabled
Check if the particle node is emitting new particles.

Sig: `emit = Particle3D:IsEmissionEnabled()`
 - Ret: `boolean emit` Whether particle is emitting new particles
---
### EnableSimulation
Set whether the particle node should simulate particles. Disabling simulation will essentially freeze particles.

Sig: `Particle3D:EnableSimulation(simulate)`
 - Arg: `boolean simulate` Whether to simulate particles
---
### IsSimulationEnabled
Check if particle simulation is enabled.

Sig: `simulate = Particle3D:IsSimulationEnabled()`
 - Ret: `boolean simulate` Whether to simulate particles
---
### EnableAutoEmit
Set the particle to automatically emit particles on Start().

Sig: `Particle3D:EnableAutoEmit(autoEmit)`
 - Arg: `boolean autoEmit` Whether to automatically begin emission on start
---
### IsAutoEmitEnabled
Check if the particle is set to automatically emit particles on Start().

Sig: `autoEmit = Particle3D:IsAutoEmitEnabled()`
 - Ret: `boolean autoEmit` Whether to automatically begin emission on start
---
### GetElapsedTime
Get the amount of elapsed time since the particle began emission.

Sig: `time = Particle3D:GetElapsedTime()`
 - Ret: `number time` Elapsed time
---
### SetParticleSystem
Set the particle system asset (template) that this node will use for simulating / rendering particles.

Sig: `Particle3D:SetParticleSystem(system)`
 - Arg: `ParticleSystem system` Particle system asset
---
### GetParticleSystem
Get the particle system asset (template) that this node will use for simulating / rendering particles.

Sig: `system = Particle3D:GetParticleSystem()`
 - Ret: `ParticleSystem system` Particle system asset
---
### SetMaterialOverride
Set a material override that this particle should use. If no override is assigned, the material assigned to the Particle System asset will be used.

Sig: `Particle3D:SetMaterialOverride(material)`
 - Arg: `Material material` Override material
---
### GetMaterial
Get the material used for rendering.

Sig: `material = Particle3D:GetMaterial()`
 - Ret: `Material material` Material for rendering
---
### GetMaterialOverride
Get the material override assigned to this node. If no override is assigned, the material assigned to the Particle System asset will be used.

Sig: `material = Particle3D:GetMaterialOverride()`
 - Ret: `Material material` Override material
---
### SetTimeMultiplier
Set a time multiplier to speed up / slow down the simulation.

Sig: `Particle3D:SetTimeMultiplier(mult)`
 - Arg: `number mult` Time multiplier
---
### GetTimeMultiplier
Get the current time multiplier.

Sig: `mult = Particle3D:GetTimeMultiplier()`
 - Ret: `number mult` Time multiplier
---
### SetUseLocalSpace
Set whether the particle transforms should be updated in world space or local space. If a particle node is set to use local space, then adjusting this node's transform will affect the transform of all it's currently spawned particles.

Sig: `Particle3D:SetUseLocalSpace(localSpace)`
 - Arg: `boolean localSpace` Update particles in local space
---
### GetUseLocalSpace
Get whether the particle transforms should be updated in world space or local space. If a particle node is set to use local space, then adjusting this node's transform will affect the transform of all it's currently spawned particles.

Sig: `localSpace = Particle3D:GetUseLocalSpace()`
 - Ret: `boolean localSpace` Update particles in local space
---
### GetNumParticles
Get the current number of live particles.

Sig: `numParticles = Particle3D:GetNumParticles()`
 - Ret: `number numParticles` Number of particles
---
### GetParticleData
Get the particle data of a specific particle.

Sig: `data = Particle3D:GetParticleData(index)`
 - Arg: `integer index` Particle index
 - Ret: `table data` Particle data
   - `Vector position`
   - `Vector velocity`
   - `Vector size`
   - `number elapsedTime`
   - `number lifeTime`
   - `number rotationSpeed`
   - `number rotation`
---
### SetParticleData
Set the particle data for a specific particle.

Sig: `Particle3D:SetParticleData(index, data)`
 - Arg: `integer index` Particle index
 - Arg: `table data` Particle data
   - `Vector position`
   - `Vector velocity`
   - `Vector size`
   - `number elapsedTime`
   - `number lifeTime`
   - `number rotationSpeed`
   - `number rotation`
---
### SetParticleOrientation
Set the particle orientation (of all particles in the system).

See [ParticleOrientation](../../Misc/Enums.md#particleorientation)

Sig: `Particle3D:SetParticleOrientation(orientation)`
 - Arg: `ParticleOrientation(integer) orientation` Particle orientation
---
### GetParticleOrientation
Get the particle orientation (of all particles in the system).

See [ParticleOrientation](../../Misc/Enums.md#particleorientation)

Sig: `orientation = Particle3D:GetParticleOrientation()`
 - Ret: `ParticleOrientation(integer) orientation` Particle orientation
---
### EnableAutoDestroy
Enable the particle to automatically destroy itself after its duration has finished.

Sig: `Particle3D:EnableAutoDestroy(autoDestroy)`
 - Arg: `boolean autoDestroy` Enable auto-destroy
---
### InstantiateParticleSystem
Create a new instance of the particle system asset this node is currently using. This can be useful for adjusting particle system settings for a specific node without changing all other particles using the same ParticleSystem asset.

Sig: `instance = Particle3D:InstantiateParticleSystem()`
 - Ret: `ParticleSystemInstance instance` New particle system instance
---
