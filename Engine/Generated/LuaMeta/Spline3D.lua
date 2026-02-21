--- @meta

---@class Spline3D : Node3D
local Spline3D = {}

---@param p Vector
function Spline3D:AddPoint(p) end

function Spline3D:ClearPoints() end

---@return integer
function Spline3D:GetPointCount() end

---@param index integer
---@return Vector
function Spline3D:GetPoint(index) end

---@param index integer
---@param p Vector
function Spline3D:SetPoint(index, p) end

---@param t number
---@return Vector
function Spline3D:GetPositionAt(t) end

---@param t number
---@return Vector
function Spline3D:GetTangentAt(t) end

function Spline3D:Play() end

function Spline3D:Stop() end

---@param paused boolean
function Spline3D:SetPaused(paused) end

---@return boolean
function Spline3D:IsPaused() end

---@param index integer
---@param enabled boolean
function Spline3D:SetFollowLinkEnabled(index, enabled) end

---@param index integer
---@return boolean
function Spline3D:IsFollowLinkEnabled(index) end

---@param index integer
---@param arg2? number
---@return boolean
function Spline3D:IsNearLinkFrom(index, arg2) end

---@param index integer
---@param arg2? number
---@return boolean
function Spline3D:IsNearLinkTo(index, arg2) end

---@param index integer
---@param arg2? number
---@return boolean
function Spline3D:IsLinkDirectionForward(index, arg2) end

---@param index integer
---@return boolean
function Spline3D:TriggerLink(index) end

function Spline3D:CancelActiveLink() end
