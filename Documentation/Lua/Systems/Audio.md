# Audio

System to play and manage sounds.

---
### PlaySound2D
Play a sound with no positional data.
TODO: Rename to PlaySound() if we add 2D support.

Sig: `Audio.PlaySound(sound, volume=1, pitch=1, startTime=0, loop=false, priority=0)`
 - Arg: `SoundWave sound` Sound wave to play
 - Arg: `number volume` Volume multiplier
 - Arg: `number pitch` Pitch multiplier
 - Arg: `number startTime` Start time offset
 - Arg: `boolean loop` Loop sound
 - Arg: `integer priority` Sound priority
---
### PlaySound3D
Play a sound at a position in 3D space.
TODO: Rename to PlaySoundAtPosition() to handle both 2D and 3D.

Sig: `Audio.PlaySound3D(sound, position, innerRadius, outerRadius, attenuationFunc=AttenuationFunc.Linear, volume=1, pitch=1, startTime=0, loop=false, priority=0)`
 - Arg: `SoundWave sound` Sound wave to play
 - Arg: `Vector position` Position of sound
 - Arg: `number innerRadius` Inner radius
 - Arg: `number outerRadius` Outer radius
 - Arg: `AttenuationFunc(integer) attenuationFunc` Attenuation function
 - Arg: `number volume` Volume multiplier
 - Arg: `number pitch` Pitch multiplier
 - Arg: `number startTime` Start time offset
 - Arg: `boolean loop` Loop sound
 - Arg: `integer priority` Sound priority
---
### StopSounds
Stop sounds using a particular sound wave.

Alias: `StopSound`

Sig: `Audio.StopSounds(sound)`
 - Arg: `SoundWave sound` Sound wave to stop
---
### StopAllSounds
Stop all sounds.

Sig: `Audio.StopAllSounds()`

---
### IsSoundPlaying
Check if a sound wave is playing.

Sig: `playing = Audio.IsSoundPlaying(sound)`
 - Arg: `SoundWave sound` Sound wave to check
 - Ret: `boolean playing` Is the sound playing
---
### SetAudioClassVolume
Set the volume of an audio class. Audio classes can be used to control the volume and pitch of multiple sounds.

Sig: `Audio.SetAudioClassVolume(volume)`
 - Arg: `number volume` Volume multiplier
---
### GetAudioClassVolume
Get the volume of an audio class. Audio classes can be used to control the volume and pitch of multiple sounds.

Sig: `volume = Audio.GetAudioClassVolume()`
 - Ret: `number volume` Volume multiplier
---
### SetAudioClassPitch
Set the pitch of an audio class. Audio classes can be used to control the volume and pitch of multiple sounds.

Sig: `Audio.SetAudioClassPitch(pitch)`
 - Arg: `number pitch` Pitch multiplier
---
### GetAudioClassPitch
Get the pitch of an audio class. Audio classes can be used to control the volume and pitch of multiple sounds.

Sig: `pitch = Audio.GetAudioClassPitch()`
 - Ret: `number pitch` Pitch multiplier
---
### SetMasterVolume
Set the master volume.

Sig: `Audio.SetMasterVolume(volume)`
 - Arg: `number volume` Master volume
---
### GetMasterVolume
Get the master volume.

Sig: `volume = Audio.GetMasterVolume()`
 - Ret: `number volume` Master volume
---
### SetMasterPitch
Set the master pitch.

Sig: `Audio.SetMasterPitch(pitch)`
 - Arg: `number pitch` Master pitch
---
### GetMasterPitch
Get the master pitch.

Sig: `pitch = Audio.GetMasterPitch()`
 - Ret: `number pitch` Master pitch
---
