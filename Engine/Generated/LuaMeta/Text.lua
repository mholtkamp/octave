--- @meta

---@class Text : Widget
Text = {}

---@param font Font
function Text:SetFont(font) end

---@return any
function Text:GetFont() end

---@param value Vector
function Text:SetOutlineColor(value) end

---@return Vector
function Text:GetOutlineColor() end

---@param value number
function Text:SetTextSize(value) end

---@return number
function Text:GetTextSize() end

---@return number
function Text:GetScaledTextSize() end

---@return number
function Text:GetOutlineSize() end

---@return number
function Text:GetSoftness() end

---@return number
function Text:GetCutoff() end

---@param value string
function Text:SetText(value) end

---@return string
function Text:GetText() end

---@return number
function Text:GetTextWidth() end

---@return number
function Text:GetTextHeight() end

---@return Vector
function Text:GetScaledMinExtent() end

---@return Vector
function Text:GetScaledMaxExtent() end

---@param value integer
function Text:SetHorizontalJustification(value) end

---@return integer
function Text:GetHorizontalJustification() end

---@param value integer
function Text:SetVerticalJustification(value) end

---@return integer
function Text:GetVerticalJustification() end

---@param enable boolean
function Text:EnableWordWrap(enable) end

---@return boolean
function Text:IsWordWrapEnabled() end
