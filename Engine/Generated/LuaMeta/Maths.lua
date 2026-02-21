--- @meta

---@class MathsModule
Maths = {}

---@param source number
---@param target number
---@param smoothing number
---@param deltaTime number
---@return Vector
function Maths.Damp(source, target, smoothing, deltaTime) end

---@param source number
---@param target number
---@param smoothing number
---@param deltaTime number
---@return number
function Maths.DampAngle(source, target, smoothing, deltaTime) end

---@param source number
---@param target number
---@param speed number
---@param deltaTime number
---@return Vector
function Maths.Approach(source, target, speed, deltaTime) end

---@param source number
---@param target number
---@param speed number
---@param deltaTime number
---@return number
function Maths.ApproachAngle(source, target, speed, deltaTime) end

---@param value number
---@param start number
---@param end number
---@return number
function Maths.NormalizeRange(value, start, end) end

---@param inX number
---@param inMin number
---@param inMax number
---@param outMin number
---@param outMax number
---@return number
function Maths.Map(inX, inMin, inMax, outMin, outMax) end

---@param inX number
---@param inMin number
---@param inMax number
---@param outMin number
---@param outMax number
---@return number
function Maths.MapClamped(inX, inMin, inMax, outMin, outMax) end

---@param arg integer
---@return boolean
function Maths.IsPowerOfTwo(arg) end

---@param a integer
---@param b integer
---@return number
function Maths.Min(a, b) end

---@param a integer
---@param b integer
---@return number
function Maths.Max(a, b) end

---@param x integer
---@param min integer
---@param max integer
---@return number
function Maths.Clamp(x, min, max) end

---@param a number
---@param b number
---@param alpha number
---@return number
function Maths.Lerp(a, b, alpha) end

---@param number integer
---@return number
function Maths.Sign(number) end

---@param srcYaw number
---@param dir Vector
---@param speed number
---@param deltaTime number
---@return number
function Maths.RotateYawTowardDirection(srcYaw, dir, speed, deltaTime) end

---@param i Vector
---@param n Vector
---@return Vector
function Maths.Reflect(i, n) end

---@param vect Vector
---@return Vector
function Maths.VectorToRotation(vect) end

---@param vect Vector
---@return Vector
function Maths.VectorToQuat(vect) end

---@param rotVec Vector
---@return Vector
function Maths.RotationToVector(rotVec) end

---@param quatVect Vector
---@return Vector
function Maths.QuatToVector(quatVect) end

---@param seed integer
function Maths.SeedRand(seed) end

---@param fMin number
---@param fMax number
---@return number
function Maths.RandRange(fMin, fMax) end

---@param iMin integer
---@param iMax integer
---@return integer
function Maths.RandRangeInt(iMin, iMax) end

---@param fMin number
---@param fMax number
---@return Vector
function Maths.RandRangeVec(fMin, fMax) end

---@return number
function Maths.Rand() end

---@param value number
---@param start number
---@param end number
---@return number
function Maths.Wrap(value, start, end) end

---@param a number
---@param b number
---@param alpha number
---@return number
function Maths.Mix(a, b, alpha) end

Math = Maths
