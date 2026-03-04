--- @meta

---@class ScriptModule
Script = {}

---@param filename string
function Script.Load(filename) end

---@param filename string
function Script.Run(filename) end

---@param arg1 table
---@param className string
function Script.Inherit(arg1, className) end

---@param arg1 table
---@return any
function Script.New(arg1) end

function Script.GarbageCollect() end

---@param dirStr string
---@param recurse? boolean
function Script.LoadDirectory(dirStr, recurse) end

---@param filename string
function Script.Require(filename) end

---@param arg1 table
---@param className string
function Script.Extend(arg1, className) end
