# Globals

Global functions.

---
### RefSweep
Unload any assets that are no longer referenced.

Sig: `RefSweep()`

---
### GetAsset
Get an asset by name. Does not load it. Will return nil if the asset hasn't been loaded.

Sig: `asset = GetAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Asset
---
### LoadAsset
Load and get an asset immediately.

Sig: `asset = LoadAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Loaded asset
---
### AsyncLoadAsset
Request an asset be loaded asynchronously. This function will return a reference to an asset, and you can check if it has been loaded. Call `asset:IsLoaded()` to see if it has been loaded. TODO: Add function callback to handle when asset is loaded.

Sig: `asset = AsyncLoadAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Pending asset
---
### UnloadAsset
Unload an unreferenced asset.

Sig: `UnloadAsset(name)`
 - Arg: `string name` Asset name
---
