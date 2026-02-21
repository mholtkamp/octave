--- @meta

---@class Stream
Stream = {}

---@return any
function Stream:Create() end

---@return integer
function Stream:GetSize() end

---@return integer
function Stream:GetPos() end

---@param pos integer
function Stream:SetPos(pos) end

---@param path string
---@param maxSize integer
function Stream:ReadFile(path, maxSize) end

---@param path string
function Stream:WriteFile(path) end

---@return any
function Stream:ReadAsset() end

---@param arg1 Asset
function Stream:WriteAsset(arg1) end

---@return string
function Stream:ReadString() end

---@param str string
function Stream:WriteString(str) end

---@return integer
function Stream:ReadInt32() end

---@return integer
function Stream:ReadInt16() end

---@return integer
function Stream:ReadInt8() end

---@return number
function Stream:ReadFloat() end

---@return boolean
function Stream:ReadBool() end

---@return Vector
function Stream:ReadVec2() end

---@return Vector
function Stream:ReadVec3() end

---@return Vector
function Stream:ReadVec4() end

---@param value integer
function Stream:WriteInt32(value) end

---@param value integer
function Stream:WriteInt16(value) end

---@param value integer
function Stream:WriteInt8(value) end

---@param value number
function Stream:WriteFloat(value) end

---@param value boolean
function Stream:WriteBool(value) end

---@param value Vector
function Stream:WriteVec2(value) end

---@param value Vector
function Stream:WriteVec3(value) end

---@param value Vector
function Stream:WriteVec4(value) end
