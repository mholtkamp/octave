--- @meta

---@class Widget : Node
Widget = {}

---@return Rect
function Widget:GetRect() end

function Widget:UpdateRect() end

---@param value number
function Widget:SetX(value) end

---@param value number
function Widget:SetY(value) end

---@param value number
function Widget:SetWidth(value) end

---@param value number
function Widget:SetHeight(value) end

---@param value number
function Widget:SetXRatio(value) end

---@param value number
function Widget:SetYRatio(value) end

---@param value number
function Widget:SetWidthRatio(value) end

---@param value number
function Widget:SetHeightRatio(value) end

---@param value number
function Widget:SetLeftMargin(value) end

---@param value number
function Widget:SetTopMargin(value) end

---@param value number
function Widget:SetRightMargin(value) end

---@param value number
function Widget:SetBottomMargin(value) end

---@param x number
---@param y number
function Widget:SetPosition(x, y) end

---@param x number
---@param y number
function Widget:SetDimensions(x, y) end

---@param rect Rect
function Widget:SetRect(rect) end

---@param x number
---@param y number
---@param w number
---@param h number
function Widget:SetRatios(x, y, w, h) end

---@param left number
---@param top number
---@param right number
---@param bottom number
function Widget:SetMargins(left, top, right, bottom) end

---@param x number
---@param y number
function Widget:SetOffset(x, y) end

---@return Vector
function Widget:GetOffset() end

---@param x number
---@param y number
function Widget:SetSize(x, y) end

---@return Vector
function Widget:GetSize() end

---@return integer
function Widget:GetAnchorMode() end

---@param value integer
function Widget:SetAnchorMode(value) end

---@return boolean
function Widget:StretchX() end

---@return boolean
function Widget:StretchY() end

---@return number
function Widget:GetX() end

---@return number
function Widget:GetY() end

---@return number
function Widget:GetWidth() end

---@return number
function Widget:GetHeight() end

---@return Vector
function Widget:GetPosition() end

---@return Vector
function Widget:GetDimensions() end

---@return number
function Widget:GetParentWidth() end

---@return number
function Widget:GetParentHeight() end

---@param value Vector
function Widget:SetColor(value) end

---@return boolean
function Widget:ShouldHandleInput() end

function Widget:MarkDirty() end

---@return boolean
function Widget:IsDirty() end

---@return boolean
function Widget:ContainsMouse() end

---@param x integer
---@param y integer
---@return boolean
function Widget:ContainsPoint(x, y) end

---@param value number
function Widget:SetRotation(value) end

---@return number
function Widget:GetRotation() end

---@param x number
---@param y number
function Widget:SetPivot(x, y) end

---@return Vector
function Widget:GetPivot() end

---@param x number
---@param y number
function Widget:SetScale(x, y) end

---@return Vector
function Widget:GetScale() end

---@return boolean
function Widget:IsScissorEnabled() end

---@param value boolean
function Widget:EnableScissor(value) end

---@param value integer
function Widget:SetOpacity(value) end

---@param value number
function Widget:SetOpacityFloat(value) end

---@return integer
function Widget:GetOpacity() end

---@return number
function Widget:GetOpacityFloat() end
