--- @meta

---@class AudioModule
Audio = {}

---@param soundWave SoundWave
---@param volume? number
---@param pitch? number
---@param startTime? number
---@param loop? boolean
---@param priority? integer
function Audio.PlaySound2D(soundWave, volume, pitch, startTime, loop, priority) end

---@param soundWave SoundWave
---@param pos Vector
---@param innerRadius number
---@param outerRadius number
---@param attenFunc? integer
---@param volume? number
---@param pitch? number
---@param startTime? number
---@param loop? boolean
---@param priority? integer
function Audio.PlaySound3D(soundWave, pos, innerRadius, outerRadius, attenFunc, volume, pitch, startTime, loop, priority) end

---@param soundWave SoundWave
function Audio.StopSounds(soundWave) end

---@param soundWave SoundWave
---@param volume number
---@param pitch number
---@param priority integer
function Audio.UpdateSound(soundWave, volume, pitch, priority) end

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
