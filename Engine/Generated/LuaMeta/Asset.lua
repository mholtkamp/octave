--- @meta

---@class Asset
Asset = {}

---@param assetB Asset
---@return boolean
function Asset:Equals(assetB) end

---@return string
function Asset:GetName() end

---@return boolean
function Asset:IsRefCounted() end

---@return integer
function Asset:GetRefCount() end

---@return string
function Asset:GetTypeName() end

---@return boolean
function Asset:IsTransient() end

---@return boolean
function Asset:IsLoaded() end
