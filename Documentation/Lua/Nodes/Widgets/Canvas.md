# Canvas

A container widget that serves as a root for UI hierarchies. Canvas widgets can be used to group and organize other widgets.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---

Canvas inherits all methods from [Widget](Widget.md). It serves as a container and organizational element for building UI hierarchies.

### Usage

Canvas is typically used as a root widget or to group related widgets together. It can enable scissor clipping to constrain child widgets within its bounds.

```lua
-- Create a canvas
local canvas = self:CreateChild("Canvas")
canvas:SetDimensions(400, 300)
canvas:EnableScissor(true)  -- Clip children to canvas bounds

-- Add child widgets
local button = canvas:CreateChild("Button")
button:SetPosition(10, 10)
button:SetTextString("Click Me")
```

### XML UI Documents

Canvas widgets can also be used to display XML UI documents. See the UI documentation for details on creating and mounting UI documents.
