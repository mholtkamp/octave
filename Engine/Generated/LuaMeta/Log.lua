--- @meta

---@class LogModule
Log = {}

---@param string string
function Log.Debug(string) end

---@param string string
function Log.Warning(string) end

---@param string string
function Log.Error(string) end

---@param string string
---@param color Vector
function Log.Console(string, color) end

---@param value boolean
function Log.Enable(value) end

---@return boolean
function Log.IsEnabled() end

---@param string string
function Log.Info(string) end
