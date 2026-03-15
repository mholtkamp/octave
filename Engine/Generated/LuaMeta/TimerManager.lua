--- @meta

---@class TimerManagerModule
TimerManager = {}

---@param arg1 function
---@param time number
---@return integer
function TimerManager.SetTimer(arg1, time) end

function TimerManager.ClearAllTimers() end

---@param id integer
function TimerManager.ClearTimer(id) end

---@param id integer
function TimerManager.PauseTimer(id) end

---@param id integer
function TimerManager.ResumeTimer(id) end

---@param id integer
function TimerManager.ResetTimer(id) end

---@param id integer
---@return number
function TimerManager.GetTimeRemaining(id) end
