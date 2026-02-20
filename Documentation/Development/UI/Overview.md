# Widget System Overview

Octave's 2D UI is built on a hierarchy of **Widget** nodes. Every widget inherits from `Node`, so you create, parent, and manage widgets the same way you manage any other node in the scene tree.

---

## Widget Hierarchy

```
Node
 └── Widget            (base: position, size, color, opacity, anchor, scissor)
      ├── Canvas        (empty container for grouping)
      ├── Quad          (textured rectangle)
      ├── Text          (rendered text string)
      ├── Button        (interactive, contains internal Quad + Text)
      ├── Poly          (arbitrary polygon / lines, Vulkan only)
      │    └── PolyRect (auto-generated rectangle outline, Vulkan only)
      └── ArrayWidget   (auto-layout container, C++ only)
```

All widgets share the base `Widget` properties: position, dimensions, color, opacity, anchor mode, rotation, scale, pivot, and scissor clipping. Specialized widgets add their own features on top.

---

## Creating Widgets

Widgets are nodes. You create them the same way you create any other node.

**C++**
```cpp
// As a child of an existing node
Quad* bg = parentNode->CreateChild<Quad>("Background");

// Or standalone
Quad* bg = Node::Construct<Quad>();
bg->SetName("Background");
parentNode->AddChild(bg);
```

**Lua**
```lua
-- As a child of an existing node (e.g. inside a script's Start())
local bg = self:CreateChild("Quad")
bg:SetName("Background")

-- Or construct standalone and attach later
local bg = Node.Construct("Quad")
bg:SetName("Background")
bg:Attach(parentNode)
```

**Editor:** Right-click in the scene hierarchy and select **Add Node > Widget/Quad/Text/Button/Canvas** to add a widget. Configure its properties in the Properties panel.

---

## The Anchor System

The anchor mode controls how a widget is positioned and sized relative to its parent. There are 16 anchor modes defined in `AnchorMode`:

### Corner and Edge Anchors (9 fixed-position modes)

These modes anchor the widget to a specific point on the parent. The widget's position (`SetPosition` / `SetOffset`) is relative to that anchor point, and the widget keeps its explicit width/height.

```
 TopLeft ──── TopMid ──── TopRight
    │            │            │
 MidLeft ────── Mid ────── MidRight
    │            │            │
 BottomLeft ─ BottomMid ─ BottomRight
```

| Mode | Anchor Point |
|---|---|
| `TopLeft` | Parent's top-left corner |
| `TopMid` | Parent's top-center edge |
| `TopRight` | Parent's top-right corner |
| `MidLeft` | Parent's left-center edge |
| `Mid` | Parent's center |
| `MidRight` | Parent's right-center edge |
| `BottomLeft` | Parent's bottom-left corner |
| `BottomMid` | Parent's bottom-center edge |
| `BottomRight` | Parent's bottom-right corner |

### Stretch Anchors (7 stretching modes)

These modes stretch the widget along one or both axes. Use `SetRatios()` to define the widget's area as a fraction of the parent.

| Mode | Stretches | Description |
|---|---|---|
| `TopStretch` | Horizontal | Fills parent width, anchored to top |
| `MidHorizontalStretch` | Horizontal | Fills parent width, centered vertically |
| `BottomStretch` | Horizontal | Fills parent width, anchored to bottom |
| `LeftStretch` | Vertical | Fills parent height, anchored to left |
| `MidVerticalStretch` | Vertical | Fills parent height, centered horizontally |
| `RightStretch` | Vertical | Fills parent height, anchored to right |
| `FullStretch` | Both | Fills entire parent area |

### Using Anchors

**C++ / Lua:**
```cpp
// C++
widget->SetAnchorMode(AnchorMode::FullStretch);
widget->SetRatios(0.0f, 0.0f, 1.0f, 1.0f); // fill entire parent

// Lua
widget:SetAnchorMode(AnchorMode.FullStretch)
widget:SetRatios(0.0, 0.0, 1.0, 1.0)
```

The `SetRatios(x, y, w, h)` parameters are fractions of the parent size:
- `x` — horizontal offset ratio (0.0 = left edge, 1.0 = right edge)
- `y` — vertical offset ratio (0.0 = top edge, 1.0 = bottom edge)
- `w` — width ratio (1.0 = full parent width)
- `h` — height ratio (1.0 = full parent height)

**Example:** A bottom bar spanning 80% width, centered, 10% height:
```lua
bar:SetAnchorMode(AnchorMode.FullStretch)
bar:SetRatios(0.1, 0.9, 0.8, 0.1)
```

You can check whether an anchor stretches on a given axis:
```lua
local stretchesX = widget:StretchX()
local stretchesY = widget:StretchY()
```

---

## Pixel vs Ratio Positioning

Widgets support two coordinate modes depending on the anchor:

- **Pixel-based** (non-stretch anchors): Use `SetPosition(x, y)` and `SetDimensions(w, h)` with pixel values.
- **Ratio-based** (stretch anchors): Use `SetRatios(x, y, w, h)` with values from 0.0 to 1.0 representing fractions of the parent.

You can also mix approaches using `SetOffset()` and `SetSize()`, or set individual components with `SetXRatio()`, `SetYRatio()`, `SetWidthRatio()`, `SetHeightRatio()`.

---

## Margins

Margins add pixel offsets to the edges of a widget when using stretch anchors. They are useful for adding padding inside a stretched layout.

```cpp
// C++
widget->SetMargins(10.0f, 10.0f, 10.0f, 10.0f); // left, top, right, bottom

// Lua
widget:SetMargins(10.0, 10.0, 10.0, 10.0)
```

Individual margins: `SetLeftMargin()`, `SetTopMargin()`, `SetRightMargin()`, `SetBottomMargin()`.

---

## Color and Opacity

Every widget has a color (`glm::vec4` / `Vec` in Lua) and an opacity (0–255 integer or 0.0–1.0 float).

**Color** is multiplied with any child-specific coloring (e.g., texture tint on Quad, text color on Text). Child widgets inherit opacity from their parents — a parent at 50% opacity makes all children at most 50% visible.

```cpp
// C++
widget->SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // red
widget->SetOpacity(128);       // half transparent (0-255)
widget->SetOpacityFloat(0.5f); // same thing (0.0-1.0)

// Lua
widget:SetColor(Vec(1, 0, 0, 1))
widget:SetOpacity(128)
widget:SetOpacityFloat(0.5)
```

---

## Transform: Pivot, Rotation, Scale

Widgets support 2D transforms:

- **Pivot** — the point around which rotation and scale are applied, as a ratio of the widget's dimensions (default `(0.5, 0.5)` = center).
- **Rotation** — rotation in degrees.
- **Scale** — 2D scale factor.

```cpp
// C++
widget->SetPivot({0.5f, 0.5f}); // rotate/scale around center
widget->SetRotation(45.0f);      // 45 degrees
widget->SetScale({2.0f, 2.0f}); // double size

// Lua
widget:SetPivot(0.5, 0.5)
widget:SetRotation(45.0)
widget:SetScale(2.0, 2.0)
```

---

## Scissor Clipping

Enable scissor to clip child widgets that extend beyond the parent's bounds. This is useful for scroll areas or masked content.

```cpp
// C++
widget->EnableScissor(true);

// Lua
widget:EnableScissor(true)
```

---

## Visibility and Active State

Widgets inherit visibility from `Node`:

```cpp
// C++
widget->SetVisible(false); // hides this widget and all children
widget->SetActive(false);  // disables ticking but still renders

// Lua
widget:SetVisible(false)
widget:SetActive(false)
```

---

## Input / Hit Testing

Widgets can detect mouse interaction:

```cpp
// C++
bool mouseOver = widget->ContainsMouse();
bool hitTest = widget->ContainsPoint(x, y);

// Lua
local mouseOver = widget:ContainsMouse()
local hitTest = widget:ContainsPoint(x, y)
```

Buttons handle input automatically (see [Buttons](Buttons.md)).

---

## Persistence Across Scenes

By default, widgets are destroyed when the scene changes. To keep a widget alive across scene loads (e.g., a persistent HUD):

```cpp
// C++
widget->SetPersistent(true);

// Lua
widget:SetPersistent(true)
```

---

## Further Reading

- [Displaying Images (Quad)](DisplayingImages.md)
- [Text](Text.md)
- [Buttons](Buttons.md)
- [Animation](Animation.md)
- [Building Complete UIs](BuildingUI.md)
- [Widget Lua API Reference](../../Lua/Nodes/Widgets/Widget.md)
