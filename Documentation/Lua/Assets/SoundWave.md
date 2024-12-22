# SoundWave

An asset that represents a sound wave.

Inheritance:
* [Asset](Asset.md)

---
### GetVolumeMultiplier
Get the volume multiplier.

Sig: `mult = SoundWave:GetVolumeMultiplier()`
 - Ret: `number mult` Volume multiplier
---
### SetVolumeMultiplier
Set the volume multiplier.

Sig: `SoundWave:SetVolumeMultiplier(mult)`
 - Arg: `number mult` Volume multiplier
---
### GetPitchMultiplier
Get the pitch multiplier.

Sig: `mult = SoundWave:GetPitchMultiplier()`
 - Ret: `number mult` Pitch multiplier
---
### SetPitchMultiplier
Set the pitch multiplier.

Sig: `SoundWave:SetPitchMultiplier(mult)`
 - Arg: `number mult` Pitch multiplier
---
### GetWaveDataSize
Get the wave data size in bytes.

Sig: `size = SoundWave:GetWaveDataSize()`
 - Ret: `integer size` Wave data size in bytes
---
### GetNumChannels
Get the number of sound channels. Either 1 (mono) or 2 (stereo).

Sig: `num = SoundWave:GetNumChannels()`
 - Ret: `integer num` Num channels
---
### GetBitsPerSample
Get the number of bits per sample. Either 8 or 16.

Sig: `bits = SoundWave:GetBitsPerSample()`
 - Ret: `integer bits` Bits per sample
---
### GetSampleRate
Get the sample rate. Either 22050 or 44100.

Sig: `rate = SoundWave:GetSampleRate()`
 - Ret: `integer rate` Sample rate
---
### GetNumSamples
Get the total number of samples.

Sig: `num = SoundWave:GetNumSamples()`
 - Ret: `integer num` Num samples
---
### GetByteRate
Get the byte rate.

Sig: `rate = SoundWave:GetByteRate()`
 - Ret: `integer rate` Byte rate
---
### GetDuration
Get the sound wave duration in seconds.

Sig: `duration = SoundWave:GetDuration()`
 - Ret: `number duration` Duration in seconds
---
