--- @meta

---@class TimelinePlayer : Node
TimelinePlayer = {}

function TimelinePlayer:Play() end

function TimelinePlayer:Pause() end

function TimelinePlayer:Stop() end

---@param time number
function TimelinePlayer:SetTime(time) end

---@return number
function TimelinePlayer:GetTime() end

---@return number
function TimelinePlayer:GetDuration() end

---@return boolean
function TimelinePlayer:IsPlaying() end

---@return boolean
function TimelinePlayer:IsPaused() end

---@param arg1? Asset
function TimelinePlayer:SetTimeline(arg1) end

---@return Asset
function TimelinePlayer:GetTimeline() end

---@param playOnStart boolean
function TimelinePlayer:SetPlayOnStart(playOnStart) end

---@return boolean
function TimelinePlayer:GetPlayOnStart() end
