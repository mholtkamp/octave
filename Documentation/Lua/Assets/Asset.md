# Asset

Base class for any assets that can be loaded into memory from file. Most assets are created by importing source files in the editor (like a Texture by importing a .png file).

Assets can only be unloaded from memory once nothing else references them. To unload all unreferenced assets, call AssetManager.RefSweep(). In the future, options for incremental asset cleanup will be implemented.

---
### GetName
Get this asset's name.

Sig: `name = Asset:GetName()`
 - Ret: `string name` Asset name
---
### GetRefCount
Get the number of references that are pointing to this asset. These references include variables in both C++ and Lua code.

Sig: `count = Asset:GetRefCount()`
 - Ret: `integer count` Reference count
---
### GetTypeName
Get the asset's type as a string. For instance "Texture".

Sig: `typeName = Asset:GetTypeName()`
 - Ret: `string typeName` Asset type name
---
### IsTransient
Check if this asset is transient. A transient asset is not stored in the asset registry and cannot be saved to a file. Common transient assets may be LiteMaterial, MaterialInstance, and ParticleSystemInstance.

Sig: `transient = Asset:IsTransient()`
 - Ret: `boolean transient` Is a transient asset
---
### IsLoaded
Check if the asset is loaded. This is useful for checking if an asset is loaded after initiating an asynchronous load with AssetManager.AsyncLoadAsset().

Sig: `loaded = Asset:IsLoaded()`
 - Ret: `boolean loaded` Is asset loaded
---
