# Camera3D

A node that represents a camera viewpoint which can be rendered. Change the camera used for rendering by calling World:SetActiveCamera().

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)

---

### EnablePerspective
Enable rendering with a perspective projection. When enabled, the camera's field of view will be used in conjunction with the Near and Far plane values. When disabled, an orthographic projection will be created using OrthoWidth, OrthoHeight, Near, and Far plane values.

Sig: `Camera3D:EnablePerspective(perspective)`
 - Arg: `boolean perspective` True for perspective, false for orthographic
---
### IsPerspectiveEnabled
Check if this camera is using a perspective projection.

Sig: `perspective = Camera3D:IsPerspectiveEnabled()`
 - Ret: `boolean perspective` True if using a perspective projection, false if using an orthographic projection
---
### IsOrtho
Check if this camera is using an orthographic projection.

Sig: `ortho = Camera3D:IsOrtho()`
 - Ret: `boolean ortho` True if using an orthographic projection, false if using perspective
---
### GetNear
Get the near plane distance. Anything in front of the near plane won't be rendered. Lowering the near plane distance may lower depth precision and cause z-fighting artifacts.

Sig: `near = Camera3D:GetNear()`
 - Ret: `number near` Near plane distance
---
### GetFar
Get the far plane distance. Anything beyond the far plane won't be rendered.

Sig: `far = Camera3D:GetFar()`
 - Ret: `number far` Far plane distance
---
### GetFieldOfView
Get the field of view along the vertical (Y) dimension. Used when perspective is enabled.

Sig: `fovY = Camera3D:GetFieldOfView()`
 - Ret: `number fovY` Y Field of view
---
### GetAspectRatio
Get the viewport aspect ratio (ViewportWidth / ViewportHeight).

Sig: `aspectRatio = Camera3D:GetAspectRatio()`
 - Ret: `number aspectRatio` Viewport aspect ratio
---
### GetOrthoWidth
Get the orthographic width of the camera when using an orthographic projection. Orthographic width is the world space units covered from the middle of the screen to either horizontal edge.

Sig: `orthoWidth = Camera3D:GetOrthoWidth()`
 - Ret: `number orthoWidth` Ortho width
---
### GetOrthoHeight
Get the orthographic height of the camera when using an orthographic projection. Orthographic height is the world space units covered from the middle of the screen to either vertical edge. The ortho height cannot be manually set. It is set automatically depending on the ortho width and the aspect ratio.

Sig: `orthoHeight = Camera3D:GetOrthoHeight()`
 - Ret: `number orthoHeight` Ortho height
---
### SetNear
Set the near plane distance. Anything in front of the near plane won't be rendered. Lowering the near plane value may lower depth precision and cause z-fighting artifacts.

Sig: `Camera3D:SetNear(near)`
 - Arg: `number near` Near plane distance
---
### SetFar
Set the far plane distance. Anything beyond the far plane won't be rendered.

Sig: `Camera3D:SetFar(far)`
 - Arg: `number far` Far plane distance
---
### SetFieldOfView
Set the field of view along the vertical (Y) dimension. Used when perspective is enabled.

Sig: `Camera3D:SetFieldOfView(fovY)`
 - Arg: `number fovY` Y field of view 
---
### SetOrthoWidth
Set the orthographic width of the camera when using an orthographic projection. Orthographic width is the world space units covered from the middle of the screen to either horizontal edge.

Sig: `Camera3D:SetOrthoWidth(orthoWidth)`
 - Arg: `number orthoWidth` Ortho width
---
### WorldToScreenPosition
Convert a world space position to screen space. The returned vector will contain the X/Y screen position in pixels in the corresponding .x/.y members. The .z member will be positive if the world position is in front of the camera and negative if behind.

Sig: `screenPos = Camera3D:WorldToScreenPosition(worldPos)`
 - Arg: `Vector worldPos` World position
 - Ret: `Vector screenPos` Screen position
---
### ScreenToWorldPosition
Convert a screen space position to its world space position on the near plane.

Sig: `worldPos = Camera3D:ScreenToWorldPosition(x, y)`
 - Arg: `number x` X screen position (pixels)
 - Arg: `number y` Y screen position (pixels)
 - Ret: `Vector worldPos` World position on near plane
---
### TraceScreenToWorld
Perform a ray test to determine where in the world a screen position corresponds to. Use the colMask argument to selectively choose which collision groups to trace against.

Sig: `worldPos = Camera3D:TraceScreenToWorld(x, y, colMask=0xff)`
 - Arg: `number x` X screen position (pixels)
 - Arg: `number y` Y screen position (pixels)
 - Arg: `integer colMask` Collision mask to trace against (0xff = trace against everything)
 - Ret: `Vector worldPos` World position
---