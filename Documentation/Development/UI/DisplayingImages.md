# Displaying Images with Quad

The `Quad` widget renders a textured rectangle. Use it for backgrounds, icons, health bars, portraits, and any other image in your UI.

---

## Creating a Quad

**C++**
```cpp
#include "Nodes/Widgets/Quad.h"
#include "Assets/Texture.h"
#include "AssetManager.h"

Quad* icon = parentWidget->CreateChild<Quad>("Icon");
icon->SetTexture(LoadAsset<Texture>("T_Icon"));
icon->SetDimensions(64.0f, 64.0f);
```

**Lua**
```lua
local icon = self:CreateChild("Quad")
icon:SetName("Icon")
icon:SetTexture(LoadAsset("T_Icon"))
icon:SetDimensions(64.0, 64.0)
```

**Editor:** Add a **Quad** node in the scene hierarchy. In the Properties panel, assign a texture and set the dimensions.

---

## Sizing and Positioning

Quads follow the same anchor/position system as all widgets (see [Overview](Overview.md)).

**Fixed size at a specific position:**
```lua
local icon = self:CreateChild("Quad")
icon:SetAnchorMode(AnchorMode.TopLeft)
icon:SetPosition(20.0, 20.0)
icon:SetDimensions(48.0, 48.0)
icon:SetTexture(LoadAsset("T_Coin"))
```

**Stretch to fill parent:**
```lua
local bg = self:CreateChild("Quad")
bg:SetAnchorMode(AnchorMode.FullStretch)
bg:SetRatios(0.0, 0.0, 1.0, 1.0) -- fill entire parent
bg:SetTexture(LoadAsset("T_Background"))
```

**Proportional sizing (e.g., bottom 20% of screen):**
```lua
local bar = self:CreateChild("Quad")
bar:SetAnchorMode(AnchorMode.FullStretch)
bar:SetRatios(0.0, 0.8, 1.0, 0.2) -- bottom 20%
bar:SetTexture(LoadAsset("T_BarBackground"))
```

---

## UV Scale and Offset

By default a Quad maps the full texture across its area (UV scale `(1, 1)`, offset `(0, 0)`). Adjust these for tiling and scrolling effects.

### Tiling

Set UV scale greater than 1 to repeat the texture:

```lua
local tiledBg = self:CreateChild("Quad")
tiledBg:SetAnchorMode(AnchorMode.FullStretch)
tiledBg:SetRatios(0.0, 0.0, 1.0, 1.0)
tiledBg:SetTexture(LoadAsset("T_GridPattern"))
tiledBg:SetUvScale(Vec(4.0, 4.0)) -- tile 4x4
```

### Scrolling

Animate the UV offset over time for scrolling textures (e.g., clouds, water):

```lua
function MyUI:Tick(deltaTime)
    local offset = self.scrollBg:GetUvOffset()
    offset.x = offset.x + deltaTime * 0.1
    self.scrollBg:SetUvOffset(offset)
end
```

---

## Per-Vertex Colors

Quad supports setting a different color on each of its four corners to create gradient effects.

**C++**
```cpp
quad->SetColor(
    {1.0f, 0.0f, 0.0f, 1.0f},  // top-left (red)
    {0.0f, 0.0f, 1.0f, 1.0f},  // top-right (blue)
    {1.0f, 0.0f, 0.0f, 1.0f},  // bottom-left (red)
    {0.0f, 0.0f, 1.0f, 1.0f}   // bottom-right (blue)
);
```

**Lua**
```lua
quad:SetVertexColors(
    Vec(1, 0, 0, 1),  -- top-left (red)
    Vec(0, 0, 1, 1),  -- top-right (blue)
    Vec(1, 0, 0, 1),  -- bottom-left (red)
    Vec(0, 0, 1, 1)   -- bottom-right (blue)
)
```

Use a single color to tint the entire quad:
```lua
quad:SetColor(Vec(1.0, 0.5, 0.5, 1.0)) -- pinkish tint
```

---

## Opacity

Control transparency with opacity (see [Overview - Color and Opacity](Overview.md#color-and-opacity)):

```lua
quad:SetOpacityFloat(0.5) -- 50% transparent
```

Opacity is inherited from parent widgets, so fading a parent Canvas fades all its children.

---

## Example: Displaying a Simple Image

**C++**
```cpp
void MyHUD::Create()
{
    Widget::Create();

    SetAnchorMode(AnchorMode::FullStretch);
    SetRatios(0.0f, 0.0f, 1.0f, 1.0f);

    // Background image
    Quad* bg = CreateChild<Quad>("Background");
    bg->SetAnchorMode(AnchorMode::FullStretch);
    bg->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    bg->SetTexture(LoadAsset<Texture>("T_HudBackground"));

    // Player portrait in top-left
    Quad* portrait = CreateChild<Quad>("Portrait");
    portrait->SetAnchorMode(AnchorMode::TopLeft);
    portrait->SetPosition(16.0f, 16.0f);
    portrait->SetDimensions(64.0f, 64.0f);
    portrait->SetTexture(LoadAsset<Texture>("T_Portrait"));
}
```

**Lua**
```lua
function MyHUD:Start()
    self:SetAnchorMode(AnchorMode.FullStretch)
    self:SetRatios(0.0, 0.0, 1.0, 1.0)

    -- Background image
    self.bg = self:CreateChild("Quad")
    self.bg:SetAnchorMode(AnchorMode.FullStretch)
    self.bg:SetRatios(0.0, 0.0, 1.0, 1.0)
    self.bg:SetTexture(LoadAsset("T_HudBackground"))

    -- Player portrait in top-left
    self.portrait = self:CreateChild("Quad")
    self.portrait:SetAnchorMode(AnchorMode.TopLeft)
    self.portrait:SetPosition(16.0, 16.0)
    self.portrait:SetDimensions(64.0, 64.0)
    self.portrait:SetTexture(LoadAsset("T_Portrait"))
end
```

---

## Further Reading

- [Widget System Overview](Overview.md)
- [Quad Lua API Reference](../../Lua/Nodes/Widgets/Quad.md)
- [Animation](Animation.md) — animate quad opacity, position, and UV offset
