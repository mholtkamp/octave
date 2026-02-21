--- @meta

---@class TextMesh3D : StaticMesh3D
local TextMesh3D = {}

---@param text string
function TextMesh3D:SetText(text) end

---@return string
function TextMesh3D:GetText() end

---@param font Font
function TextMesh3D:SetFont(font) end

---@return any
function TextMesh3D:GetFont() end

---@param value Vector
function TextMesh3D:SetColor(value) end

---@return Vector
function TextMesh3D:GetColor() end

---@param blendMode integer
function TextMesh3D:SetBlendMode(blendMode) end

---@return integer
function TextMesh3D:GetBlendMode() end

---@param just number
function TextMesh3D:SetHorizontalJustification(just) end

---@return number
function TextMesh3D:GetHorizontalJustification() end

---@param just number
function TextMesh3D:SetVerticalJustification(just) end

---@return number
function TextMesh3D:GetVerticalJustification() end
