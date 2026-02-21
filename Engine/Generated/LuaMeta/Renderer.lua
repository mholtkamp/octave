--- @meta

---@class RendererModule
Renderer = {}

---@param value boolean
function Renderer.EnableStatsOverlay(value) end

---@param value boolean
function Renderer.EnableConsole(value) end

function Renderer.DirtyAllWidgets() end

---@return integer
function Renderer.GetFrameNumber() end

---@return integer
function Renderer.GetFrameIndex() end

---@return integer
function Renderer.GetScreenIndex() end

---@param arg1? integer
---@return Vector
function Renderer.GetScreenResolution(arg1) end

---@return Vector
function Renderer.GetActiveScreenResolution() end

---@return number
function Renderer.GetGlobalUiScale() end

---@param scale number
function Renderer.SetGlobalUiScale(scale) end

---@param arg1 integer
function Renderer.SetDebugMode(arg1) end

---@return integer
function Renderer.GetDebugMode() end

---@param value boolean
function Renderer.EnableProxyRendering(value) end

---@return boolean
function Renderer.IsProxyRenderingEnabled() end

---@param arg1 integer
function Renderer.SetBoundsDebugMode(arg1) end

---@return integer
function Renderer.GetBoundsDebugMode() end

---@param value boolean
function Renderer.EnableFrustumCulling(value) end

---@return boolean
function Renderer.IsFrustumCullingEnabled() end

---@param mesh StaticMesh
---@param pos Vector
---@param arg3? Vector
---@param arg4? Vector
---@param arg5? Vector
---@param arg6? number
---@param arg7? Material
function Renderer.AddDebugDraw(mesh, pos, arg3, arg4, arg5, arg6, arg7) end

---@param start Vector
---@param end Vector
---@param color Vector
---@param life number
function Renderer.AddDebugLine(start, end, color, life) end

---@param enable boolean
function Renderer.Enable3dRendering(enable) end

---@return boolean
function Renderer.Is3dRenderingEnabled() end

---@param enable boolean
function Renderer.Enable2dRendering(enable) end

---@return boolean
function Renderer.Is2dRenderingEnabled() end

---@return boolean
function Renderer.IsLightFadeEnabled() end

---@param value boolean
function Renderer.EnableLightFade(value) end

---@param value integer
function Renderer.SetLightFadeLimit(value) end

---@return integer
function Renderer.GetLightFadeLimit() end

---@param value number
function Renderer.SetLightFadeSpeed(value) end

---@return number
function Renderer.GetLightFadeSpeed() end

---@param value number
function Renderer.SetResolutionScale(value) end

---@return number
function Renderer.GetResolutionScale() end

---@param value Vector
function Renderer.SetClearColor(value) end
