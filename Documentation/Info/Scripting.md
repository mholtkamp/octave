# Scripting

## Overview

Scripts are written in Lua.
Any node can have a single script assigned to it.

For a simple script example, look at [Rotator.lua](../../Engine/Scripts/Rotator.lua)

For a more complex script example, check out [FirstPersonController.lua](../../Engine/Scripts/FirstPersonController.lua)

In your script, you must define a table with the same name as the file. For example, the first line in Rotator.lua is `Rotator = {}`.

Scripts must be placed inside your project's Script directory.

Script files (and thus tables) must have unique names, even if they are contained in separate subdirectories. For instance, you cannot have both `Scripts/Test.lua` and `Scripts/Test/Test.lua`.

When assigning a script to a node, you must provide the path relative to the Script directory. You may omit the .lua extension. For instance, if you have a Player.lua script in your Scripts directory, then you can assign `Player` to the "Script" field on a node. But if you have a script file in Scripts/Monsters/Goblin.lua, then you must assign `Monsters/Goblin` to the "Script" field.

As a general rule of thumb, when placing a script on multiple nodes, you should place them on nodes of the same type. So for instance, if you placed a script on a Sphere3D, you should generally only place that script on other Sphere3D nodes. Now, let's say you have an EventTrigger.lua file, and you want to place it on different shape nodes like Sphere3D and Box3D. It's okay to do that, as long as you don't attempt to call any functions that are specific to Box3D or Sphere3D. The reason for this is that your custom script table will inherit from whichever node you place it on first. For example, if you assign your EventTrigger.lua on a Box3D node first, your EventTrigger {} table will "inherit" from Box3D. If you then assign EventTrigger.lua onto a Sphere3D node, EventTrigger will still inherit from Box3D and you will receive a warning "Multiple inheritance chains used for same script". Calling the function self:GetExtents() would be valid, but calling self:GetRadius() would throw an error.


## Callbacks

---
`Create()`

Called when the script is first created, before any properties have been serialized.

---
`Awake()`

Called after script properties have been serialized.

---
`Start()`

Called on the first frame that this script will Tick().

---
`Stop()`

Called when the script is destroyed.

---
`Tick(deltaTime)`

Called every frame during gameplay. deltaTime is a number and represents the time in seconds since the last frame. 

---
`EditorTick(deltaTime)`

Similar to tick, but is called in the editor while not playing.

---
`BeginOverlap(thisNode, otherNode)`

Called when a node begins to overlap this node or one of its descendants. thisNode and otherNode are both Primitive3Ds.

---
`EndOverlap(thisNode, otherNode)`

Called when a node stops overlapping this node or one of its descendants. thisNode and otherNode are both Primitive3Ds.

---
`OnCollision(thisNode, otherNode, impactPoint, impactNormal)`

Called when a node collides with this node or one of its descendants. thisNode and otherNode are both Primitive3Ds. impactPoint is a Vector containing the world-space position of impact. impactNormal is a Vector containing the world-space normal of impact.

---
`GatherProperties()`

This function allows you to define custom properties that will be inspectable and modifiable in the Editor. These properties will also be serialized when saving/loading a scene.

Example:
```lua
function Bomber:GatherProperties()
    return
    {
        { name = "moveSpeed", type = DatumType.Float },
        { name = "bombScene", type = DatumType.Asset },
    }
end
```

See [DatumType](../Lua/Misc/Enums.md#datumtype) for a list of possible property types.

---
`GatherReplicatedData()`

This function allows you to define properties that will be replicated from the server to clients in a network multiplayer game.

```lua
function Bomber:GatherReplicatedData()
    return 
    {
        { name = 'netYaw', type = DatumType.Float },
        { name = 'netPosition', type = DatumType.Vector, onRep = 'OnRep_netPosition'},
        { name = 'curMoveSpeed', type = DatumType.Float },
        { name = 'bombCount', type = DatumType.Byte },
        { name = 'bombRange', type = DatumType.Byte },
        { name = 'moveSpeed', type = DatumType.Float },
    }
end
```

See [DatumType](../Lua/Misc/Enums.md#datumtype) for a list of possible property types.

Providing a function name to the onRep key will cause that function to be called whenever that variable is replicated on the client, allowing you to react to changes.

---
`GatherNetFuncs()`

This function allows you to define remote procedure calls that can be invoked across the network.

```lua
function Bomber:GatherNetFuncs()
    return 
    {
        { name = 'S_PlantBomb', type = NetFuncType.Server, reliable = true},
        { name = 'S_SwingCane', type = NetFuncType.Server, reliable = true},
        { name = 'S_SyncTransform', type = NetFuncType.Server, reliable = false},
        { name = "C_ForceWorldPosition", type = NetFuncType.Client, reliable = true},
        { name = 'M_SwingCane', type = NetFuncType.Multicast, reliable = false},
    }
end
```

See [NetFuncType](../Lua/Misc/Enums.md#netfunctype) for a description of the different RPC types.

---
`OwnerChanged()`

Called in network multiplayer games whenever the owning host of this node changes.
