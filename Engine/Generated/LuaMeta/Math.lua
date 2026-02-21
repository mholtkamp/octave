--- @meta

---@class MathModule
Math = {}

---@param source number
---@param target number
---@param smoothing number
---@param deltaTime number
---@return Vector
function Math.Damp(source, target, smoothing, deltaTime) end

---@param source number
---@param target number
---@param smoothing number
---@param deltaTime number
---@return number
function Math.DampAngle(source, target, smoothing, deltaTime) end

---@param source number
---@param target number
---@param speed number
---@param deltaTime number
---@return Vector
function Math.Approach(source, target, speed, deltaTime) end

---@param source number
---@param target number
---@param speed number
---@param deltaTime number
---@return number
function Math.ApproachAngle(source, target, speed, deltaTime) end

---@param value number
---@param start number
---@param end number
---@return number
function Math.NormalizeRange(value, start, end) end

---@param inX number
---@param inMin number
---@param inMax number
---@param outMin number
---@param outMax number
---@return number
function Math.Map(inX, inMin, inMax, outMin, outMax) end

---@param inX number
---@param inMin number
---@param inMax number
---@param outMin number
---@param outMax number
---@return number
function Math.MapClamped(inX, inMin, inMax, outMin, outMax) end

---@param arg integer
---@return boolean
function Math.IsPowerOfTwo(arg) end

---@param a integer
---@param b integer
---@return number
function Math.Min(a, b) end

---@param a integer
---@param b integer
---@return number
function Math.Max(a, b) end

---@param x integer
---@param min integer
---@param max integer
---@return number
function Math.Clamp(x, min, max) end

---@param a number
---@param b number
---@param alpha number
---@return number
function Math.Lerp(a, b, alpha) end

---@param number integer
---@return number
function Math.Sign(number) end

---@param srcYaw number
---@param dir Vector
---@param speed number
---@param deltaTime number
---@return number
function Math.RotateYawTowardDirection(srcYaw, dir, speed, deltaTime) end

---@param i Vector
---@param n Vector
---@return Vector
function Math.Reflect(i, n) end

---@param vect Vector
---@return Vector
function Math.VectorToRotation(vect) end

---@param vect Vector
---@return Vector
function Math.VectorToQuat(vect) end

---@param rotVec Vector
---@return Vector
function Math.RotationToVector(rotVec) end

---@param quatVect Vector
---@return Vector
function Math.QuatToVector(quatVect) end

---@param seed integer
function Math.SeedRand(seed) end

---@param fMin number
---@param fMax number
---@return number
function Math.RandRange(fMin, fMax) end

---@param iMin integer
---@param iMax integer
---@return integer
function Math.RandRangeInt(iMin, iMax) end

---@param fMin number
---@param fMax number
---@return Vector
function Math.RandRangeVec(fMin, fMax) end

---@return number
function Math.Rand() end

---@param value number
---@param start number
---@param end number
---@return number
function Math.Wrap(value, start, end) end

---@param a number
---@param b number
---@param alpha number
---@return number
function Math.Mix(a, b, alpha) end
