# AssetManager

System that manages loading and unloading assets.

Some of these functions are available as global functions, so you can just call `LoadAsset(name)` instead of `AssetManager.LoadAsset(name)`. See [Globals](../Misc/Globals.md).

---
### RefSweep
Unload any assets that are no longer referenced.

Sig: `AssetManager.RefSweep()`

---
### GetAsset
Get an asset by name. Does not load it. Will return nil if the asset hasn't been loaded.

Sig: `asset = AssetManager.GetAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Asset
---
### LoadAsset
Load and get an asset immediately.

Sig: `asset = AssetManager.LoadAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Loaded asset
---
### SaveAsset
Save an asset. Only available in Editor.

Sig: `AssetManager.SaveAsset(name)`
 - Arg: `string name` Asset name
---
### AsyncLoadAsset
Request that an asset be loaded asynchronously. This function will return a reference to an asset, and you can check if it has been loaded. Call `asset:IsLoaded()` to see if it has been loaded. TODO: Add function callback to handle when asset is loaded.

Sig: `asset = AssetManager.AsyncLoadAsset(name)`
 - Arg: `string name` Asset name
 - Ret: `Asset asset` Pending asset
---
### UnloadAsset
Unload an unreferenced asset.

Sig: `AssetManager.UnloadAsset(name)`
 - Arg: `string name` Asset name
---
### CreateAndRegisterAsset
Create an asset of a given type.

Sig: `asset = AssetManager.CreateAndRegisterAsset(typeName, path, name)`
 - Arg: `string typeName` Name of the type to create (e.g. MaterialLite)
 - Arg: `string path` Where the asset should be saved
 - Arg: `string name` Name of the new asset
 - Ret: `Asset asset` The newly created asset
---
