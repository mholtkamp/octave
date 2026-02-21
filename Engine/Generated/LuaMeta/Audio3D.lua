--- @meta

---@class Audio3D : Node3D
local Audio3D = {}

---@param arg1? SoundWave
function Audio3D:SetSoundWave(arg1) end

---@return any
function Audio3D:GetSoundWave() end

---@param value number
function Audio3D:SetInnerRadius(value) end

---@return number
function Audio3D:GetInnerRadius() end

---@param value number
function Audio3D:SetOuterRadius(value) end

---@return number
function Audio3D:GetOuterRadius() end

---@param value number
function Audio3D:SetVolume(value) end

---@return number
function Audio3D:GetVolume() end

---@param value number
function Audio3D:SetPitch(value) end

---@return number
function Audio3D:GetPitch() end

---@param value number
function Audio3D:SetStartOffset(value) end

---@return number
function Audio3D:GetStartOffset() end

---@param arg1 integer
function Audio3D:SetPriority(arg1) end

---@return integer
function Audio3D:GetPriority() end

---@param arg1 integer
function Audio3D:SetAttenuationFunc(arg1) end

---@return integer
function Audio3D:GetAttenuationFunc() end

---@param value boolean
function Audio3D:SetLoop(value) end

---@return boolean
function Audio3D:GetLoop() end

---@param value boolean
function Audio3D:SetAutoPlay(value) end

---@return boolean
function Audio3D:GetAutoPlay() end

---@return number
function Audio3D:GetPlayTime() end

---@return boolean
function Audio3D:IsPlaying() end

---@return boolean
function Audio3D:IsAudible() end

function Audio3D:PlayAudio() end

function Audio3D:PauseAudio() end

function Audio3D:StopAudio() end

function Audio3D:ResetAudio() end
