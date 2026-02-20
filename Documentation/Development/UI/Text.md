# Displaying and Updating Text

The `Text` widget renders a string using a signed-distance-field font. Use it for scores, labels, dialogue, timers, and any other on-screen text.

---

## Creating a Text Widget

**C++**
```cpp
#include "Nodes/Widgets/Text.h"

Text* label = parentWidget->CreateChild<Text>("ScoreLabel");
label->SetText("Score: 0");
label->SetTextSize(24.0f);
```

**Lua**
```lua
local label = self:CreateChild("Text")
label:SetName("ScoreLabel")
label:SetText("Score: 0")
label:SetTextSize(24.0)
```

**Editor:** Add a **Text** node in the scene hierarchy. Set the text string, font, and size in the Properties panel.

---

## Setting Font

Assign a font asset. If no font is set, the engine uses its default font.

```cpp
// C++
label->SetFont(LoadAsset<Font>("F_MainFont"));

// Lua
label:SetFont(LoadAsset("F_MainFont"))
```

---

## Text Size

Text size is in pixels:

```lua
label:SetTextSize(32.0)
local size = label:GetTextSize()
local scaledSize = label:GetScaledTextSize() -- accounts for widget scale
```

---

## Color

Text color is set through the widget's `SetColor()`:

```cpp
// C++
label->SetColor({1.0f, 1.0f, 0.0f, 1.0f}); // yellow

// Lua
label:SetColor(Vec(1, 1, 0, 1)) -- yellow
```

---

## Updating Text Dynamically

Change the displayed string at any time with `SetText()`. This is how you update scores, timers, and other dynamic values.

**C++**
```cpp
void ScoreHUD::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);
    std::string scoreStr = "Score: " + std::to_string(mScore);
    mScoreText->SetText(scoreStr);
}
```

**Lua**
```lua
function ScoreHUD:Tick(deltaTime)
    self.scoreText:SetText("Score: " .. tostring(self.score))
end
```

### Timer Example

```lua
function TimerUI:Start()
    self.timeRemaining = 60.0
    self.timerText = self:CreateChild("Text")
    self.timerText:SetAnchorMode(AnchorMode.TopMid)
    self.timerText:SetPosition(0.0, 10.0)
    self.timerText:SetTextSize(28.0)
    self.timerText:SetHorizontalJustification(Justification.Center)
end

function TimerUI:Tick(deltaTime)
    self.timeRemaining = self.timeRemaining - deltaTime
    if self.timeRemaining < 0.0 then
        self.timeRemaining = 0.0
    end

    local minutes = math.floor(self.timeRemaining / 60)
    local seconds = math.floor(self.timeRemaining % 60)
    self.timerText:SetText(string.format("%d:%02d", minutes, seconds))
end
```

---

## Justification

Text supports horizontal and vertical alignment within the widget's bounds.

### Horizontal Justification

| Value | Behavior |
|---|---|
| `Justification.Left` | Align text to the left edge (default) |
| `Justification.Center` | Center text horizontally |
| `Justification.Right` | Align text to the right edge |

### Vertical Justification

| Value | Behavior |
|---|---|
| `Justification.Top` | Align text to the top (default) |
| `Justification.Center` | Center text vertically |
| `Justification.Bottom` | Align text to the bottom |

```cpp
// C++
label->SetHorizontalJustification(Justification::Center);
label->SetVerticalJustification(Justification::Center);

// Lua
label:SetHorizontalJustification(Justification.Center)
label:SetVerticalJustification(Justification.Center)
```

---

## Word Wrapping

Enable word wrap to automatically break text at the widget's width boundary:

```lua
local description = self:CreateChild("Text")
description:SetAnchorMode(AnchorMode.FullStretch)
description:SetRatios(0.1, 0.3, 0.8, 0.4)
description:SetTextSize(18.0)
description:EnableWordWrap(true)
description:SetText("This is a long description that will wrap to fit within the widget bounds.")
```

---

## Outline

Text supports an outline effect for readability against busy backgrounds:

```cpp
// C++
label->SetOutlineColor({0.0f, 0.0f, 0.0f, 1.0f}); // black outline

// Lua
label:SetOutlineColor(Vec(0, 0, 0, 1))
```

The outline size, softness, and cutoff are configurable in the Properties panel in the editor. You can read these values at runtime:

```lua
local outlineSize = label:GetOutlineSize()
local softness = label:GetSoftness()
local cutoff = label:GetCutoff()
```

---

## Measuring Text Dimensions

Get the rendered width and height of the current text string. This is useful for dynamically sizing containers or positioning adjacent elements.

```lua
local width = label:GetTextWidth()
local height = label:GetTextHeight()
```

Scaled extents account for the widget's scale:
```lua
local minExtent = label:GetScaledMinExtent()
local maxExtent = label:GetScaledMaxExtent()
```

---

## Example: Score Counter

**C++**
```cpp
void GameHUD::Create()
{
    Canvas::Create();
    SetAnchorMode(AnchorMode::FullStretch);
    SetRatios(0.0f, 0.0f, 1.0f, 1.0f);

    mScoreText = CreateChild<Text>("Score");
    mScoreText->SetAnchorMode(AnchorMode::TopRight);
    mScoreText->SetPosition(-150.0f, 10.0f);
    mScoreText->SetDimensions(140.0f, 40.0f);
    mScoreText->SetTextSize(24.0f);
    mScoreText->SetHorizontalJustification(Justification::Right);
    mScoreText->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    mScoreText->SetText("Score: 0");
}

void GameHUD::UpdateScore(int score)
{
    mScoreText->SetText("Score: " + std::to_string(score));
}
```

**Lua**
```lua
function GameHUD:Start()
    self:SetAnchorMode(AnchorMode.FullStretch)
    self:SetRatios(0.0, 0.0, 1.0, 1.0)

    self.scoreText = self:CreateChild("Text")
    self.scoreText:SetAnchorMode(AnchorMode.TopRight)
    self.scoreText:SetPosition(-150.0, 10.0)
    self.scoreText:SetDimensions(140.0, 40.0)
    self.scoreText:SetTextSize(24.0)
    self.scoreText:SetHorizontalJustification(Justification.Right)
    self.scoreText:SetColor(Vec(1, 1, 1, 1))
    self.scoreText:SetText("Score: 0")
end

function GameHUD:UpdateScore(score)
    self.scoreText:SetText("Score: " .. tostring(score))
end
```

---

## Further Reading

- [Widget System Overview](Overview.md)
- [Text Lua API Reference](../../Lua/Nodes/Widgets/Text.md)
- [Buttons](Buttons.md) — buttons contain an internal Text widget
