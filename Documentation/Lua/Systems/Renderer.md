# Renderer

System that controls graphical rendering.

---
### EnableStatsOverlay
Enable the stats overlay which can be used for displaying performance information.

Sig: `Renderer.EnableStatsOverlay(enable)`
 - Arg: `boolean enable` Enable stats overlay
---
### EnableConsole
Enable the console. The console will display log messages on screen. It currently doesn't accept any sort of console commands.

Sig: `Renderer.EnableConsole(enable)`
 - Arg: `boolean enable` Enable console
---
### DirtyAllWidgets
Dirty all widgets. Probably shouldn't be called...

Sig: `Renderer.DirtyAllWidgets()`

---
### GetFrameNumber
Get the current frame number.

Sig: `frameNum = Renderer.GetFrameNumber()`
 - Ret: `integer frameNum` Frame number
---
### GetFrameIndex
Get the frame index. Either 0 or 1.

Sig: `index = Renderer.GetFrameIndex()`
 - Ret: `integer index` Frame index
---
### GetScreenIndex
Get the screen index that is currently being rendered to. Not sure if this is useful to Lua code.

Sig: `index = Renderer.GetScreenIndex()`
 - Ret: `integer index` Screen index being rendered
---
### GetScreenResolution
Get the screen resolution.

Sig: `res = Renderer.GetScreenResolution(index=1)`
 - Arg: `integer index` Screen index 
---
### GetGlobalUiScale
Get the global UI scale.

Sig: `scale = Renderer.GetGlobalUiScale()` 
 - Ret: `number scale` Global UI scale
---
### SetGlobalUiScale
Set the global UI scale.

Sig: `Renderer.SetGlobalUiScale(scale)` 
 - Arg: `number scale` Global UI scale
---
### SetDebugMode
Set the debug mode.
Debug mode:
 - 0 = None
 - 1 = Shadow map
 - 2 = Wireframe
 - 3 = Collision

Sig: `Renderer.SetDebugMode(mode)`
 - Arg: `integer mode` Debug mode
---
### GetDebugMode
Get the debug mode.
Debug mode:
 - 0 = None
 - 1 = Shadow map
 - 2 = Wireframe
 - 3 = Collision

Sig: `mode = Renderer.GetDebugMode()`
 - Ret: `integer mode` Debug mode
---
### EnableProxyRendering
Enable proxy rendering. Proxy rendering is used to visualize where non-renderable components are located. For instance, Node3D, PointLight3D, Box3D...

Sig: `Renderer.EnableProxyRendering(enable)`
 - Arg: `boolean enable` Enable proxy rendering
---
### IsProxyRenderingEnabled
Check if proxy rendering is enabled. Proxy rendering is used to visualize where non-renderable components are located. For instance, Node3D, PointLight3D, Box3D...

Sig: `enabled = Renderer.IsProxyRenderingEnabled()`
 - Ret: `boolean enabled` Is proxy rendering enabled
---
### SetBoundsDebugMode
Enable bounds debug rendering.

Bounds Debug Mode:
 - 0 = Off
 - 1 = All primitives
 - 2 = Selected primitive

Sig: `Renderer.SetBoundsDebugMode(mode)`
 - Arg: `integer mode` Bounds debug mode
---
### GetBoundsDebugMode
Get the bounds debug rendering mode.

Bounds Debug Mode:
 - 0 = Off
 - 1 = All primitives
 - 2 = Selected primitive

Sig: `mode = Renderer.GetBoundsDebugMode()`
 - Ret: `integer mode` Bounds debug mode
---
### EnableFrustumCulling
Enable/disable frustum culling. This is enabled by default. Disabling this will hurt performance.

Sig: `Renderer.EnableFrustumCulling(enable)`
 - Arg: `boolean enable` Enable frustum culling
---
### IsFrustumCullingEnabled
Check if frustum culling is enabled.

Sig: `enabled = Renderer.IsFrustumCullingEnabled()`
 - Ret: `boolean enabled` Frustum culling enabled
---
### AddDebugDraw
Add a debug draw.

Sig: `Renderer.AddDebugDraw(mesh, pos, rot=Vec(0,0,0), scale=Vec(1,1,1), color=Vec(0.25, 0.25, 1.0, 1.0), life=0, material=nil)`
 - Arg: `StaticMesh mesh` Mesh to render
 - Arg: `Vector pos` Position
 - Arg: `Vector rot` Rotation
 - Arg: `Vector scale` Scale
 - Arg: `Vector color` Color (for wireframe rendering)
 - Arg: `number life` Life of draw (0 = draw for current frame only)
 - Arg: `Material material` Material to render. Wireframe draw if nil.
---
### AddDebugLine
Add a debug line.

Sig: `Renderer.AddDebugLine(start, end, color, life)`
 - Arg: `Vector start` Line start position
 - Arg: `Vector end` Line end position
 - Arg: `Vector color` Line color
 - Arg: `number life` Life of draw (0 = draw for current frame only)
---
### Enable3dRendering
Enable/disable 3D rendering.

Sig: `Renderer.Enable3dRendering(enable)`
 - Arg: `boolean enable` Enable 3D rendering
---
### Is3dRenderingEnabled
Check if 3D rendering is enabled.

Sig: `enabled = Renderer.Is3dRenderingEnabled()`
 - Ret: `boolean enabled` Is 3D rendering enabled
---
### Enable2dRendering
Enable/disable 2D rendering.

Sig: `Renderer.Enable2dRendering(enable)`
 - Arg: `boolean enable` Enable 2D rendering
---
### Is2dRenderingEnabled
Check if 2D rendering is enabled.

Sig: `enabled = Renderer.Is2dRenderingEnabled()`
 - Ret: `boolean enabled` Is 2D rendering enabled
---
### IsLightFadeEnabled
Check if light fade is enabled. Light fading is a way to reduce the number of active lights to improve performance. The lights closest to the camera will be visible.

Sig: `enabled = Renderer.IsLightFadeEnabled()`
 - Ret: `boolean enabled` Is light fade enabled
---
### EnableLightFade
Set whether light fade is enabled. Light fading is a way to reduce the number of active lights to improve performance. The lights closest to the camera will be visible.

Sig: `Renderer.EnableLightFade(enable)`
 - Arg: `boolean enable` Enable light fade
---
### SetLightFadeLimit
Set the maximum number of lights that can be rendered when light fade is enabled.

Sig: `Renderer.SetLightFadeLimit(limit)`
 - Arg: `integer limit` Light count limit
---
### GetLightFadeLimit
Get the maximum number of lights that can be rendered when light fade is enabled.

Sig: `limit = Renderer.GetLightFadeLimit()`
 - Ret: `integer limit` Light count limit
---
### SetLightFadeSpeed
Set the light fade in/out speed when light fade is enabled. Light fade speed of 1 = completely fade out a light from full brightness to nothing in the span of one second.

Sig: `Renderer.SetLightFadeSpeed(speed)`
 - Arg: `number speed` Fade speed
---
### GetLightFadeSpeed
Get the light fade in/out speed when light fade is enabled.

Sig: `speed = Renderer.GetLightFadeSpeed()`
 - Ret: `number speed` Fade speed
---
### SetResolutionScale
Set the resolution scale. Only supported on Vulkan platforms.

Sig: `Renderer.SetResolutionScale(scale)`
 - Arg: `number scale` Resolution scale (1.0 = 100% resolution)
---
### GetResolutionScale
Get the resolution scale. Resolution scale is only supported on Vulkan platforms.

Sig: `scale = Renderer.GetResolutionScale()`
 - Ret: `number scale` Resolution scale
---
