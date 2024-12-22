# Widget

A node which represents a user interface element and is rendered in 2D.

Inheritance:
* [Node](../Node.md)

---
### GetRect
Get the screen space (pixels) rectangle that determines where the widget is rendered. This is computed on Tick().

Sig: `rect = Widget:GetRect()`
 - Ret: `Rect rect` Screen space rect 
---
### UpdateRect
Forcefully update this widget's screen space rect.

Sig: `Widget:UpdateRect()`

---
### SetX
Set the X position of this widget in pixels.

Sig: `Widget:SetX(x)`
 - Arg: `number x` X position in pixels
---
### SetY
Set the Y position of this widget in pixels.

Sig: `Widget:SetY(y)`
 - Arg: `number y` Y position in pixels
---
### SetWidth
Set the width of this widget in pixels.

Sig: `Widget:SetWidth(width)`
 - Arg: `number width` Width in pixels
---
### SetHeight
Set the height of this widget in pixels.

Sig: `Widget:SetHeight(height)`
 - Arg: `number height` Height in pixels
---
### SetXRatio
Set the X position of this widget as a ratio of the parent widget width.

Sig: `Widget:SetXRatio(xRatio)`
 - Arg: `number xRatio` X as a ratio of parent width
---
### SetYRatio
Set the Y position of this widget as a ratio of the parent widget height.

Sig: `Widget:SetYRatio(yRatio)`
 - Arg: `number yRatio` Y as a ratio of parent height
---
### SetWidthRatio
Set the width of this widget as a ratio of the parent widget width.

Sig: `Widget:SetWidthRatio(widthRatio)`
 - Arg: `number widthRatio` Width as a ratio of parent width
---
### SetHeightRatio
Set the height of this widget as a ratio of the parent height.

Sig: `Widget:SetHeightRatio(heightRatio)`
 - Arg: `number heightRatio` Height as a ratio of parent height
---
### SetLeftMargin
Set the left margin.

Sig: `Widget:SetLeftMargin(margin)`
 - Arg: `number margin` Left margin
---
### SetTopMargin
Set the top margin.

Sig: `Widget:SetTopMargin(margin)`
 - Arg: `number margin` Top margin
---
### SetRightMargin
Set the right margin.

Sig: `Widget:SetRightMargin(margin)`
 - Arg: `number margin` Right margin
---
### SetBottomMargin
Set the bottom margin.

Sig: `Widget:SetBottomMargin(margin)`
 - Arg: `number margin` Bottom margin
---
### SetPosition
Set the position of this widget in pixels. A single Vector may be passed instead of the two separate arguments.

Sig: `Widget:SetPosition(x, y)`
 - Arg: `number x` X position
 - Arg: `number y` Y position
---
### SetDimensions
Set the dimensions of this widget in pixels. A single Vector may be passed instead of the two separate arguments.

Sig: `Widget:SetDimensions(width, height)`
 - Arg: `number width` Width
 - Arg: `number height` Height
---
### SetRect
Set position and dimensions. Note: this is not the same thing that GetRect() will return. GetRect() returns the final screen-space position/dimensions.

Sig: `Widget:SetRect(rect)`
 - Arg: `Rect rect` Local rect (x, y, w, h)
---
### SetRatios
Set the position/dimensions of this widget as ratios of the parent dimensions.

Sig: `Widget:SetRatios(x, y, width, height)`
 - Arg: `number x` X ratio
 - Arg: `number y` Y ratio
 - Arg: `number width` Width ratio
 - Arg: `number height` Height ratio
---
### SetMargins
Set the margins.

Sig: `Widget:SetMargins(left, top, right, bottom)`
 - Arg: `number left` Left margin
 - Arg: `number top` Top margin
 - Arg: `number right` Right margin
 - Arg: `number bottom` Bottom margin
---
### SetOffset
Explicitly set the widget's offset. The offset may be interpreted as either a pixel offset or a parent-ratio offset depending on the AnchorMode of the widget.

A single Vector may be passed instead of the two separate numbers.

Sig: `Widget:SetOffset(x, y)`
 - Arg: `number x` X offset
 - Arg: `number y` Y offset
---
### GetOffset
Get the widget's offset.

Sig: `offset = Widget:GetOffset()`
 - Ret: `Vector offset` Offset (in .x and .y)
---
### SetSize
Set the widget's size. The size may be interpreted as either a pixel size or a parent-ratio size depending on the AnchorMode of the widget.

A single Vector may be passed instead of the two separate numbers.

Sig: `Widget:SetSize(x, y)`
 - Arg: `number x` X size
 - Arg: `number y` Y size
---
### GetSize
Get the widget's size.

Sig: `size = Widget:GetSize()`
 - Ret: `Vector size` Size (in .x and .y)
---
### GetAnchorMode
Get the widget's anchor mode. The anchor mode determines how a widget is positioned and sized relative to its parent.

See [AnchorMode](../../Misc/Enums.md#anchormode)

Sig: `anchorMode = Widget:GetAnchorMode()`
 - Ret: `AnchorMode(integer) anchorMode` Widget anchor mode
---
### SetAnchorMode
Set the widget's anchor mode. The anchor mode determines how a widget is positioned and sized relative to its parent.

See [AnchorMode](../../Misc/Enums.md#anchormode)

Sig: `Widget:SetAnchorMode(anchorMode)`
 - Arg: `AnchorMode(integer) anchorMode`
---
### StretchX
Check if the widget should stretch along the X axis based on its anchor mode.

Sig: `stretch = Widget:StretchX()`
 - Ret: `boolean stretch` Stretch along X
---
### StretchY
Check if the widget should stretch along the Y axis based on its anchor mode.

Sig: `stretch = Widget:StretchY()`
 - Ret: `boolean stretch` Stretch along Y
---
### GetX
Get the widget's relative X position in pixels.

Sig: `x = Widget:GetX()`
 - Ret: `number x` Relative X in pixels
---
### GetY
Get the widget's relative Y position in pixels.

Sig: `y = Widget:GetY()`
 - Ret: `number y` Relative Y in pixels
---
### GetWidth
Get the widget's relative width in pixels.

Sig: `width = Widget:GetWidth()`
 - Ret: `number width` Relative width in pixels
---
### GetHeight
Get the widget's relative height in pixels.

Sig: `height = Widget:GetHeight()`
 - Ret: `number height` Relative height in pixels
---
### GetPosition
Get the widget's relative position in pixels.

Sig: `position = Widget:GetPosition()`
 - Ret: `Vector position` Relative position in pixels
---
### GetDimensions
Get the widget's relative dimensions in pixels.

Sig: `dimensions = Widget:GetDimensions()`
 - Ret: `Vector dimensions` Relative dimensions in pixels
---
### GetParentWidth
Get the parent widget's width.

Sig: `width = Widget:GetParentWidth()`
 - Ret: `number width` Parent's width
---
### GetParentHeight
Get the parent widget's height.

Sig: `height = Widget:GetParentHeight()`
 - Ret: `number height` Parent's height
---
### SetColor
Set the widget's color.

Sig: `Widget:SetColor(color)`
 - Arg: `Vector color` Widget color
---
### ShouldHandleInput
Check if this widget should handle input.

Sig: `handle = Widget:ShouldHandleInput()`
 - Ret: `boolean handle` Handle input
---
### MarkDirty
Mark this widget as dirty. When a widget is dirty, it will need to have its Rect updated on Tick(). Other data may be updated as well depending on the type of widget.

Sig: `Widget:MarkDirty()`

---
### IsDirty
Check if the widget is marked as dirty.

Sig: `dirty = Widget:IsDirty()`
 - Ret: `boolean dirty` Is widget dirty
---
### ContainsMouse
Check if the mouse is contained within the area of the widget (inside the Rect). The mouse will not be considered inside the Rect if it is outside the scissor bounds.

Sig: `contained = Widget:ContainsMouse()`
 - Ret: `boolean contained` Widget contains mouse
---
### ContainsPoint
Check if a point is within the area of the widget (inside the Rect). Clamps within the scissor bounds if scissor is enabled.

Sig: `contained = Widget:ContainsPoint(x, y)`
 - Arg: `number x` X screen position in pixels
 - Arg: `number y` Y screen position in pixels
 - Ret: `boolean contained` Widget contains point
---
### SetRotation
Set the widget rotation in degrees.

Sig: `Widget:SetRotation(rotation)`
 - Arg: `number rotation` Rotation in degrees
---
### GetRotation
Get the widget rotation in degrees.

Sig: `rotation = Widget:GetRotation()`
 - Ret: `number rotation` Rotation in degrees
---
### SetPivot
Set the widget's pivot point which it will rotate around. A pivot of <0,0> is the top left corner. <0.5, 0.5> would be in the middle. <1, 1> would be the bottom right.

Sig: `Widget:SetPivot(x, y)`
 - Arg: `number x` X pivot (should likely be between 0 and 1)
 - Arg: `number y` Y pivot (should likely be between 0 and 1)
---
### GetPivot
Get the widget's pivot point which it will rotate around. A pivot of <0,0> is the top left corner. <0.5, 0.5> would be in the middle. <1, 1> would be the bottom right.

Sig: `pivot = Widget:GetPivot()`
 - Ret: `Vector pivot` The pivot point (as a ratio in the range 0 to 1)
---
### SetScale
Set the widget's scale.

A single Vector argument can be used instead of the two number arguments.

Sig: `Widget:SetScale(x, y)`
 - Arg: `number x` X scale
 - Arg: `number y` Y scale
---
### GetScale
Get the widget's scale.

Sig: `scale = Widget:GetScale()`
 - Ret: `Vector scale` Widget scale
---
### IsScissorEnabled
Check if the scissor is enabled. When the scissor is enabled, child widgets will be clipped to stay within the bounds of this widget.

Sig: `scissor = Widget:IsScissorEnabled()`
 - Ret: `boolean scissor` Whether scissor is enabled
---
### EnableScissor
Set whether to enable the scissor. When the scissor is enabled, child widgets will be clipped to stay within the bounds of this widget.

Sig: `Widget:EnableScissor(scissor)`
 - Arg: `boolean scissor` Whether to enable scissor
---
### SetOpacity
Set the widget's opacity as an integer between 0 and 255. When rendered, a widget's opacity is compounded with its parent opacity.

Sig: `Widget:SetOpacity(opacity)`
 - Arg: `integer opacity` Widget opacity (0 to 255)
---
### SetOpacityFloat
Set the widget's opacity as a floating point number between 0 and 1. When rendered, a widget's opacity is compounded with its parent opacity.

Sig: `Widget:SetOpacityFloat(opacity)`
 - Arg: `number opacity` Widget opacity (0 to 1)
---
### GetOpacity
Get the widget's opacity as an integer between 0 and 255. When rendered, a widget's opacity is compounded with its parent opacity.

Sig: `opacity = Widget:GetOpacity()`
 - Ret: `integer opacity` Widget opacity (0 to 255)
---
### GetOpacityFloat
Get the widget's opacity as a floating point number between 0 and 1. When rendered, a widget's opacity is compounded with its parent opacity.

Sig: `opacity = Widget:GetOpacityFloat()`
 - Ret: `number opacity` Widget opacity (0 to 1)
---
