# NodeRef

A reference to a node. If the node is destroyed, this reference will be automatically updated to be nil. NodeRefs offer a safe way to store a reference to a node. If you use a normal Node variable, you may get a crash if you attempt to use it after it has been destroyed. I think in the future, all Node userdata objects might become NodeRefs so that you don't have to worry about it. However, I need to test the performance impact this might have.

---
### Create
Create a NodeRef.

Sig: `nodeRef = NodeRef.Create(srcNode=nil)`
 - Arg: `Node srcNode` Source node to reference
 - Ret: `NodeRef nodeRef` Newly created NodeRef 
---
### Get
Get the Node being referenced.

Sig: `node = NodeRef:Get()`
 - Ret: `Node node` Referenced node (may be nil)
---
### Set
Set the Node to reference.

Sig: `NodeRef:Set(node)`
 - Arg: `Node node` Node to reference
---
