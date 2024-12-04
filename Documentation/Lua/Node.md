# Node

### Node.Construct
Construct a node from a given class name. (e.g. "Node3D", "Primitive3D", "Text", "Image")

Sig: `node = Node.Construct(className="Node")`
 - Arg: `string className` The class of the node to create. Default = "Node".
 - Ret: `Node node` The newly constructed node.
---
### Node.Destruct
Destruct a node, freeing the memory and making it unsuable.

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
Sets the visibility of the node. Only visible nodes will be rendered. If a node is not visible none of its children nodes will be rendered either.

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
---
### DestroyChild
---
### DestroyAllChildren
---
### Start
---
### HasStarted
---
### Destroy
---
### SetPendingDestroy
---
### IsPendingDestroy
---
### EnableTick
---
### IsTickEnabled
---
### GetScene
---
### GetNetId
---
### GetOwningHost
---
### SetOwningHost
---
### SetReplicate
---
### IsReplicated
---
### SetReplicateTransform
---
### IsTransformReplicated
---
### ForceReplication
---
### HasTag
---
### AddTag
---
### RemoveTag
---
### HasAuthority
---
### IsOwned
---
### IsLateTickEnabled
---
### EnableLateTick
---
### InvokeNetFunc
---
### CheckType
---
