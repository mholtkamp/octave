--- @meta

---@class Vector
---@field x number
---@field y number
---@field z number
---@field w number
--- Vector + Vector|number -> Vector
--- Vector - Vector|number -> Vector
--- Vector * Vector|number -> Vector
--- Vector / Vector|number -> Vector
--- -Vector -> Vector
--- Vector == Vector -> boolean
Vector = {}

function Vector:Create() end

---@param srcVec Vector
function Vector:Set(srcVec) end

---@return Vector
function Vector:Clone() end

---@return Vector
function Vector:Add() end

---@return Vector
function Vector:Subtract() end

---@return Vector
function Vector:Multiply() end

---@return Vector
function Vector:Divide() end

---@param right Vector
---@return boolean
function Vector:Equals(right) end

---@param right Vector
---@return number
function Vector:Dot(right) end

---@param right Vector
---@return number
function Vector:Dot3(right) end

---@param right Vector
---@return Vector
function Vector:Cross(right) end

---@param b Vector
---@param alpha number
---@return Vector
function Vector:Lerp(b, alpha) end

---@param b Vector
---@return Vector
function Vector:Max(b) end

---@param b Vector
---@return Vector
function Vector:Min(b) end

---@param min Vector
---@param max Vector
---@return Vector
function Vector:Clamp(min, max) end

---@return Vector
function Vector:Normalize() end

---@return Vector
function Vector:Normalize3() end

---@param nrm3 Vector
---@return Vector
function Vector:Reflect(nrm3) end

---@param dst Vector
---@param smoothing number
---@param deltaTime number
---@return Vector
function Vector:Damp(dst, smoothing, deltaTime) end

---@param angle number
---@param axis3 Vector
---@return Vector
function Vector:Rotate(angle, axis3) end

---@return number
function Vector:Length() end

---@return number
function Vector:Length2() end

---@param b Vector
---@return number
function Vector:Distance(b) end

---@param b Vector
---@return number
function Vector:Distance2(b) end

---@param b Vector
---@return number
function Vector:Angle(b) end

---@param b Vector
---@param n Vector
---@return number
function Vector:SignedAngle(b, n) end

---@return number
function Vector:Magnitude() end

---@return number
function Vector:LengthSquared() end

---@return number
function Vector:Magnitude2() end

---@return number
function Vector:MagnitudeSquared() end

---@param b Vector
---@return number
function Vector:DistanceSquared(b) end

function Vec(...) end
