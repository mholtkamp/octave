# Building Complete UIs

This guide walks through building full UI screens from code, covering layout patterns, hierarchy best practices, and complete examples.

---

## Widget Hierarchy Best Practices

- **Canvas as root containers.** Use `Canvas` widgets as the root of each UI screen. Canvas is a lightweight container that adds no rendering overhead.
- **Anchor for responsiveness.** Use `FullStretch` with `SetRatios()` for elements that should scale with the screen. Use fixed-position anchors (`TopLeft`, `Mid`, etc.) for elements with a constant pixel size.
- **Group related widgets.** Parent related widgets under a shared Canvas so you can show/hide or move them together.
- **Naming conventions.** Name your widgets descriptively (`"HealthBar"`, `"ScoreText"`, `"PauseMenu"`) for easier debugging.

---

## Example: Pause Menu

A typical pause menu: a dimmed background, title text, and a column of buttons.

```
Canvas "PauseMenu"
 ├── Quad "DimOverlay"        (FullStretch, semi-transparent black)
 ├── Text "Title"             (centered, top area)
 ├── Button "ResumeBtn"       (centered, stacked)
 ├── Button "OptionsBtn"
 └── Button "QuitBtn"
```

### Lua

```lua
function PauseMenu:Start()
    self:SetAnchorMode(AnchorMode.FullStretch)
    self:SetRatios(0.0, 0.0, 1.0, 1.0)
    self:SetVisible(false) -- hidden by default

    -- Dim overlay
    self.overlay = self:CreateChild("Quad")
    self.overlay:SetAnchorMode(AnchorMode.FullStretch)
    self.overlay:SetRatios(0.0, 0.0, 1.0, 1.0)
    self.overlay:SetColor(Vec(0, 0, 0, 0.6))

    -- Title
    self.title = self:CreateChild("Text")
    self.title:SetAnchorMode(AnchorMode.TopMid)
    self.title:SetPosition(0.0, 60.0)
    self.title:SetDimensions(300.0, 60.0)
    self.title:SetTextSize(40.0)
    self.title:SetText("Paused")
    self.title:SetHorizontalJustification(Justification.Center)
    self.title:SetColor(Vec(1, 1, 1, 1))

    -- Buttons
    local buttonNames = {"Resume", "Options", "Quit"}
    self.buttons = {}

    for i, name in ipairs(buttonNames) do
        local btn = self:CreateChild("Button")
        btn:SetTextString(name)
        btn:SetAnchorMode(AnchorMode.Mid)
        btn:SetPosition(0.0, (i - 1) * 55.0 - 30.0)
        btn:SetDimensions(200.0, 45.0)
        btn:SetStateColors(
            Vec(0.3, 0.3, 0.3, 0.9),
            Vec(0.4, 0.4, 0.5, 1.0),
            Vec(0.2, 0.2, 0.2, 1.0),
            Vec(0.1, 0.1, 0.1, 0.5)
        )
        btn:ConnectSignal("Activated", self, PauseMenu.OnButton)
        self.buttons[i] = btn
    end

    -- Navigation
    for i, btn in ipairs(self.buttons) do
        btn:SetNavigation(
            self.buttons[i - 1],
            self.buttons[i + 1],
            nil, nil
        )
    end
end

function PauseMenu:Show()
    self:SetVisible(true)
    Button.SetSelected(self.buttons[1])
end

function PauseMenu:Hide()
    self:SetVisible(false)
end

function PauseMenu:OnButton()
    local selected = Button.GetSelected()
    if selected == self.buttons[1] then
        self:Hide()
        -- Resume game
    elseif selected == self.buttons[2] then
        -- Open options
    elseif selected == self.buttons[3] then
        -- Quit
    end
end
```

### C++

```cpp
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"

class PauseMenu : public Canvas
{
    DECLARE_NODE(PauseMenu, Canvas);

public:
    Quad* mOverlay = nullptr;
    Text* mTitle = nullptr;
    Button* mResumeBtn = nullptr;
    Button* mOptionsBtn = nullptr;
    Button* mQuitBtn = nullptr;

    static void OnButtonActivated(Node* listener, const std::vector<Datum>& args)
    {
        PauseMenu* menu = static_cast<PauseMenu*>(listener);
        Button* btn = static_cast<Button*>(args[0].GetPointer());

        if (btn == menu->mResumeBtn)
            menu->Hide();
        else if (btn == menu->mQuitBtn)
        {
            // Quit logic
        }
    }

    virtual void Create() override
    {
        Canvas::Create();
        SetAnchorMode(AnchorMode::FullStretch);
        SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
        SetVisible(false);

        // Dim overlay
        mOverlay = CreateChild<Quad>("DimOverlay");
        mOverlay->SetAnchorMode(AnchorMode::FullStretch);
        mOverlay->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
        mOverlay->SetColor({0.0f, 0.0f, 0.0f, 0.6f});

        // Title
        mTitle = CreateChild<Text>("Title");
        mTitle->SetAnchorMode(AnchorMode::TopMid);
        mTitle->SetPosition(0.0f, 60.0f);
        mTitle->SetDimensions(300.0f, 60.0f);
        mTitle->SetTextSize(40.0f);
        mTitle->SetText("Paused");
        mTitle->SetHorizontalJustification(Justification::Center);

        // Buttons
        const char* names[] = {"Resume", "Options", "Quit"};
        Button* buttons[] = {nullptr, nullptr, nullptr};

        for (int i = 0; i < 3; ++i)
        {
            buttons[i] = CreateChild<Button>(names[i]);
            buttons[i]->SetTextString(names[i]);
            buttons[i]->SetAnchorMode(AnchorMode::Mid);
            buttons[i]->SetPosition(0.0f, (float)(i * 55 - 30));
            buttons[i]->SetDimensions(200.0f, 45.0f);
            buttons[i]->ConnectSignal("Activated", this, OnButtonActivated);
        }

        mResumeBtn = buttons[0];
        mOptionsBtn = buttons[1];
        mQuitBtn = buttons[2];

        // Navigation
        mResumeBtn->SetNavDown(mOptionsBtn);
        mOptionsBtn->SetNavUp(mResumeBtn);
        mOptionsBtn->SetNavDown(mQuitBtn);
        mQuitBtn->SetNavUp(mOptionsBtn);
    }

    void Show()
    {
        SetVisible(true);
        Button::SetSelectedButton(mResumeBtn);
    }

    void Hide()
    {
        SetVisible(false);
    }
};
```

---

## Example: HUD with Health Bar and Score

A game HUD showing a health bar (Quad with dynamic width), score text, and a minimap image.

```
Canvas "HUD"
 ├── Quad "HealthBarBg"       (top-left, fixed size)
 ├── Quad "HealthBarFill"     (top-left, width changes with health)
 ├── Text "HealthText"        (overlaid on health bar)
 ├── Text "ScoreText"         (top-right)
 └── Quad "Minimap"           (bottom-right, fixed size)
```

### Lua

```lua
function GameHUD:Start()
    self:SetAnchorMode(AnchorMode.FullStretch)
    self:SetRatios(0.0, 0.0, 1.0, 1.0)

    -- Health bar background
    self.healthBg = self:CreateChild("Quad")
    self.healthBg:SetAnchorMode(AnchorMode.TopLeft)
    self.healthBg:SetPosition(20.0, 20.0)
    self.healthBg:SetDimensions(200.0, 24.0)
    self.healthBg:SetColor(Vec(0.2, 0.2, 0.2, 0.8))

    -- Health bar fill
    self.healthFill = self:CreateChild("Quad")
    self.healthFill:SetAnchorMode(AnchorMode.TopLeft)
    self.healthFill:SetPosition(20.0, 20.0)
    self.healthFill:SetDimensions(200.0, 24.0)
    self.healthFill:SetColor(Vec(0.2, 0.8, 0.2, 1.0))

    -- Health text overlay
    self.healthText = self:CreateChild("Text")
    self.healthText:SetAnchorMode(AnchorMode.TopLeft)
    self.healthText:SetPosition(20.0, 20.0)
    self.healthText:SetDimensions(200.0, 24.0)
    self.healthText:SetTextSize(16.0)
    self.healthText:SetHorizontalJustification(Justification.Center)
    self.healthText:SetVerticalJustification(Justification.Center)
    self.healthText:SetColor(Vec(1, 1, 1, 1))

    -- Score text (top-right)
    self.scoreText = self:CreateChild("Text")
    self.scoreText:SetAnchorMode(AnchorMode.TopRight)
    self.scoreText:SetPosition(-160.0, 20.0)
    self.scoreText:SetDimensions(150.0, 30.0)
    self.scoreText:SetTextSize(22.0)
    self.scoreText:SetHorizontalJustification(Justification.Right)
    self.scoreText:SetColor(Vec(1, 1, 1, 1))
    self.scoreText:SetText("Score: 0")

    -- Minimap (bottom-right)
    self.minimap = self:CreateChild("Quad")
    self.minimap:SetAnchorMode(AnchorMode.BottomRight)
    self.minimap:SetPosition(-140.0, -140.0)
    self.minimap:SetDimensions(128.0, 128.0)
    self.minimap:SetTexture(LoadAsset("T_Minimap"))

    self.maxHealth = 100
    self.currentHealth = 100
    self.score = 0
end

function GameHUD:SetHealth(current, max)
    self.currentHealth = current
    self.maxHealth = max
    local ratio = current / max

    -- Resize the fill bar
    self.healthFill:SetDimensions(200.0 * ratio, 24.0)

    -- Update color: green -> yellow -> red
    local r = 1.0 - ratio
    local g = ratio
    self.healthFill:SetColor(Vec(r, g, 0.2, 1.0))

    -- Update text
    self.healthText:SetText(tostring(current) .. " / " .. tostring(max))
end

function GameHUD:SetScore(score)
    self.score = score
    self.scoreText:SetText("Score: " .. tostring(score))
end
```

### C++

```cpp
class GameHUD : public Canvas
{
    DECLARE_NODE(GameHUD, Canvas);

public:
    Quad* mHealthBg = nullptr;
    Quad* mHealthFill = nullptr;
    Text* mHealthText = nullptr;
    Text* mScoreText = nullptr;
    Quad* mMinimap = nullptr;

    float mMaxHealth = 100.0f;
    float mBarWidth = 200.0f;

    virtual void Create() override
    {
        Canvas::Create();
        SetAnchorMode(AnchorMode::FullStretch);
        SetRatios(0.0f, 0.0f, 1.0f, 1.0f);

        // Health bar background
        mHealthBg = CreateChild<Quad>("HealthBg");
        mHealthBg->SetAnchorMode(AnchorMode::TopLeft);
        mHealthBg->SetPosition(20.0f, 20.0f);
        mHealthBg->SetDimensions(mBarWidth, 24.0f);
        mHealthBg->SetColor({0.2f, 0.2f, 0.2f, 0.8f});

        // Health bar fill
        mHealthFill = CreateChild<Quad>("HealthFill");
        mHealthFill->SetAnchorMode(AnchorMode::TopLeft);
        mHealthFill->SetPosition(20.0f, 20.0f);
        mHealthFill->SetDimensions(mBarWidth, 24.0f);
        mHealthFill->SetColor({0.2f, 0.8f, 0.2f, 1.0f});

        // Score text
        mScoreText = CreateChild<Text>("Score");
        mScoreText->SetAnchorMode(AnchorMode::TopRight);
        mScoreText->SetPosition(-160.0f, 20.0f);
        mScoreText->SetDimensions(150.0f, 30.0f);
        mScoreText->SetTextSize(22.0f);
        mScoreText->SetHorizontalJustification(Justification::Right);
        mScoreText->SetText("Score: 0");

        // Minimap
        mMinimap = CreateChild<Quad>("Minimap");
        mMinimap->SetAnchorMode(AnchorMode::BottomRight);
        mMinimap->SetPosition(-140.0f, -140.0f);
        mMinimap->SetDimensions(128.0f, 128.0f);
        mMinimap->SetTexture(LoadAsset<Texture>("T_Minimap"));
    }

    void SetHealth(float current, float max)
    {
        float ratio = glm::clamp(current / max, 0.0f, 1.0f);
        mHealthFill->SetDimensions(mBarWidth * ratio, 24.0f);
        mHealthFill->SetColor({1.0f - ratio, ratio, 0.2f, 1.0f});

        mHealthText->SetText(
            std::to_string((int)current) + " / " + std::to_string((int)max));
    }

    void SetScore(int score)
    {
        mScoreText->SetText("Score: " + std::to_string(score));
    }
};
```

---

## Showing and Hiding UI Screens

Toggle visibility to show/hide entire UI screens:

```lua
function MyGame:TogglePause()
    local visible = self.pauseMenu:IsVisible()
    self.pauseMenu:SetVisible(not visible)
end
```

Combine with opacity animation for smooth transitions (see [Animation](Animation.md)).

---

## Persistent Widgets

By default, all nodes are destroyed when a new scene loads. Mark a widget as persistent to keep it alive across scene transitions:

```lua
function GameHUD:Start()
    self:SetPersistent(true) -- survives scene loads
    -- ... build UI ...
end
```

```cpp
// C++
hud->SetPersistent(true);
```

This is useful for HUDs, notification systems, and debug overlays that should remain visible regardless of the current scene.

---

## ArrayWidget (C++ Only)

`ArrayWidget` automatically arranges its children in a vertical or horizontal list with configurable spacing. It is not exposed to Lua.

```cpp
#include "Nodes/Widgets/ArrayWidget.h"

ArrayWidget* list = parentWidget->CreateChild<ArrayWidget>("ButtonList");
list->SetAnchorMode(AnchorMode::Mid);
list->SetDimensions(200.0f, 300.0f);

// Default is Vertical orientation, set spacing between items
// Spacing and orientation are configured via properties in the editor

// Add children — they will be arranged automatically
for (int i = 0; i < 5; ++i)
{
    Button* btn = list->CreateChild<Button>(("Item" + std::to_string(i)).c_str());
    btn->SetTextString("Item " + std::to_string(i));
    btn->SetDimensions(180.0f, 40.0f);
}
```

ArrayWidget supports:
- **Vertical** or **Horizontal** orientation (`ArrayOrientation` enum)
- **Spacing** between children
- **Centering** children within the widget bounds (`SetCentered(true)`)

For Lua, arrange widgets manually using a loop with position offsets (as shown in the pause menu example above).

---

## PolyRect for Bordered Panels

`PolyRect` draws a rectangle outline (Vulkan only). Use it for selection indicators, panel borders, or debug outlines.

```cpp
#include "Nodes/Widgets/PolyRect.h"

PolyRect* border = parentWidget->CreateChild<PolyRect>("Border");
border->SetAnchorMode(AnchorMode::FullStretch);
border->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
border->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
border->SetLineWidth(2.0f);
```

**Lua**
```lua
local border = self:CreateChild("PolyRect")
border:SetAnchorMode(AnchorMode.FullStretch)
border:SetRatios(0.0, 0.0, 1.0, 1.0)
border:SetColor(Vec(1, 1, 1, 1))
border:SetLineWidth(2.0)
```

---

## Layout Tips

- **Layer order:** Children render on top of parents. Add backgrounds first, then foreground elements.
- **Responsive layout:** Use `FullStretch` + `SetRatios()` for elements that should scale with resolution. Use fixed pixel sizes for elements that should stay constant (icons, text).
- **Screen-relative positioning:** Anchor to the nearest corner/edge. A minimap in the bottom-right should use `BottomRight` anchor, not `TopLeft` with a large offset.
- **Margins for padding:** When using stretch anchors, use `SetMargins()` to add pixel-based padding inside the stretched area.

---

## Further Reading

- [Widget System Overview](Overview.md)
- [Displaying Images](DisplayingImages.md)
- [Text](Text.md)
- [Buttons](Buttons.md)
- [Animation](Animation.md)
- [Widget Lua API Reference](../../Lua/Nodes/Widgets/Widget.md)
