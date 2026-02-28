--- @meta

---@class EngineModule
Engine = {}

function Engine.Quit() end

---@return boolean
function Engine.IsShuttingDown() end

---@param worldIndex? integer
---@return World
function Engine.GetWorld(worldIndex) end

---@return number
function Engine.GetTime() end

---@return number
function Engine.GetGameDeltaTime() end

---@return number
function Engine.GetRealDeltaTime() end

---@return number
function Engine.GetGameElapsedTime() end

---@return number
function Engine.GetRealElapsedTime() end

---@return string
function Engine.GetPlatform() end

---@return boolean
function Engine.IsEditor() end

function Engine.Break() end

---@param msg string
function Engine.Alert(msg) end

---@param enableBreak boolean
function Engine.SetBreakOnScriptError(enableBreak) end

---@return boolean
function Engine.IsPlayingInEditor() end

---@return boolean
function Engine.IsPlaying() end

---@param restartScripts? boolean
function Engine.ReloadAllScripts(restartScripts) end

---@param value boolean
function Engine.SetPaused(value) end

---@return boolean
function Engine.IsPaused() end

function Engine.FrameStep() end

---@param value number
function Engine.SetTimeDilation(value) end

---@return number
function Engine.GetTimeDilation() end

function Engine.GarbageCollect() end

---@return number
function Engine.GetDeltaTime() end

---@return number
function Engine.GetElapsedTime() end
