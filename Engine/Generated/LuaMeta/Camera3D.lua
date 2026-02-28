--- @meta

---@class Camera3D : Node3D
Camera3D = {}

---@param persp boolean
function Camera3D:EnablePerspective(persp) end

---@return boolean
function Camera3D:IsPerspectiveEnabled() end

---@return boolean
function Camera3D:IsOrtho() end

---@return number
function Camera3D:GetNear() end

---@return number
function Camera3D:GetFar() end

---@return number
function Camera3D:GetFieldOfView() end

---@return number
function Camera3D:GetAspectRatio() end

---@return number
function Camera3D:GetOrthoWidth() end

---@return number
function Camera3D:GetOrthoHeight() end

---@param value number
function Camera3D:SetNear(value) end

---@param value number
function Camera3D:SetFar(value) end

---@param value number
function Camera3D:SetFieldOfView(value) end

---@param value number
function Camera3D:SetOrthoWidth(value) end

---@param worldPos Vector
---@return Vector
function Camera3D:WorldToScreenPosition(worldPos) end

---@param x integer
---@param y integer
---@return Vector
function Camera3D:ScreenToWorldPosition(x, y) end

---@param x integer
---@param y integer
---@param colMask? integer
---@return Vector, Node
function Camera3D:TraceScreenToWorld(x, y, colMask) end
