# Audio3D

A node that plays sounds in 3D space. It can be used to play a looping sound or a one-off sound. See the Audio system for playing audio without using a node.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
    
---
### SetSoundWave
Assign a sound wave asset to be played.

Sig: `Audio3D:SetSoundWave(soundWave)`
 - Arg: `SoundWave soundWave` SoundWave asset
---
### GetSoundWave
Get the currently assigned sound wave.

Sig: `soundWave = Audio3D:GetSoundWave()`
 - Ret: `SoundWave soundWave` SoundWave asset
---
### SetInnerRadius
Set the inner radius. The inner radius defines the distance away from the node at which the sound is the loudest. Going closer to the node won't increase the volume any further.

Sig: `Audio3D:SetInnerRadius(radius)`
 - Arg: `number radius` Inner radius
---
### GetInnerRadius
Get the inner radius. The inner radius defines the distance away from the node at which the sound is the loudest. Going closer to the node won't increase the volume any further.

Sig: `radius = Audio3D:GetInnerRadius()`
 - Ret: `number radius` Inner radius
---
### SetOuterRadius
Set the outer radius. The outer radius defines the distance away from the node at which the sound can't be heard anymore. Volume will increase when moving from the outer radius distance to the inner radius distance.

Sig: `Audio3D:SetOuterRadius(radius)`
 - Arg: `number radius` Outer radius
---
### GetOuterRadius
Get the outer radius. The outer radius defines the distance away from the node at which the sound can't be heard anymore. Volume will increase when moving from the outer radius distance to the inner radius distance.

Sig: `radius = Audio3D:GetOuterRadius()`
 - Ret: `number radius` Outer radius
---
### SetVolume
Set the volume multiplier.

Sig: `Audio3D:SetVolume(volume)`
 - Arg: `number volume` Volume
---
### GetVolume
Get the volume multiplier.

Sig: `volume = Audio3D:GetVolume()`
 - Ret: `number volume` Volume
---
### SetPitch
Set the pitch multiplier.

Sig: `Audio3D:SetPitch(pitch)`
 - Arg: `number pitch` Pitch
---
### GetPitch
Get the pitch multiplier.

Sig: `pitch = Audio3D:GetPitch()`
 - Ret: `number pitch` Pitch
---
### SetStartOffset
Adjust the start time when the audio is played.

Sig: `Audio3D:SetStartOffset(offset)`
 - Arg: `number offset` Time offset in seconds
---
### GetStartOffset
Get the start time offset.

Sig: `offset = Audio3D:GetStartOffset()`
 - Ret: `number offset` Time offset in seconds
---
### SetPriority
Set the priority of this node's audio. If the number of concurrent sounds has been reached, sounds with lower priority will be stopped to allow sounds with higher priority to play. Default priority is 0. Priority is stored as 32-bit integer and can be negative.

Sig: `Audio3D:SetPriority(priority)`
 - Arg: `integer priority` Sound priority
---
### GetPriority
Get the audio priority of this node. If the number of concurrent sounds has been reached, sounds with lower priority will be stopped to allow sounds with higher priority to play. Default priority is 0. Priority is stored as 32-bit integer and can be negative.

Sig: `priority = Audio3D:GetPriority()`
 - Ret: `integer priority` Sound priority
---
### SetAttenuationFunc
Set the sound intensity attenuation function. The attenuation controls how quickly a sound's volume fades out when moving from the inner radius to the outer radius, or alternatively, how quickly a sound's volume fades in when moving from the outer radius to the inner radius.

See the [AttenuationFunc](../../Misc/Enums.md#attenuationfunc) enumeration.

Sig: `Audio3D:SetAttenuationFunc(attenFunc)`
 - Arg: `AttenuationFunc(integer) attenFunc` Attenuation function
---
### GetAttenuationFunc
Get the sound intensity attenuation function. The attenuation controls how quickly a sound's volume fades out when moving from the inner radius to the outer radius, or alternatively, how quickly a sound's volume fades in when moving from the outer radius to the inner radius.

See the [AttenuationFunc](../../Misc/Enums.md#attenuationfunc) enumeration.

Sig: `attenFunc = Audio3D:GetAttenuationFunc()`
 - Ret: `AttenuationFunc(integer) attenFunc` Attenuation function
---
### SetLoop
Set whether the sound wave should loop.

Sig: `Audio3D:SetLoop(loop)`
 - Arg: `boolean loop` Loop
---
### GetLoop
Check if the sound wave is set to loop after finishing playing.

Sig: `loop = Audio3D:GetLoop()`
 - Ret: `boolean loop` Loop
---
### SetAutoPlay
Set whether sound should automatically play when the node starts (i.e. when the node's Start() function is called).

Sig: `Audio3D:SetAutoPlay(autoPlay)`
 - Arg: `boolean autoPlay` Auto play
---
### GetAutoPlay
Check if the sound is set to automatically play when the node starts.

Sig: `autoPlay = Audio3D:GetAutoPlay()`
 - Ret: `boolean autoPlay` Auto play
---
### GetPlayTime
Return the amount of time in seconds that this node has been playing audio.

Sig: `playTime = Audio3D:GetPlayTime()`
 - Ret: `number playTime` Time in seconds that audio has been playing.
---
### IsPlaying
Check if this node is currently playing a sound wave.

Sig: `playing = Audio3D:IsPlaying()`
 - Ret: `boolean playing` True if a sound wave is playing
---
### IsAudible
Check if this node can be heard by the audio receiver (e.g. the player, or camera). If the audio receiver moves outside of this node's outer radius, then this will return false. See World.SetAudioReceiver() for how to change the receiver.

Sig: `audible = Audio3D:IsAudible()`
 - Ret: `boolean audible` True if receiver can hear the audio
---
### PlayAudio
Play this node's assigned sound wave. If this node is already playing a sound wave, this function won't do anything. You can use ResetAudio() if you want to replay from the beginning of the sound wave. An Audio3D node can only play one sound wave at a time.

Sig: `Audio3D:PlayAudio()`

---
### PauseAudio
Pause the sound wave that is being played. Calling PlayAudio() later will pick up where it left off.

Sig: `Audio3D:PauseAudio()`

---
### StopAudio
Stop the sound wave that is being played. This will also reset the current play time back to the beginning of the sound wave. Use PauseAudio() if you want to pick up where it left off.

Sig: `Audio3D:StopAudio()`

---
### ResetAudio
Reset the play position back to the beginning of the sound wave. This works whether the sound wave is playing or not.

Sig: `Audio3D:ResetAudio()`

---
