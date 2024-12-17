# Rect 

A table that represents a 2D rectangle.

---
### Create
Create a Rect.

Sig: `rect = Rect.Create()`
 - Ret: `Rect rect` Newly created Rect

Sig: `rect = Rect.Create(x, y, w, h)`
 - Arg: `number x` Top left x position
 - Arg: `number y` Top left y position
 - Arg: `number w` Width
 - Arg: `number h` Height
 - Ret: `Rect rect` Newly created Rect

Sig: `rect = Rect.Create(srcRect)`
 - Arg: `Rect srcRect` Rect to copy from
 - Ret: `Rect rect` Newly created Rect
---
### ContainsPoint
Check if this Rect contains a point.

Sig: `contained = Rect:ContainsPoint(x, y)`
 - Arg: `number x` Point x position
 - Arg: `number y` Point y position
 - Ret: `boolean contained` Is point contained in Rect
---
### OverlapsRect
Check if this Rect overlaps another Rect.

Sig: `overlaps = Rect:OverlapsRect(otherRect)`
 - Arg: `Rect otherRect` Other Rect to check
 - Ret: `boolean overlaps` Do the Rects overlap
---
### Clamp
Clamp this Rect to be contained within the bounds of another Rect.

Sig: `Rect:Clamp(other)`
 - Arg: `Rect other` Other Rect
---
### Top
Get the top Y position.

Sig: `top = Rect:Top()`
 - Ret: `number top` Top y value
---
### Bottom
Get the bottom Y position.

Sig: `bottom = Rect:Bottom()`
 - Ret: `number bottom` Bottom y value
---
### Left
Get the left X position.

Sig: `left = Rect:Left()`
 - Ret: `number left` Left x value
---
### Right
Get the right X position.

Sig: `right = Rect:Right()`
 - Ret: `number right` Right x value
---
