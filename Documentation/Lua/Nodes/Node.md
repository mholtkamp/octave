# Node

The base object that can be added to a world and saved in a scene.

---

### Construct
Construct a node from a given class name. (e.g. "Node3D", "Primitive3D", "Text", "Image")

Alias: `New`

Sig: `node = Node.Construct(className="Node")`
 - Arg: `string className` The class of the node to create. Default = "Node".
 - Ret: `Node node` The newly constructed node.
---
### Destruct
Destruct a node, freeing the memory and making it unusable.

Sig: `Node.Destruct(node)`
 - Arg: `Node node` The node to be destroyed.
---
### IsValid
Check whether a node is valid. This function currently always returns true, but when the engine is compiled using LUA_SAFE_NODE, this function will return true only if the underlying node reference is valid.

Sig: `valid = Node:IsValid()`
 - Ret: `boolean valid` Whether the node is valid.
---
### GetName
Get this node's name. A node's name must be unique among all of its siblings.

Sig: `name = Node:GetName()`
 - Ret: `string name` The node's name
---
### SetName
Set this node's name. A node's name must be unique among all of its siblings. If a sibling node already has the same name, then SetName() will append a unique identifier so that it can be differentiated.

Sig: `Node:SetName(name)`
 - Arg: `string name` The node's new name
---
### SetActive
Control whether the node is active. Active nodes will tick and recursively tick their children. If a parent node is not active, but the child node is active, the child node will never tick. 

Sig: `Node:SetActive(active)`
 - Arg: `boolean active` New active status

---
### IsActive
Check if the node is currently active. An optional boolean can be passed to check if any ancestor node is inactive, essentially making this node inactive.

Sig: `active = Node:IsActive(recurse=false)`
 - Arg: `boolean recurse` Recursively check ancestors
 - Ret: `boolean active` Active status
---
### SetVisible
Sets the visibility of the node. Only visible nodes will be rendered. If a node is not visible, none of its children nodes will be rendered either.

Sig: `Node:SetVisible(visible)`
 - Arg: `boolean visible` Visible status
---
### IsVisible
Checks if the node is visible. An optional boolean can be passed to check if any ancestor node is not visible, essentially making this node not visible.

Sig: `visible = Node:IsVisible(recurse=false)`
 - Arg: `boolean recurse` Recursively check ancestors
 - Ret: `boolean visible` Visible status
---
### GetWorld
Returns the World that the node is currently in. Returns nil if not part of a World.

Sig: `world = Node:GetWorld()`
 - Ret: `World world` Current world 
---
### GetParent
Returns this node's parent node.

Sig: `parent = Node:GetParent()`
 - Ret: `Node parent` This node's parent node 
---
### Attach
Attach this node to a new parent node.

Sig: `Node:Attach(parent, keepWorldTransform=false, childIndex=0)`
 - Arg: `Node parent` The new parent. Can be nil.
 - Arg: `boolean keepWorldTransform` Option to keep the same world-space transform. Useful when you want to reparent something while keeping it in the same position.
 - Arg: `integer childIndex` Where in the parent's child array this node should be placed. If 0, this node will be placed at the end of the parent node's child array. 
---
### Detach
Detach this node from its parent. This is essentially the same thing as calling Node:Attach(nil).

Sig: `Node:Detach(keepWorldTransform=false)`
 - Arg: `boolean keepWorldTransform` Option to keep the same world-space transform. Useful when you want to reparent something while keeping it in the same position.
---
### GetChild
Get a child node by its index.

Sig: `child = Node:GetChild(index)`
 - Arg: `integer index` Which child to get
 - Ret: `Node child` The child node. Will be nil if the index is out of range.
---
### GetChildByType
Get the first child node of a given type.

Sig: `child = Node:GetChildByType(typeName)`
 - Arg: `string typeName` The typename of child (e.g. Node3D, Primitive3D, Text, Image...).
 - Ret: `Node child` The child node (nil if none of the children match the type).
---
### GetNumChildren
Return the number of children this node has.

Sig: `numChildren = Node:GetNumChildren()`
 - Ret: `integer numChildren` Number of children
---
### AddChild
Parent the given node to this node. This is equivalent to calling newChild:Attach(thisNode).

Sig: `Node:AddChild(newChild, index=0)`
 - Arg: `Node newChild` The new child node you want to add
 - Arg: `integer index` The child index the new node should be placed at. If 0, it will be placed at the end.
---
### RemoveChild
Remove a child node.

Sig: `Node:RemoveChild(childIndexOrName)`
 - Arg: `integer/string childIndexOrName` Either an integer index, or a string name to identify which child should be removed.
---
### FindChild
Find a child node by its name. An optional recurse argument can be provided to check all children recursively.

Sig: `child = Node:FindChild(name, recurse=false)`
 - Arg: `string name` Name of the child to find
 - Arg: `boolean recurse` Whether to recursively search all children. If false (the default value), only the immediate children will be searched. 
 - Ret: `Node child` The found child node. Will return nil if no child was found.
---
### FindChildWithTag
Find the first child node with a given tag. An optional recurse argument can be provided to check all children recursively.

Sig: `child = Node:FindChild(tag, recurse=false)`
 - Arg: `string tag` Tag to search for
 - Arg: `boolean recurse` Whether to recursively search all children. If false (the default value), only the immediate children will be searched. 
 - Ret: `Node child` The found child node. Will return nil if no child was found.
---
### FindDescendant
Find a descendant node by name. This is equivalent to calling Node:FindChild(name, true).

Sig: `descendant = Node:FindDescendant(name)`
 - Arg: `string name` Descendant name
 - Ret: `Node descendant` Descandant node if found (otherwise nil)
---
### FindAncestor
Find an ancestor node by name.

Sig: `ancestor = Node:FindAncestor(name)`
 - Arg: `string name` Ancestor name 
 - Ret: `Node ancestor` Ancestor node if found (otherwise nil)
---
### HasAncestor
Check if the given node is an ancestor of this node.

Sig: `hasAncestor = Node:HasAncestor(node)`
 - Arg: `Node node` Node to check 
 - Ret: `boolean hasAncestor` True if node was an ancestor of self 
---
### GetRoot
Return the root ancestor node of this node (i.e. the first node with no parent).

Sig: `root = Node:GetRoot()`
 - Ret: `Node root` The root node of this node's tree
---
### IsWorldRoot
Checks to see if this node is the root node of the world it is currently in.

Sig: `isRoot = Node:IsWorldRoot()`
 - Ret: `boolean isRoot` true if node is in a world and is the world's root node, otherwise false.
---
### Traverse
Execute a function for this node and it's descendants. An example that prints out all of the node tree's names is given below:
```
function PrintNodeNames(node)
    Log.Debug(node:GetName())
    return true
end

node:Traverse(PrintNodeNames)
```
If false is returned by the passed in function, it indicates that the Traverse function should not call the function on its children.
Another example where we only print out the names of visible nodes:
```
function PrintVisibleNodeNames(node)
    if (node:IsVisible()) then
        Log.Debug(node:GetName())
        return true
    end
    
    return false
end

node:Traverse(PrintVisibleNodeNames)
```

Sig: `Node:Traverse(func, inverted=false)`
 - Arg: `function func` Function to invoke on each node
 - Arg: `boolean inverted` If true invoke function on leaf nodes first, and work backwards to the self node
---
### ForEach
Execute a function on this node and its children, similarly to Traverse(), except the passed in function should return true to continue execution on the next node and false to break out of the iteration entirely (not just skip over children).

Sig: `success = Node:ForEach(func, inverted=false)`
 - Arg: `function func` Function to invoke on each node
 - Arg: `boolean inverted` If true invoke function on leaf nodes first, and work backwards to the self node
 - Ret: `boolean success` If true, all nodes were iterated over. If false, the execution was broken early

---
### CreateChild
Construct a new node and parent it to this node.

Sig: `child = Node:CreateChild(className)`
 - Arg: `string className` Class name of node type to construct (e.g. Primitive3D, Image)
 - Ret: `Node child` The newly created child (nil if failed or an invalid class name was provided)
---
### CreateChildClone
Construct a new node cloned from a source node and parent it to this node.

Sig: `child = Node:CreateChildClone(srcNode, recurse)`
 - Arg: `Node srcNode` The source node to copy from
 - Arg: `boolean recurse` Whether child nodes should be recursively created and copied
 - Ret: `Node child` The newly cloned node
---
### Clone
Create a copy of node. If the recurse option is set to true, it will copy all child nodes as well.

Sig: `node = Node:Clone(srcNode, recurse)`
 - Arg: `Node srcNode` The source node to copy from
 - Arg: `boolean recurse` If true, clone all children nodes as well
 - Ret: `Node node` Newly created, cloned node
---
### DestroyChild
Destroy a child node based on a given name or index.
Note: This function seems excessive. Can we remove it?

Sig: `Node:DestroyChild(nameOrIndex)`
 - Arg: `string/integer nameOrIndex` Name (string) or index (integer) of child node to destroy
---
### DestroyAllChildren
Destroy all child nodes.

Sig: `Node:DestroyAllChildren()`

---
### Start
Manually invoke the Start function of the node if it hasn't been started already. Normally a node will have its Start() function called on the first frame it ticks, but sometimes it can be useful to manually invoke Start() to control the order that nodes are started.

Sig: `Node:Start()`

---
### HasStarted
Check if the node has already started (i.e. has had its Start() function called)

Sig: `hasStarted = Node:HasStarted()`
 - Ret: `boolean hasStarted` true if Start() has been called for this node
---
### SetPendingDestroy
Mark the node to be destroyed at the end of the frame.

Sig: `Node:SetPendingDestroy(destroy)`
 - Arg: `boolean destroy` true to mark for destruction
---
### IsPendingDestroy
Check if the node is marked to be destroyed at the end of the frame.

Sig: `pendingDestroy = Node:IsPendingDestroy()`
 - Ret: `boolean pendingDestroy` true if marked for destruction 
---
### EnableTick
Set whether or not this node should tick every frame.
Note: I don't think there is any difference between this and SetActive() right now, so this function might be removed in the future.

Sig: `Node:EnableTick(enable)`
 - Arg: `boolean enable` Whether to tick
---
### IsTickEnabled
Check if this node will have its Tick() function called.
Note: I don't think there is any difference between this and IsActive() right now, so this function might be removed in the future.

Sig: `enabled = Node:IsTickEnabled()`
 - Ret: `boolean enabled` true if can tick
---
### GetScene
Return this node's associated Scene. When a scene is instantiated, the newly instantiated root node will hold a reference to the Scene asset which it was instantiated from. GetScene() will return this reference.

Sig: `scene = Node:GetScene()`
 - Arg: `Scene scene` The node's associated scene
---
### GetNetId
Return the net id associated with this node. A NetId will be assigned to nodes with replication enabled when hosting a session or connected to another session. NetIds are the internal mechanism for referencing the same node across different instances of the game.

Sig: `netId = Node:GetNetId()`
 - Arg: `integer netId` The node's NetId, 0 is an invalid net id
---
### GetOwningHost
Return the owning host id. Only useful in net play sessions for determining which player/host should control the node.

Sig: `hostId = Node:GetOwningHost()`
 - Ret: `integer hostId` Owning host id
---
### SetOwningHost
Set which host should own the node. The owning host id will be replicated to clients, so this function should only be called on the server. Ownership doesn't do anything on it's own, it's up to the game code to handle what ownership means (e.g. only the owning player can tell the player node where to move)

Sig: `Node:SetOwningHost(hostId)`
 - Arg: `integer hostId` The new host id that should own the node
---
### SetReplicate
Enable or disable network replication for this node.
Note: The replicate flag should probably be set in the Editor via the Properties panel. I don't think setting it at runtime is advised.

Sig: `Node:SetReplicate(replicate)`
 - Arg: `boolean replicate` true to replicate over the network
---
### IsReplicated
Check if this node is replicated over the network.

Sig: `replicated = Node:IsReplicated()`
 - Ret: `boolean replicated` true if replicated over the network
---
### SetReplicateTransform
Set whether the transform (position/rotation/scale) should be replicated for this node. This is currently only used by 3D nodes, but in the future it might be expanded to work for widgets or 2D nodes.
Note: Should probably be set in the editor instead of changed at runtime.

Sig: `Node:SetReplicateTransform(replicateTransform)`
 - Arg: `boolean replicateTransform` true to replicate transform
---
### IsTransformReplicated
Check if this node's transform is replicated.

Sig: `transformReplicated = Node:IsTransformReplicated()`
 - Ret: `boolean transformReplicated` true if transform is replicated
---
### ForceReplication
Forcefully replicate properties of this node on the next network update, even if nothing has changed.

Sig: `Node:ForceReplication()`

---
### HasTag
Check if a node has a given tag assigned to it. Tags are strings that can be assigned to a node. Nodes can have an unlimited number of tags assigned to them.

Sig: `hasTag = Node:HasTag(tag)`
 - Arg: `string tag` tag
 - Ret: `boolean hasTag` true if the node has that tag assigned to it
---
### AddTag
Add a tag to this node. Nothing will happen if this node already has the tag assigned to it.

Sig: `Node:AddTag(tag)`
 - Arg: `string tag` Tag to assign
---
### RemoveTag
Remove a tag on this node. Does nothing if the tag hasn't been assigned.

Sig: `Node:RemoveTag(tag)`
 - Arg: `string tag` Tag to remove
---
### HasAuthority
This function was added to match Unreal functionality, but the server will have network authority on all nodes. This function calls Network.IsAuthority(), it does not really matter which node this function is invoked on.

Sig: `hasAuthority = Node:HasAuthority()`
 - Arg: `boolean hasAuthority` true if this host is a server
---
### IsOwned
Check whether this computer host owns this node. Will always return true if in a local game. Otherwise will return true if the OwningHost is equivalent to this computer's host id.

Sig: `owned = Node:IsOwned()`
 - Ret: `boolean owned` true if owned by this host
---
### IsLateTickEnabled
Check if late tick is enabled. Late tick means that the node will tick all of its children before it ticks itself. By default, the parent node will tick first.

Sig: `lateTick = Node:IsLateTickEnabled()`
 - Ret: `boolean lateTick` true if node will tick after its children
---
### EnableLateTick
Enable late tick on this node. Late tick means that the node will tick all of its children before it ticks itself. By default, the parent node will tick first.

Sig: `Node:EnableLateTick(lateTick)`
 - Arg: `boolean lateTick` true to tick this node after its children
---
### InvokeNetFunc
Used to invoke a remote procedure call on this node. Up to 8 arguments can be passed.

Sig: `Node:InvokeNetFunc(name, arg1, arg2, ... arg8)`
 - Arg: `string name` name of net func (must exist in GatherNetFuncs())
---
### CheckType
Check if this node is of a given type.

Alias: `Is`

Alias: `IsA`

Sig: `Node:CheckType(className)`
 - Arg: `string className` type name to check (e.g. "Primitive3D")
---
