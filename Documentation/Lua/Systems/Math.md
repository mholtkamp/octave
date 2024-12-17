# Math

A system for common math functions

---
### Damp
Smoothly move a source value toward a destination value. Framerate independent.

Sig: `value = Math.Damp(source, target, smoothing, deltaTime)`
 - Arg: `Vector/number source` Source value
 - Arg: `Vector/number target` Target value
 - Arg: `number smoothing` Smoothing factor (0 - 1) Lower values will move slower. Try 0.005.
 - Arg: `number deltaTime` Delta time
 - Ret: `number value` Smoothed value
---
### DampAngle
Smoothly move a source angle toward a destination angle. Framerate independent. Will handle the 0/360 degree boundary correctly.

Sig: `value = Math.DampAngle(source, target, smoothing, deltaTime)`
 - Arg: `number source` Source angle in degrees
 - Arg: `number target` Target angle in degrees
 - Arg: `number smoothing` Smoothing factor (0 - 1) Lower values will move slower. Try 0.005.
 - Arg: `number deltaTime` Delta time
 - Ret: `number value` Smoothed angle
---
### Approach
Approach a target value from a source value based on a given speed.

Sig: `value = Math.Approach(source, target, speed, deltaTime)`
 - Arg: `Vector/number source` Source value
 - Arg: `Vector/number target` Target value
 - Arg: `number speed` Rate at which the source should be moving toward the target
 - Arg: `number deltaTime` Delta time
 - Ret: `number value` The updated value
---
### ApproachAngle
Approach a target angle from a source angle based on a given speed.

Sig: `value = Math.ApproachAngle(source, target, speed, deltaTime)`
 - Arg: `number source` Source angle in degrees
 - Arg: `number target` Target angle in degrees
 - Arg: `number speed` Rate at which the source should be moving toward the target
 - Arg: `number deltaTime` Delta time
 - Ret: `number value` The updated value
---
### Wrap
Wrap a value into a particular range.

Alias: `NormalizeRange`

Sig: `wrapped = Math.Wrap(value, start, end)`
 - Arg: `number value` Input value
 - Arg: `number start` Range start
 - Arg: `number end` Range end
 - Ret: `number wrapped` Wrapped value within the start and end range
---
### Map
Map a value from an input range to an output range.

Sig: `outX = Math.Map(inX, inMin, inMax, outMin, outMax)`
 - Arg: `number inX` Input value
 - Arg: `number inMin` Input range min
 - Arg: `number inMax` Input range max
 - Arg: `number outMin` Output range min
 - Arg: `number outMax` Output range max
 - Ret: `number outX` Output value
---
### MapClamped
Map a value from an input range to an output range, clamping it within the output range.

Sig: `outX = Math.MapClamped(inX, inMin, inMax, outMin, outMax)`
 - Arg: `number inX` Input value
 - Arg: `number inMin` Input range min
 - Arg: `number inMax` Input range max
 - Arg: `number outMin` Output range min
 - Arg: `number outMax` Output range max
 - Ret: `number outX` Output value clamped within the output range
---
### IsPowerOfTwo
Check if an integer is a power of 2.

Sig: `pow2 = Math.IsPowerOfTwo(value)`
 - Arg: `integer value` Value to check
 - Ret: `boolean pow2` Whether the input value is a power of 2
---
### Min
Get the minimum of two numbers.

Sig: `min = Math.Min(a, b)`
 - Arg: `integer/number a` First number
 - Arg: `integer/number b` Second number
 - Ret: `integer/number min` Minimum number
---
### Max
Get the maximum of two numbers.

Sig: `max = Math.Max(a, b)`
 - Arg: `integer/number a` First number
 - Arg: `integer/number b` Second number
 - Ret: `integer/number max` Maximum number
---
### Clamp
Clamp an input value between two numbers.

Sig: `clamped = Math.Clamp(value, min, max)`
 - Arg: `integer/number value` Input value
 - Arg: `integer/number min` Min value
 - Arg: `integer/number max` Max value
 - Ret: `integer/number clamped` Clamped value
---
### Lerp
Linearly interpolate a value between two numbers based on a ratio.

Alias: `Mix`

Sig: `res = Math.Lerp(a, b, alpha)`
 - Arg: `number a` First number
 - Arg: `number b` Second number
 - Arg: `number alpha` Ratio (should be between 0 and 1)
 - Ret: `number res` The mixed result
---
### Sign
Check if a number is positive or negative. If positive (or 0) 1 is returned. If negative, -1 is returned.

Sig: `sign = Math.Sign(value)`
 - Arg: `integer/number value` Input value to test
 - Ret: `integer/number sign` Sign of the value (-1 or 1)
---
### RotateYawTowardDirection
Rotate a yaw angle towards a 3D direction. Useful for things like rotating a character to look in the direction they are running.

Sig: `resYaw = Math.RotateYawTowardDirection(srcYaw, dir, speed, deltaTime)`
 - Arg: `number srcYaw` Source yaw in degrees
 - Arg: `Vector dir` Direction to rotate towards
 - Arg: `number speed` Angular speed
 - Arg: `number deltaTime` Delta time
 - Ret: `number resYaw` Resulting yaw
---
### Reflect
Reflect a vector against a normal.

Sig: `res = Math.Reflect(incident, normal)`
 - Arg: `Vector incident` Incident vector
 - Arg: `Vector normal` Normal direction
 - Ret: `Vector res` Reflected vector 
---
### VectorToRotation
Convert a vector to an euler angle rotation that would map the forward vector (0,0,-1) to the input vector.

Sig: `rotEuler = Math.VectorToRotation(vector)`
 - Arg: `Vector vector` Input vector
 - Ret: `Vector rotEuler` Rotation as euler angles in degrees
---
### VectorToQuat
Convert a vector to a quaternion rotation that would map the forward vector (0,0,-1) to the input vector.

Sig: `rotQuat = Math.VectorToQuat(vector)`
 - Arg: `Vector vector` Input vector
 - Ret: `Vector rotQuat` Rotation as a quaternion
---
### RotationToVector
Convert an euler angle rotation in degrees to a normalized directional vector.

Sig: `vect = Math.RotationToVector(rotEuler)`
 - Arg: `Vector rotEuler` Euler angle rotation in degrees
 - Ret: `Vector vect` Equivalent vector
---
### QuatToVector
Convert a quaternion rotation to a normalized directional vector.

Sig: `vect = Math.QuatToVector(rotQuat)`
 - Arg: `Vector rotQuat` Quaternion rotation
 - Ret: `Vector vect` Equivalent vector
---
### SeedRand
Seed the Math system's random number generator.

Sig: `Math.SeedRand(seed)`
 - Arg: `integer seed` RNG seed
---
### RandRange
Get a random number between a minimum and maximum value.

Sig: `rand = Math.RandRange(min, max)`
 - Arg: `number min` Min range
 - Arg: `number max` Max range
 - Ret: `number rand` Random number
---
### RandRangeInt
Get a random integer between a minimum and maximum value.

Sig: `rand = Math.RandRangeInt(min, max)`
 - Arg: `integer min` Min range
 - Arg: `integer max` Max range
 - Ret: `integer rand` Random number
---
### RandRangeVec
Get a random Vector between a minimum and maximum value.

Sig: `rand = Math.RandRangeVec(min, max)`
 - Arg: `Vector/number min` Min range
 - Arg: `Vector/number max` Max range
 - Ret: `Vector rand` Random vector
---
### Rand
Get a random number between 0 and 1.

Sig: `random = Math.Rand()`
 - Ret: `number random` Random number between 0 and 1.
---
