# Scene

An asset that describes a collection of nodes. It can be thought of as a Prefab, Level, Blueprint, or Collection in other game engines.

Inheritance:
* [Asset](Asset.md)

---
### Capture
Save a node tree into this scene asset.

Sig: `Scene:Capture(root)`
 - Arg: `Node root` The root node
---
### Instantiate
Create a node tree from the description saved in this asset.

Sig: `root = Scene:Instantiate()`
 - Ret: `Node root` The newly created root node
---
