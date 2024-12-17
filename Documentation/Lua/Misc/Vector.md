# Vector

A table that represents a 4-dimensional vector. Even though the vector has 4 components, it can be used for 2D and 3D vectors as well. Vectors are also used to store colors, Euler rotations, and quaternions.

The .x, .y, .z, .w keys can be used to read and write to the components individually. Alternatively, .r, .g, .b, .a can be used, which makes more sense for Vectors that store color data.

Vectors can be created by using the `Vec()` global function. This is equivalent to calling Vector.Create().

Examples:
`local pos = Vec()` Creates a vector with all components set to 0
`local dir = Vec(0, 0, 1)` Creates a vector by specifying 3 components (4th component set to 0)

Addition, subtraction, multiplication, and division of vectors can be performed by using `+` `-` `*` `/` symbols.

Vector equality can be tested by using == and ~= operators.

NOTE: Many of the functions in Vector are written as static non-member functions because it may be easier to think of the operation that way instead of invoking a member function on a Vector instance. However, these functions may be called either way. For instance, you can take the max of two vectors like this `max = Vector.Max(a, b)` but you can also do the exact same thing like this `max = a:Max(b)`.

---
### Create
Create a new Vector. Instead of passing in individual components, you pass in another Vector instead.

Note: Use the global function `Vec()` as a shorthand for `Vector.Create()`.

Sig: `vector = Vector.Create(x=0, y=0, z=0, w=0)`
 - Arg: `number x` X component
 - Arg: `number y` Y component
 - Arg: `number z` Z component
 - Arg: `number w` W component
 - Ret: `Vector vector` Newly created vector

Sig: `vector = Vector.Create(src)`
 - Arg: `Vector src` Source vector to copy
 - Ret: `Vector vector` Newly created vector
---
### Set
Set the components of this Vector. If a provided component is nil, the vector will keep its original value for that component.

Sig: `Vector:Set(x=nil, y=nil, z=nil, w=nil)`
 - Arg: `number x` X component
 - Arg: `number y` Y component
 - Arg: `number z` Z component
 - Arg: `number w` W component

Sig: `Vector:Set(src)`
 - Arg: `Vector src` Source vector to copy
---
### Clone
Make a clone of this vector.

Sig: `clone = Vector:Clone()`
 - Ret: `Vector clone` Newly created clone
---
### Dot
Take the dot product between two vectors.

Sig: `dot = Vector.Dot(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `number dot` Dot product
---
### Dot3
Take the dot product between two 3D vectors (ignore the 4th component).

Sig: `dot = Vector.Dot3(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `number dot` Dot product
---
### Cross
Take the cross product of two 3D vectors.

Sig: `cross = Vector.Cross(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `Vector cross` Cross product
---
### Lerp
Create a new vector by performing a component-wise linear interpolation between two vectors.

Sig: `lerped = Vector.Lerp(a, b, alpha)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Arg: `number alpha` Interpolation factor (0 to 1)
 - Ret: `Vector lerped` Linearly interpolated result
---
### Max
Create a new Vector by taking the maximum of each component of two vectors.

Sig: `max = Vector.Max(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `Vector max` Component-wise max
---
### Min
Create a new Vector by taking the minimum of each component of two vectors.

Sig: `min = Vector.Min(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `Vector min` Component-wise min
---
### Clamp
Create a new Vector by clamping this vector between a min and max.

Sig: `clamped = Vector:Clamp(min, max)`
 - Arg: `Vector min` Min vector
 - Arg: `Vector max` Max vector
 - Ret: `Vector clamped` Clamped vector
---
### Normalize
Normalize the vector.

Sig: `normal = Vector:Normalize()`
 - Ret: `Vector normal` Normalized vector
---
### Normalize3
Normalize the vector, ignoring the 4th component.

Sig: `normal = Vector:Normalize3()`
 - Ret: `Vector normal` Normalized vector
---
### Reflect
Reflect this vector against a normal.

Sig: `reflected = Vector:Reflect(normal)`
 - Arg: `Vector normal` Normal vector
 - Ret: `Vector reflected` Reflected vector
---
### Damp
Smoothly move a source Vector toward a destination Vector. Framerate independent.

Sig: `damped = Vector.Damp(source, target, smoothing, deltaTime)`
 - Arg: `Vector source` Source vector
 - Arg: `Vector target` Target vector
 - Arg: `number smoothing` Smoothing factor (0 - 1) Lower values will move slower. Try 0.005.
 - Arg: `number deltaTime` Delta time
 - Ret: `Vector damped` Damped vector
---
### Rotate
Rotate a vector around an axis.

Sig: `rotated = Vector:Rotate(angle, axis)`
 - Arg: `number angle` Angle in degrees
 - Arg: `Vector axis` Axis of rotation
 - Ret: `Vector rotated` Rotated vector
---
### Length
Get the length of this Vector.

Alias: `Magnitude`

Sig: `length = Vector:Length()`
 - Ret: `number length` Vector length/size/magnitude
---
### Distance
Get the distance between two vectors.

Sig: `dist = Vector.Distance(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `number dist` Distance between the two vectors
---
### Distance2
Get the squared distance between two vectors.

Sig: `dist = Vector.Distance2(a, b)`
 - Arg: `Vector a` First vector
 - Arg: `Vector b` Second vector
 - Ret: `number dist` Squared distance between the two vectors
---
### Angle
Get the angle between two vectors.

Sig: `angle = Vector.Angle(a, b)`
 - Arg: `Vector a` First vector (must be normalized)
 - Arg: `Vector b` Second vector (must be normalized)
 - Ret: `number angle` Angle in degrees
---
### SignedAngle
Get the signed angle between two vectors.

Sig: `angle = Vector.SignedAngle(a, b)`
 - Arg: `Vector a` First vector (must be normalized)
 - Arg: `Vector b` Second vector (must be normalized)
 - Ret: `number angle` Signed angle in degrees
---
### Negate
Negate this vector. You can use the unary minus operator instead of calling this function (i.e. -myVector).

Sig: `negated = Vector:Negate()`
 - Ret: `Vector negated` The negated vector
---
