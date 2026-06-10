# Sprite

A widget that animates an array of textures. A Sprite can have multiple animations, which are made up of a name, a vector of frames, 

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)
* [Quad](Quad.md)

---
### AddAnimation
Add an Animation with an empty frame. An empty name will be named "Animation " followed by the first available integer.

Sig: `Sprite:AddAnimation(animationName = "")`
 - Arg: `string animationName` The animation's name.
---
### AddFrame
Add a frame to the end of the animation, or overwrites or inserts at a given frame.

Sig: `Sprite:AddFrame(texture, frameIndex = -1, insert = false, animationIndex = -1)`
 - Arg: `Texture texture` Texture asset
 - Arg: `number frameIndex` Frame to add texture to. Default is current frame
 - Arg: `boolean insert` Whether to insert or overwrite an existing frame
 - Arg: `number animationIndex` The animation to apply this frame to. Default is current animation
---
### AddEmptyFrame
Add an empty frame to the end of the animation, or overwrite or insert at a given frame.

Sig: `Sprite:AddEmptyFrame(frameIndex = -1, insert = false, animationIndex = -1)`
 - Arg: `number frameIndex` Frame to add texture to. Default is current frame
 - Arg: `boolean insert` Whether to insert or overwrite an existing frame. Default is to overwrite
 - Arg: `number animationIndex` The animation to apply this frame to. Default is current animation
---
### RemoveAnimation
Remove an animation.

Sig: `Sprite:RemoveAnimation(animationIndex)`
 - Arg: `number animationIndex` The animation to remove
---
### RemoveFrame
Remove a frame.

Sig: `Sprite:RemoveFrame(frameIndex, animationIndex = -1)`
 - Arg: `number frameIndex` Frame to remove
 - Arg: `number animationIndex` Animation to remove frame from. Default is current animation
---
### SetAnimation
Set the active animation to play.

Sig: `Sprite:SetAnimation(animationIndex)`
 - Arg: `number animationIndex` Index of animation to set as active
 
 Sig: `Sprite:SetAnimation(animationName)`
 - Arg: `string animationName` Name of animation to set as active
---
### SetFrame
Set the animation to a specific frame.

Sig: `Sprite:SetFrame(frameIndex)`
 - Arg: `number frameIndex` Index of frame to cut to
---
### SetFPS
Set the animation framerate.

Sig: `Sprite:SetFPS(fps)`
 - Arg: `number fps` Framerate in frames per second
---
### SetPlay
Set the play/pause state of the animation.

Sig: `Sprite:SetPlay(enablePlay)`
 - Arg: `boolean enablePlay` Plays or pauses animation
---
### SetLoop
Set whether to loop the animation or not.

Sig: `Sprite:SetLoop(enableLoop)`
 - Arg: `boolean enableLoop` Whether to loop
---
### SetAnimationName
Set the name of an animation.

Sig: `Sprite:SetAnimationName(animationName, animationIndex = -1)`
 - Arg: `string animationName` The animation's new name
 - Arg: `number animationIndex` The animation to rename. Defualt is current animation
---
### GetFrame
Get a given frame texture.

Sig: `texture = Sprite:GetFrame(frameIndex, animationIndex = -1)`
 - Arg: `number frameIndex` Frame to retrieve
 - Arg: `number animationIndex` Animation to retrieve frame from. Default is current animation
 - Ret: `Texture texture` Texture asset
---
### GetAnimationLength
Get the length of an animation.

Sig: `animationLength = Sprite:GetAnimationLength(animationIndex = -1)`
 - Arg: `number animationIndex` Animation to retrieve frame count from. Default is current animation
 - Ret: `number animationLength` Number of frames in animation
---
### GetAnimationName
Get the name of an animation.

Sig: `animationName = Sprite:GetAnimationName(animationIndex = -1)`
 - Arg: `number animationIndex` Animation to retrieve name from. Default is current animation
 - Ret: `number animationName` Name of indexed animation
---
### IsPlaying
Check if an animation is playing.

Sig: `isPlay = Sprite:IsPlaying()`
 - Ret: `boolean isPlay` Whether animation is playing
---
### GetLoop
Check if an animation will loop.

Sig: `isLoop = Sprite:GetLoop()`
 - Ret: `boolean isLoop` Whether animation is looping
---
### GetFrameIndex
Check the active frame index.

Sig: `frameIndex = Sprite:GetFrameIndex()`
 - Ret: `number frameIndex` Current frame's index
---
### GetNumAnimations
Get the number of animations.

Sig: `numAnimations = Sprite:GetNumAnimations()`
 - Ret: `number numAnimations` Number of animations
---
### GetAnimationIndex
Get the active animation's index.

Sig: `animationIndex = Sprite:GetAnimationIndex()`
 - Ret: `number animationIndex` Current animation index. Value of 0 means no active animation
---
### GetNumFrames
Get the number of frames in a given animation.

Sig: `numFrames = Sprite:GetNumFrames(animationIndex = -1)`
 - Arg: `number animationIndex` Animation to retrieve frame count from. Default is current animation
 - Ret: `number numFrames` Number of frames
---
### GetFPS
Get the framerate.

Sig: `fps = Sprite:GetFPS()`
 - Ret: `number fps` Current framerate
---
