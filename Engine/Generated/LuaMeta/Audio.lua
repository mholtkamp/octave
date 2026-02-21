--- @meta

---@class AudioModule
Audio = {}

---@param soundWave SoundWave
---@param arg2? number
---@param arg3? number
---@param arg4? number
---@param arg5? boolean
---@param arg6? integer
function Audio.PlaySound2D(soundWave, arg2, arg3, arg4, arg5, arg6) end

---@param soundWave SoundWave
---@param pos Vector
---@param innerRadius number
---@param outerRadius number
---@param arg5? integer
---@param arg6? number
---@param arg7? number
---@param arg8? number
---@param arg9? boolean
---@param arg10? integer
function Audio.PlaySound3D(soundWave, pos, innerRadius, outerRadius, arg5, arg6, arg7, arg8, arg9, arg10) end

---@param soundWave SoundWave
function Audio.StopSounds(soundWave) end

---@param soundWave SoundWave
---@param volume number
---@param arg3 number
---@param arg4 integer
function Audio.UpdateSound(soundWave, volume, arg3, arg4) end

function Audio.StopAllSounds() end

---@param soundWave SoundWave
---@return boolean
function Audio.IsSoundPlaying(soundWave) end

---@param audioClass integer
---@param volume number
function Audio.SetAudioClassVolume(audioClass, volume) end

---@param audioClass integer
---@return number
function Audio.GetAudioClassVolume(audioClass) end

---@param audioClass integer
---@param pitch number
function Audio.SetAudioClassPitch(audioClass, pitch) end

---@param audioClass integer
---@return number
function Audio.GetAudioClassPitch(audioClass) end

---@param value number
function Audio.SetMasterVolume(value) end

---@return number
function Audio.GetMasterVolume() end

---@param value number
function Audio.SetMasterPitch(value) end

---@return number
function Audio.GetMasterPitch() end

---@param soundWave SoundWave
function Audio.StopSound(soundWave) end
