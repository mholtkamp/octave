--- @meta

---@class GizmosModule
Gizmos = {}

---@param color Vector
function Gizmos.SetColor(color) end

---@return Vector
function Gizmos.GetColor() end

function Gizmos.SetMatrix() end

---@return number, any, any, any, any, any, any, any, any, any, any, any, any, any, any, any
function Gizmos.GetMatrix() end

function Gizmos.ResetState() end

---@param centerV Vector
---@param sizeV Vector
function Gizmos.DrawCube(centerV, sizeV) end

---@param centerV Vector
---@param radius number
function Gizmos.DrawSphere(centerV, radius) end

---@param mesh StaticMesh
---@param posV Vector
---@param rv? Vector
---@param sv? Vector
function Gizmos.DrawMesh(mesh, posV, rv, sv) end

---@param centerV Vector
---@param sizeV Vector
function Gizmos.DrawWireCube(centerV, sizeV) end

---@param centerV Vector
---@param radius number
function Gizmos.DrawWireSphere(centerV, radius) end

---@param mesh StaticMesh
---@param posV Vector
---@param rv? Vector
---@param sv? Vector
function Gizmos.DrawWireMesh(mesh, posV, rv, sv) end

---@param fromV Vector
---@param toV Vector
function Gizmos.DrawLine(fromV, toV) end

---@param originV Vector
---@param directionV Vector
function Gizmos.DrawRay(originV, directionV) end

function Gizmos.DrawFrustum() end
