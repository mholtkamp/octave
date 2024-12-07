# SkeletalMesh3D

A node that renders a skeletal mesh. Multiple animations can be played at the same time.

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
Play an animation contained within the skeletal mesh asset.

Sig: `SkeletalMesh3D:PlayAnimation(animName, loop=false, speed=1, weight=1, priority=255)`
 - Arg: `string animName` Name of the animation to play
 - Arg: `boolean loop` Should the animation loop once reaching the end
 - Arg: `number speed` Speed multiplier. 1 = normal speed.
 - Arg: `number weight` 
---
### StopAnimation
---
### StopAllAnimations
---
### IsAnimationPlaying
---
### QueueAnimation
---
### CancelQueuedAnimation
---
### CancelAllQueuedAnimations
---
### SetInheritPose
---
### IsInheritPoseEnabled
---
### ResetAnimation
---
### GetAnimationSpeed
---
### SetAnimationSpeed
---
### SetAnimationPaused
---
### IsAnimationPaused
---
### GetBonePosition
---
### GetBoneRotation
---
### GetBoneScale
---
### SetBonePosition
---
### SetBoneRotation
---
### SetBoneScale
---
### GetNumBones
---
### SetAnimEventHandler
---
### SetBoundsRadiusOverride
---
### GetBoundsRadiusOverride
---