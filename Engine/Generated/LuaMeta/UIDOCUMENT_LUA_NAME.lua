--- @meta

---@class UIDOCUMENT_LUA_NAME : Asset
UIDOCUMENT_LUA_NAME = {}

---@return any
function UIDOCUMENT_LUA_NAME:Instantiate() end

---@param parent Widget
function UIDOCUMENT_LUA_NAME:Mount(parent) end

function UIDOCUMENT_LUA_NAME:Unmount() end

---@param id string
---@return any
function UIDOCUMENT_LUA_NAME:FindById(id) end

---@param className string
---@return integer
function UIDOCUMENT_LUA_NAME:FindByClass(className) end

---@return any
function UIDOCUMENT_LUA_NAME:GetRootWidget() end

---@param key string
---@param value string
function UIDOCUMENT_LUA_NAME:SetData(key, value) end

---@param elementId string
---@param eventName string
function UIDOCUMENT_LUA_NAME:SetCallback(elementId, eventName) end

function UIDOCUMENT_LUA_NAME:Tick() end

---@return Asset
function UIDOCUMENT_LUA_NAME:Load() end

---@return nil
function UIDOCUMENT_LUA_NAME:LoadFromString() end
