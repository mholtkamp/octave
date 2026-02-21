--- @meta

---@class Vec
local Vec = {}
Vec = Vec

function Vec:Create() end

---@param srcVec Vector
function Vec:Set(srcVec) end

---@return Vector
function Vec:Clone() end

---@return Vector
function Vec:Add() end

---@return Vector
function Vec:Subtract() end

---@return Vector
function Vec:Multiply() end

---@return Vector
function Vec:Divide() end

---@param right Vector
---@return boolean
function Vec:Equals(right) end

---@param right Vector
---@return number
function Vec:Dot(right) end

---@param right Vector
---@return number
function Vec:Dot3(right) end

---@param right Vector
---@return Vector
function Vec:Cross(right) end

---@param b Vector
---@param alpha number
---@return Vector
function Vec:Lerp(b, alpha) end

---@param b Vector
---@return Vector
function Vec:Max(b) end

---@param b Vector
---@return Vector
function Vec:Min(b) end

---@param min Vector
---@param max Vector
---@return Vector
function Vec:Clamp(min, max) end

---@return Vector
function Vec:Normalize() end

---@return Vector
function Vec:Normalize3() end

---@param nrm3 Vector
---@return Vector
function Vec:Reflect(nrm3) end

---@param dst Vector
---@param smoothing number
---@param deltaTime number
---@return Vector
function Vec:Damp(dst, smoothing, deltaTime) end

---@param angle number
---@param axis3 Vector
---@return Vector
function Vec:Rotate(angle, axis3) end

---@return number
function Vec:Length() end

---@return number
function Vec:Length2() end

---@param b Vector
---@return number
function Vec:Distance(b) end

---@param b Vector
---@return number
function Vec:Distance2(b) end

---@param b Vector
---@return number
function Vec:Angle(b) end

---@param b Vector
---@param n Vector
---@return number
function Vec:SignedAngle(b, n) end

---@return number
function Vec:Magnitude() end

---@return number
function Vec:LengthSquared() end

---@return number
function Vec:Magnitude2() end

---@return number
function Vec:MagnitudeSquared() end

---@param b Vector
---@return number
function Vec:DistanceSquared(b) end

function Vec(...) end
