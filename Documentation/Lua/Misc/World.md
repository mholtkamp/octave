# World

A table that references a world and allows interacting with it.

---
### GetActiveCamera
Get the active camera. A world can only have one active camera at a time.

Sig: `camera = World:GetActiveCamera()`
 - Ret: `Camera3D camera` Active camera
---
### GetAudioReceiver
Get the active audio receiver. A world can only have one active audio receiver at a time. If no audio receiver is assigned, the active camera will be used as the audio receiver.

Sig: `receiver = World:GetAudioReceiver()`
 - Ret: `Node3D receiver` Audio receiver
---
### SetActiveCamera
Set the active camera. A world can only have one active camera at a time.

Sig: `World:SetActiveCamera(camera)`
 - Arg: `Camera3D camera` Active camera
---
### SetAudioReceiver
Set the active audio receiver. A world can only have one active audio receiver at a time. If no audio receiver is assigned, the active camera will be used as the audio receiver.

Sig: `World:SetAudioReceiver(receiver)`
 - Arg: `Node3D receiver` Audio receiver
---
### SpawnNode
Spawn a node from a given class. The spawned node will be attached to the world's root node. If there is no root node, this newly spawned node will become the world's root node.

Sig: `node = World:SpawnNode(className)`
 - Arg: `string className` Name of the class to spawn
 - Ret: `Node node` Newly created node
---
### SpawnScene
Spawn a scene. The spawned node will be attached to the world's root node. If there is no root node, this newly spawned node will become the world's root node.

Sig: `node = World:SpawnScene(sceneName)`
 - Arg: `string sceneName` Name of the scene asset to spawn
 - Ret: `Node node` Newly spawned node (root of spawned scene)
---
### GetRootNode
Get the world's root node.

Sig: `root = World:GetRootNode()`
 - Ret: `Node root` Root node
---
### SetRootNode
Set the world's root node.

Sig: `World:SetRootNode(root)`
 - Arg: `Node root` Root node
---
### DestroyRootNode
Destroy the world's root node.

Sig: `World:DestroyRootNode()`

---
### FindNode
Find a node by its name.

Sig: `node = World:FindNode(name)`
 - Arg: `string name` Node name
 - Ret: `Node node` Found node (or nil if it couldn't be found)
---
### FindNodesWithTag
Find all nodes with a given tag.

Sig: `nodes = World:FindNodesWithTag(tag)`
 - Arg: `string tag` Tag to search for
 - Ret: `table nodes` Array of Node elements
---
### FindNodesWithName
Find all nodes with a given name.

Sig: `nodes = World:FindNodesWithName(name)`
 - Arg: `string name` Name to search for
 - Ret: `table nodes` Array of Node elements
---
### SetAmbientLightColor
Set the world's ambient light color.

Sig: `World:SetAmbientLightColor(ambient)`
 - Arg: `Vector ambient` Ambient light color
---
### GetAmbientLightColor
Get the world's ambient light color.

Sig: `ambient = World:GetAmbientLightColor()`
 - Ret: `Vector ambient` Ambient light color
---
### SetShadowColor
Set the world's shadow color. Shadow color is used by both projected shadows and simple shadows (ShadowMesh3D).

Sig: `World:SetShadowColor(color)`
 - Arg: `Vector color` Shadow color
---
### GetShadowColor
Get the world's shadow color. Shadow color is used by both projected shadows and simple shadows (ShadowMesh3D).

Sig: `color = World:GetShadowColor()`
 - Ret: `Vector color` Shadow color
---
### SetFogSettings
Set the world's fog settings.

Sig: `World:SetFogSettings(enabled, color, exponential, near, far)`
 - Arg: `boolean enabled` Enable fog
 - Arg: `Vector color` Fog color
 - Arg: `boolean exponential` Exponential fog falloff (vs linear)
 - Arg: `number near` Distance at which fog starts to accumulate
 - Arg: `number far` Distance at which fog is 100% dense
---
### GetFogSettings
Get the world's fog settings.

Sig: `enabled, color, exponential, near, far = World:GetFogSettings()`
 - Ret: `boolean enabled` Fog enabled
 - Ret: `Vector color` Fog color
 - Ret: `boolean exponential` Exponential fog falloff (vs linear)
 - Ret: `number near` Distance at which fog starts to accumulate
 - Ret: `number far` Distance at which fog is 100% dense
---
### SetGravity
Set the world's gravity. Only used by Primitive3D nodes with physics enabled.

Sig: `World:SetGravity(gravity)`
 - Arg: `Vector gravity` Gravity vector
---
### GetGravity
Get the world's gravity. Only used by Primitive3D nodes with physics enabled.

Sig: `gravity = World:GetGravity()`
 - Ret: `Vector gravity` Gravity vector
---
### RayTest
Find the first primitive node that intersects a ray.

Sig: `res = World:RayTest(start, end, colMask, ignoreObjects=nil)`
 - Arg: `Vector start` Start position
 - Arg: `Vector end` End position
 - Arg: `integer colMask` Collision mask (use 0xff for all collision groups)
 - Arg: `table ignoreObjects` Array of Primitive3D nodes to ignore in test
 - Ret: `table res` Ray test result
   - `Vector start`
   - `Vector end`
   - `Primitive3D hitNode`
   - `Vector hitNormal`
   - `Vector hitPosition`
   - `number hitFraction`
---
### RayTestMulti
Find all primitive nodes that intersect a ray.

Sig: `res = World:RayTestMulti(start, end, colMask, ignoreObjects=nil)`
 - Arg: `Vector start` Start position
 - Arg: `Vector end` End position
 - Arg: `integer colMask` Collision mask (use 0xff for all collision groups)
 - Ret: `table results` Array of results
   - `table res` Element of the array
     - `Primitive3D node`
     - `Vector normal`
     - `Vector position`
     - `number fraction`
---
### SweepTest
Perform a shape sweep test using a Primitive3D node's collision shape. This test will return the first primitive node hit.

TODO: Add ignore list. (This function will ignore the swept primitive though)

Sig: `res = World:SweepTest(prim, start, end, colMask)`
 - Arg: `Primitive3D prim` Primitive node whose collision shape will be used for the test
 - Arg: `Vector start` Start position
 - Arg: `Vector end` End position
 - Arg: `integer colMask` Collision mask (Use 0xff for all collision groups)
 - Ret: `table res` Test result
   - `Vector start`
   - `Vector end`
   - `Primitive3D hitNode`
   - `Vector hitNormal`
   - `Vector hitPosition`
   - `number hitFraction`
---
### LoadScene
Clear the world and instantiate a new scene as the root node.

Sig: `World:LoadScene(sceneName, instant=false)`
 - Arg: `string sceneName` Name of Scene asset to load
 - Arg: `boolean instant` Whether to instantly load the scene, or wait until the start of the next frame. Note: Using instant loading may cause problems and probably shouldn't be used. Setting instant to false is generally safer.
---
### QueueRootNode
Queue a new node to be set as the world's root node at the start of the next frame. This is used internally by World:LoadScene().

Sig: `World:QueueRootNode(newRoot)`
 - Arg: `Node newRoot` New root node
---
### EnableInternalEdgeSmoothing
Enable internal edge smoothing. May provide smoother collisions along collision mesh boundaries.

Sig: `World:EnableInternalEdgeSmoothing(enable)`
 - Arg: `boolean enable` Enable internal edge smoothing
---
### IsInternalEdgeSmoothingEnabled
Check if internal edge smoothing is enabled. May provide smoother collisions along collision mesh boundaries.

Sig: `enable = World:IsInternalEdgeSmoothingEnabled()`
 - Ret: `boolean enable` Internal edge smoothing enabled
---
### SpawnParticle
Spawn a particle system at a specific location and set it to automatically destroy itself after it finishes.

Sig: `particle = World:SpawnParticle(system, position)`
 - Arg: `ParticleSystem system` Particle system asset to instantiate
 - Arg: `Vector position` World position to place particle
 - Ret: `Particle3D particle` The newly created particle
---
