# ParticleSystemInstance

An instance of a ParticleSystem asset. Right now it's essentially a copy of a ParticleSystem, but in a future where you can use your own custom compute shaders, a ParticleSystemInstance would allow you to override shader parameters.

Inheritance:
* [Asset](Asset.md)
* [ParticleSystem](ParticleSystem.md)

---
### ParticleSystemInstance.Create
Create a new particle system instance from a ParticleSystem source asset.

Sig: `inst = ParticleSystemInstance.Create(src)`
 - Arg: `ParticleSystem src` Source particle system asset
 - Ret: `ParticleSystemInstance inst` Newly created instance
---
