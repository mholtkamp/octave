--- @meta

---@class SoundWave : Asset
local SoundWave = {}

---@return number
function SoundWave:GetVolumeMultiplier() end

---@param value number
function SoundWave:SetVolumeMultiplier(value) end

---@return number
function SoundWave:GetPitchMultiplier() end

---@param value number
function SoundWave:SetPitchMultiplier(value) end

---@return integer
function SoundWave:GetWaveDataSize() end

---@return integer
function SoundWave:GetNumChannels() end

---@return integer
function SoundWave:GetBitsPerSample() end

---@return integer
function SoundWave:GetSampleRate() end

---@return integer
function SoundWave:GetNumSamples() end

---@return integer
function SoundWave:GetByteRate() end

---@return number
function SoundWave:GetDuration() end
