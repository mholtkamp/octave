--- @meta

---@class AssetManagerModule
AssetManager = {}

function AssetManager.RefSweep() end

---@param name string
---@return any
function AssetManager.GetAsset(name) end

---@param name string
---@return any
function AssetManager.LoadAsset(name) end

---@param name string
function AssetManager.SaveAsset(name) end

---@param name string
---@return any
function AssetManager.AsyncLoadAsset(name) end

---@param name string
function AssetManager.UnloadAsset(name) end

---@param assetTypeStr string
---@param assetDirStr string
---@param assetNameStr string
---@return any
function AssetManager.CreateAndRegisterAsset(assetTypeStr, assetDirStr, assetNameStr) end

---@type function
RefSweep = AssetManager.RefSweep

---@type function
GetAsset = AssetManager.GetAsset

---@type function
LoadAsset = AssetManager.LoadAsset

---@type function
AsyncLoadAsset = AssetManager.AsyncLoadAsset

---@type function
UnloadAsset = AssetManager.UnloadAsset
