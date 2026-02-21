--- @meta

---@class ScriptModule
Script = {}

---@param filename string
function Script.Load(filename) end

---@param filename string
function Script.Run(filename) end

---@param className string
function Script.Inherit(className) end

---@return any
function Script.New() end

function Script.GarbageCollect() end

---@param dirStr string
---@param recurse? boolean
function Script.LoadDirectory(dirStr, recurse) end

---@param filename string
function Script.Require(filename) end

---@param className string
function Script.Extend(className) end
