--- @meta

---@class DataAsset : Asset
DataAsset = {}

---@param propName string
---@return nil
function DataAsset:Get(propName) end

---@param propName string
---@param val integer
function DataAsset:Set(propName, val) end

---@return string
function DataAsset:GetScriptFile() end

---@param scriptFile string
function DataAsset:SetScriptFile(scriptFile) end

function DataAsset:ReloadProperties() end
