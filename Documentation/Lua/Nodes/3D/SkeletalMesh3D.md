# SkeletalMesh3D

A node that renders a skeletal mesh. Multiple animations can be played at the same time.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)
* [Mesh3D](Mesh3D.md)

---

### SetSkeletalMesh
Set the skeletal mesh asset.

Sig: `SkeletalMesh3D:SetSkeletalMesh(mesh)`
 - Arg: `SkeletalMesh mesh` The skeletal mesh asset
---
### GetSkeletalMesh
Get the skeletal mesh asset.

Sig: `mesh = SkeletalMesh3D:GetSkeletalMesh()`
 - Ret: `SkeletalMesh mesh` The skeletal mesh asset
---
### PlayAnimation
Play an animation contained within the skeletal mesh asset. The priority is actually what index to place the animation within the array of active animations. Animations at the end of the array will be processed last, and thus overwrite previous animations depending on what bones are animated and the weight of the animation. Priority should be in the range 0 to 255.

Note: You can update an animation (for instance, change its speed) by calling PlayAnimation() a second time with the same animation name and updated options.

Sig: `SkeletalMesh3D:PlayAnimation(animName, loop=false, speed=1, weight=1, priority=255)`
 - Arg: `string animName` Name of the animation to play
 - Arg: `boolean loop` Should the animation loop once reaching the end
 - Arg: `number speed` Speed multiplier. 1 = normal speed.
 - Arg: `number weight` How much weight should be given to the animation. (1 = default)
 - Arg: `integer priority` Animation priority (0-255)
---
### StopAnimation
Stop a specific animation.

Sig: `SkeletalMesh3D:StopAnimation(animName, cancelQueued=false)`
 - Arg: `string animName` Name of the animation to cancel
 - Arg: `boolean cancelQueued` Whether to remove the animation from play queue if it hasn't been played yet.
---
### StopAllAnimations
Stop all animations.

Sig: `SkeletalMesh3D:StopAllAnimations(cancelQueued=false)`
 - Arg: `boolean cancelQueued` Whether to remove all queued animations as well.
---
### IsAnimationPlaying
Check if an animation is playing.

Sig: `playing = SkeletalMesh3D:IsAnimationPlaying(animName)`
 - Arg: `string animName` Name of animation
 - Ret: `boolean playing` Is animation playing
---
### QueueAnimation
Queue an animation to be played. A target animation name can be provided to wait on, otherwise the queued animation will be played after the animation with the highest priority finishes.

Sig: `SkeletalMesh3D:QueueAnimation(animName, loop, dependentAnimName=nil, speed=1, weight=1, priority=255)`
 - Arg: `string animName` Name of animation to queue
 - Arg: `boolean loop` Whether to loop
 - Arg: `string dependentAnimName` Name of dependent anim to wait on
 - Arg: `number speed` Speed multiplier
 - Arg: `number weight` Animation blending weight (0 - 1)
 - Arg: `integer priority` Animation priority
---
### CancelQueuedAnimation
Cancel a queued animation by name.

Sig: `SkeletalMesh3D:CancelQueuedAnimation(animName)`
 - Arg: `string animName` Name of animation to cancel
---
### CancelAllQueuedAnimations
Cancel all queued animations.

Sig: `SkeletalMesh3D:CancelAllQueuedAnimations()`

---
### SetInheritPose
Whether this skeletal mesh node should inherit its pose from its parent node (assuming its parent node is also a SkeletalMesh3D node). This is useful for things like having clothing or accessories animate when the base body mesh moves on a character for instance.

Sig: `SkeletalMesh3D:SetInheritPose(inheritPose)`
 - Arg: `boolean inheritPose` Whether to inherit parent pose
---
### IsInheritPoseEnabled
Check if this node is set to inherit its parent's pose.

Sig: `inheritPose = SkeletalMesh3D:IsInheritPoseEnabled()`
 - Ret: `boolean inheritPose` Whether pose is inherited
---
### ResetAnimation
Reset all animations to their beginning frame.

Sig: `SkeletalMesh3D:ResetAnimation()`

---
### GetAnimationSpeed
Get this node's animation playback speed. This playback speed multiplier is applied to ALL animations uniformly.

Sig: `speed = SkeletalMesh3D:GetAnimationSpeed()`
 - Ret: `number speed` Animation playback speed
---
### SetAnimationSpeed
Set this node's animation playback speed. This playback speed multiplier is applied to ALL animations uniformly.

Sig: `SkeletalMesh3D:SetAnimationSpeed(speed)`
 - Arg: `number speed` Animation playback speed
---
### SetAnimationPaused
Use to pause and unpause this node from animating.

Sig: `SkeletalMesh3D:SetAnimationPaused(paused)`
 - Arg: `boolean paused` Whether to pause
---
### IsAnimationPaused
Check if this node's animation is paused. (for all animations).

Sig: `paused = SkeletalMesh3D:IsAnimationPaused()`
 - Ret: `boolean paused` Whether node animation is paused
---
### GetBonePosition
Get a bone's world space position.

Sig: `position = SkeletalMesh3D:GetBonePosition(boneName)`
 - Arg: `string boneName` Bone name to query
 - Ret: `Vector position` Bone world space position
---
### GetBoneRotation
Get a bone's (local?) rotation as euler angles. I don't think this works yet.
TODO: Fix GetBoneRotation().

Sig: `rotEuler = SkeletalMesh3D:GetBoneRotation(boneName)`
 - Arg: `string boneName` Bone name to query
 - Ret: `Vector rotEuler` Bone rotation as euler angles
---
### GetBoneScale
Get a bone's scale (in local space?). I don't think this works yet.
TODO: Fix GetBoneScale().

Sig: `scale = SkeletalMesh3D:GetBoneScale(boneName)`
 - Arg: `string boneName` Bone name to query
 - Ret: `Vector scale` Scale of bone
---
### SetBonePosition
Set a bone's world space position. Not yet implemented.
TODO: Implement SetBonePosition().

Sig: `SkeletalMesh3D:SetBonePosition(boneName, position)`
 - Arg: `string boneName` Name of bone to adjust
 - Arg: `Vector position` World space position to place bone
---
### SetBoneRotation
Set a bone's local space rotation from euler angles. Not yet implemented.
TODO: Implement SetBoneRotation().

Sig: `SkeletalMesh3D:SetBoneRotation(boneName, rotEuler)`
 - Arg: `string boneName` Name of bone to adjust
 - Arg: `Vector rotEuler` Rotation in euler angles
---
### SetBoneScale
Set a bone's local space scale. Not yet implemented.
TODO: Implement SetBoneScale().

Sig: `SkeletalMesh3D:SetBoneScale(boneName, scale)`
 - Arg: `string boneName` Name of bone to adjust
 - Arg: `Vector scale` Bone scale
---
### GetNumBones
Get the number of bones this mesh node is using.

Sig: `numBones = SkeletalMesh3D:GetNumBones()`
 - Ret: `integer numBones` The number of bones in the mesh asset
---
### SetAnimEventHandler
Set an animation event handler. An animation event can be setup by creating a bone with the name "Event_MyAnimEvent". When any animation event is triggered on this node, the given function will be called so that you can handle it.

Events will be triggered wherever there is a keyframe on the timeline (I don't think it matters if it's position, rotation, or scale). Make sure your asset is not using Sampled frames when exporting the animation.

Sig: `SkeletalMesh3D:SetAnimEventHandler(handlerFunc)`
 - Arg: `function handlerFunc` Anim event function handler that will be called when an anim event is triggered/played.
---
### SetBoundsRadiusOverride
Override the bounds radius. Because animations can cause vertices to extend past the default pose bounds, this function lets you override the radius to avoid erroneous frustum culling. Setting the bounds radius to 0 will use the mesh default radius.

Sig: `SkeletalMesh3D:SetBoundsRadiusOverride(radius)`
 - Arg: `number radius` Override radius
---
### GetBoundsRadiusOverride
Get the overridden bounds radius.

Sig: `radius = SkeletalMesh3D:GetBoundsRadiusOverride()`
 - Ret: `number radius` Override radius
---