--- @meta

---@class SystemModule
System = {}

---@param saveName string
---@param stream Stream
function System.WriteSave(saveName, stream) end

---@param saveName string
---@param stream Stream
function System.ReadSave(saveName, stream) end

---@param saveName string
---@return boolean
function System.DoesSaveExist(saveName) end

---@param saveName string
function System.DeleteSave(saveName) end

function System.UnmountMemoryCard() end

---@param value integer
function System.SetScreenOrientation(value) end

---@return integer
function System.GetScreenOrientation() end

---@param fullscreen boolean
function System.SetFullscreen(fullscreen) end

---@return boolean
function System.IsFullscreen() end

---@param value string
function System.SetWindowTitle(value) end
