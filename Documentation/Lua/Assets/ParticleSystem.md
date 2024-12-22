# ParticleSystem

An asset that describes how a particle node should emit particles.

Inheritance:
* [Asset](Asset.md)

---
### SetMaterial
Set the material that should be used for rendering.

Sig: `ParticleSystem:SetMaterial(material)`
 - Arg: `Material material` Material to use
---
### GetMaterial
Get the material that is used for rendering.

Sig: `material = ParticleSystem:GetMaterial()`
 - Ret: `Material material` Material for rendering
---
### SetDuration
Set the duration. A duration of 0 means infinite duration.

Sig: `ParticleSystem:SetDuration(duration)`
 - Arg: `number duration` System duration
---
### GetDuration
Get the duration. A duration of 0 means infinite duration.

Sig: `duration = ParticleSystem:GetDuration()`
 - Ret: `number duration` System duration
---
### SetSpawnRate
Set the particle spawn rate (how many particles should be spawned per second).

Sig: `ParticleSystem:SetSpawnRate(rate)`
 - Arg: `number rate` Spawn rate
---
### GetSpawnRate
Get the particle spawn rate (how many particles should be spawned per second).

Sig: `rate = ParticleSystem:GetSpawnRate()`
 - Ret: `number rate` Spawn rate
---
### SetBurstCount
Set the burst count.

Sig: `ParticleSystem:SetBurstCount(count)`
 - Arg: `integer count` Burst count
---
### GetBurstCount
Get the burst count.

Sig: `count = ParticleSystem:GetBurstCount()`
 - Ret: `integer count` Burst count
---
### SetBurstWindow
Set the burst window in seconds.

Sig: `ParticleSystem:SetBurstWindow(window)`
 - Arg: `number window` Burst window
---
### GetBurstWindow
Get the burst window in seconds.

Sig: `window = ParticleSystem:GetBurstWindow()`
 - Ret: `number window` Burst window
---
### SetMaxParticles
Set the maximum number of particles. New particles will not be emitted if the system has reached its max particle count.

Sig: `ParticleSystem:SetMaxParticles(max)`
 - Arg: `integer max` Maximum number of particles
---
### GetMaxParticles
Get the maximum number of particles. New particles will not be emitted if the system has reached its max particle count.

Sig: `max = ParticleSystem:GetMaxParticles()`
 - Ret: `integer max` Maximum number of particles
---
### SetLoops
Set the number of loops. 0 = infinite loops.

Sig: `ParticleSystem:SetLoops(loops)`
 - Arg: `integer loops` Num loops
---
### GetLoops
Get the number of loops. 0 = infinite loops.

Sig: `loops = ParticleSystem:GetLoops()`
 - Ret: `integer loops` Num loops
---
### SetRadialVelocity
Set whether the particle system should use radial velocity. If using radial velocity, velocity will point away from the center and only the X component of the velocity is used.

Sig: `ParticleSystem:SetRadialVelocity(radial)`
 - Arg: `boolean radial` Use radial velocity
---
### IsRadialVelocity
Check whether the particle system should use radial velocity. If using radial velocity, velocity will point away from the center and only the X component of the velocity is used.

Sig: `radial = ParticleSystem:IsRadialVelocity()`
 - Ret: `boolean radial` Use radial velocity
---
### SetRadialSpawn
Set whether the particle system should spawn particles in a radius around the center. When using radial spawn, only the X component of the min/max position is used for positioning newly spawned particles.

Sig: `ParticleSystem:SetRadialSpawn(radial)`
 - Arg: `boolean radial` Radial spawn
---
### IsRadialSpawn
Check whether the particle system should spawn particles in a radius around the center. When using radial spawn, only the X component of the min/max position is used for positioning newly spawned particles.

Sig: `radial = ParticleSystem:IsRadialSpawn()`
 - Ret: `boolean radial` Radial spawn
---
### SetLockedRatio
Set whether the width/height ratio of particles should be locked.

Sig: `ParticleSystem:SetLockedRatio(locked)`
 - Arg: `boolean locked` Locked ratio
---
### IsRatioLocked
Check whether the width/height ratio of particles should be locked.

Sig: `locked = ParticleSystem:IsRatioLocked()`
 - Ret: `boolean locked` Locked ratio
---
### SetLifetime
Set the minimum and maximum particle lifetime.

Sig: `ParticleSystem:SetLifetime(min, max)`
 - Arg: `number min` Min lifetime
 - Arg: `number max` Max lifetime
---
### GetLifetimeMin
Get the minimum particle lifetime.

Sig: `min = ParticleSystem:GetLifetimeMin()`
 - Ret: `number min` Min lifetime
---
### GetLifetimeMax
Get the maximum particle lifetime.

Sig: `max = ParticleSystem:GetLifetimeMax()`
 - Ret: `number max` Max lifetime
---
### SetPosition
Set the minimum and maximum particle position.

Sig: `ParticleSystem:SetPosition(min, max)`
 - Arg: `Vector min` Min position
 - Arg: `Vector max` Max position
---
### GetPositionMin
Get the minimum particle position.

Sig: `min = ParticleSystem:GetPositionMin()`
 - Ret: `Vector min` Min position
---
### GetPositionMax
Get the maximum particle position.

Sig: `max = ParticleSystem:GetPositionMax()`
 - Ret: `Vector max` Max position
---
### SetVelocity
Set the minimum and maximum particle initial velocity.

Sig: `ParticleSystem:SetVelocity(min, max)`
 - Arg: `Vector min` Min velocity
 - Arg: `Vector max` Max velocity
---
### GetVelocityMin
Get the minimum particle initial velocity.

Sig: `min = ParticleSystem:GetVelocityMin()`
 - Ret: `Vector min` Min velocity
---
### GetVelocityMax
Get the maximum particle initial velocity.

Sig: `max = ParticleSystem:GetVelocityMax()`
 - Ret: `Vector max` Max velocity
---
### SetSize
Set the minimum and maximum particle initial size.

Sig: `ParticleSystem:SetSize(min, max)`
 - Arg: `Vector min` Min size
 - Arg: `Vector max` Max size
---
### GetSizeMin
Get the minimum particle initial size.

Sig: `min = ParticleSystem:GetSizeMin()`
 - Ret: `Vector min` Min size
---
### GetSizeMax
Get the maximum particle initial size.

Sig: `max = ParticleSystem:GetSizeMax()`
 - Ret: `Vector max` Max size
---
### SetRotation
Set the minimum and maximum initial particle rotation in radians.

Sig: `ParticleSystem:SetRotation(min, max)`
 - Arg: `number min` Min rotation in radians
 - Arg: `number max` Max rotation in radians
---
### GetRotationMin
Get the minimum initial particle rotation in radians.

Sig: `min = ParticleSystem:GetRotationMin()`
 - Ret: `number min` Min rotation in radians
---
### GetRotationMax
Get the maximum initial particle rotation in radians.

Sig: `max = ParticleSystem:GetRotationMax()`
 - Ret: `number max` Max rotation in radians
---
### SetRotationSpeed
Set the minimum and maximum particle rotation speed in radians/second.

Sig: `ParticleSystem:SetRotationSpeed(min, max)`
 - Arg: `number min` Min rotation speed in radians/s
 - Arg: `number max` Max rotation speed in radians/s
---
### GetRotationSpeedMin
Get the minimum particle rotation speed in radians/second.

Sig: `min = ParticleSystem:GetRotationSpeedMin()`
 - Ret: `number min` Min rotation speed in radians/s
---
### GetRotationSpeedMax
Get the maximum particle rotation speed in radians/second.

Sig: `max = ParticleSystem:GetRotationSpeedMax()`
 - Ret: `number max` Max rotation speed in radians/s
---
### SetAcceleration
Set the uniform particle acceleration.

Sig: `ParticleSystem:SetAcceleration(acceleration)`
 - Arg: `Vector acceleration` Uniform acceleration
---
### GetAcceleration
Get the uniform particle acceleration.

Sig: `acceleration = ParticleSystem:GetAcceleration()`
 - Ret: `Vector acceleration` Uniform acceleration
---
### SetAlphaEase
Set the alpha ease factor which will fade in and out particles. 0.1 ease means fade in during the first 10% of particle lifetime and fade out for the last 10% of the particle lifetime.

Sig: `ParticleSystem:SetAlphaEase(ease)`
 - Arg: `number ease` Alpha ease (0 - 0.5)
---
### GetAlphaEase
Get the alpha ease factor which will fade in and out particles. 0.1 ease means fade in during the first 10% of particle lifetime and fade out for the last 10% of the particle lifetime.

Sig: `ease = ParticleSystem:GetAlphaEase()`
 - Ret: `number ease` Alpha ease (0 - 0.5)
---
### SetScaleEase
Set the scale ease factor which will scale in and out particles. 0.1 ease means scale up during the first 10% of particle lifetime and scale down for the last 10% of the particle lifetime.

Sig: `ParticleSystem:SetScaleEase(ease)`
 - Arg: `number ease` Scale ease (0 - 0.5)
---
### GetScaleEase
Get the scale ease factor which will scale in and out particles. 0.1 ease means scale up during the first 10% of particle lifetime and scale down for the last 10% of the particle lifetime.

Sig: `ease = ParticleSystem:GetScaleEase()`
 - Ret: `number ease` Scale ease (0 - 0.5)
---
### SetColorStart
Set the starting particle color.

Sig: `ParticleSystem:SetColorStart(color)`
 - Arg: `Vector color` Start color
---
### GetColorStart
Get the starting particle color.

Sig: `color = ParticleSystem:GetColorStart()`
 - Ret: `Vector color` Start color
---
### SetColorEnd
Set the end particle color.

Sig: `ParticleSystem:SetColorEnd(color)`
 - Arg: `Vector color` End color
---
### GetColorEnd
Get the end particle color.

Sig: `color = ParticleSystem:GetColorEnd()`
 - Ret: `Vector color` End color
---
