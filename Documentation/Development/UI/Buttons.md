# Interactive Buttons

The `Button` widget provides a clickable UI element with built-in state management, visual feedback, and gamepad/keyboard navigation. Internally, a Button contains a `Quad` (background) and a `Text` (label) as children.

---

## Creating a Button

**C++**
```cpp
#include "Nodes/Widgets/Button.h"

Button* btn = parentWidget->CreateChild<Button>("PlayButton");
btn->SetTextString("Play");
btn->SetDimensions(200.0f, 50.0f);
btn->SetAnchorMode(AnchorMode::Mid);
```

**Lua**
```lua
local btn = self:CreateChild("Button")
btn:SetName("PlayButton")
btn:SetTextString("Play")
btn:SetDimensions(200.0, 50.0)
btn:SetAnchorMode(AnchorMode.Mid)
```

**Editor:** Add a **Button** node in the scene hierarchy. Set the text string, textures, and colors in the Properties panel.

---

## Handling Button Clicks

### Lua — Script Function

The simplest approach: define an `OnActivated()` function on a script attached to the Button node. The engine calls it automatically when the button is activated.

```lua
function MyButton:OnActivated()
    Log.Debug("Button was clicked!")
end
```

### Lua — Signal Connection

Connect to the `"Activated"` signal to handle clicks on buttons that are children of your script node:

```lua
function MyMenu:Start()
    self.playBtn = self:CreateChild("Button")
    self.playBtn:SetTextString("Play")
    self.playBtn:SetDimensions(200.0, 50.0)
    self.playBtn:SetAnchorMode(AnchorMode.Mid)
    self.playBtn:ConnectSignal("Activated", self, MyMenu.OnPlayClicked)
end

function MyMenu:OnPlayClicked()
    Log.Debug("Play button clicked!")
end
```

### C++ — Signal Connection

Use `ConnectSignal` with a `SignalHandlerFP` function pointer:

```cpp
#include "Nodes/Widgets/Button.h"

static void HandlePlayClicked(Node* listener, const std::vector<Datum>& args)
{
    // args[0] contains the Button* that was activated
    LogDebug("Play button clicked!");
}

void MyMenu::Create()
{
    Canvas::Create();

    mPlayBtn = CreateChild<Button>("Play");
    mPlayBtn->SetTextString("Play");
    mPlayBtn->SetDimensions(200.0f, 50.0f);
    mPlayBtn->SetAnchorMode(AnchorMode::Mid);
    mPlayBtn->ConnectSignal("Activated", this, HandlePlayClicked);
}
```

### C++ — Virtual Override

If you subclass Button, override `Activate()`:

```cpp
class PlayButton : public Button
{
    DECLARE_NODE(PlayButton, Button);

    virtual void Activate() override
    {
        Button::Activate(); // emits signals
        // Custom logic here
    }
};
```

---

## Button States

Buttons have four visual states:

| State | When |
|---|---|
| `Normal` | Default idle state |
| `Hovered` | Mouse cursor is over the button |
| `Pressed` | Mouse button is held down on the button |
| `Locked` | Button is disabled, ignores all input |

Check or set the current state:

```lua
local state = btn:GetState()
if state == ButtonState.Locked then
    -- button is locked
end
```

---

## Per-State Textures

Assign different textures for each state to give visual feedback:

**C++**
```cpp
btn->SetNormalTexture(LoadAsset<Texture>("T_BtnNormal"));
btn->SetHoveredTexture(LoadAsset<Texture>("T_BtnHovered"));
btn->SetPressedTexture(LoadAsset<Texture>("T_BtnPressed"));
btn->SetLockedTexture(LoadAsset<Texture>("T_BtnLocked"));
```

**Lua**
```lua
btn:SetStateTextures(
    LoadAsset("T_BtnNormal"),
    LoadAsset("T_BtnHovered"),
    LoadAsset("T_BtnPressed"),
    LoadAsset("T_BtnLocked")
)
```

Pass `nil` for any state you don't need a unique texture for.

---

## Per-State Colors

Set different colors for each state (applied to the internal Quad by default):

**C++**
```cpp
btn->SetNormalColor({0.5f, 0.5f, 0.5f, 1.0f});
btn->SetHoveredColor({0.7f, 0.7f, 0.7f, 1.0f});
btn->SetPressedColor({0.3f, 0.3f, 0.3f, 1.0f});
btn->SetLockedColor({0.2f, 0.2f, 0.2f, 0.5f});
```

**Lua**
```lua
btn:SetStateColors(
    Vec(0.5, 0.5, 0.5, 1.0), -- normal
    Vec(0.7, 0.7, 0.7, 1.0), -- hovered
    Vec(0.3, 0.3, 0.3, 1.0), -- pressed
    Vec(0.2, 0.2, 0.2, 0.5)  -- locked
)
```

Control whether state colors apply to the Quad, the Text, or both:
```cpp
// C++
btn->SetUseQuadStateColor(true);  // apply state colors to background (default: true)
btn->SetUseTextStateColor(false); // apply state colors to text (default: false)
```

---

## Locking / Unlocking

Lock a button to disable all interaction:

```lua
btn:SetLocked() -- sets state to Locked
```

To unlock, you would need to set the state back by interacting with it or managing state manually.

---

## Accessing Internal Widgets

A Button's internal Quad and Text are accessible for fine-grained control:

```lua
local quad = btn:GetQuad()
local text = btn:GetText()

-- Customize the text widget directly
text:SetTextSize(20.0)
text:SetFont(LoadAsset("F_CustomFont"))

-- Customize the quad directly
quad:SetTexture(LoadAsset("T_CustomBackground"))
```

---

## Gamepad / Keyboard Navigation

Connect buttons into a navigation graph so players can move between them with a gamepad D-pad or arrow keys:

**Lua**
```lua
function MyMenu:Start()
    self.playBtn = self:CreateChild("Button")
    self.playBtn:SetTextString("Play")
    self.playBtn:SetAnchorMode(AnchorMode.Mid)
    self.playBtn:SetPosition(0.0, -30.0)
    self.playBtn:SetDimensions(200.0, 40.0)

    self.optionsBtn = self:CreateChild("Button")
    self.optionsBtn:SetTextString("Options")
    self.optionsBtn:SetAnchorMode(AnchorMode.Mid)
    self.optionsBtn:SetPosition(0.0, 20.0)
    self.optionsBtn:SetDimensions(200.0, 40.0)

    self.quitBtn = self:CreateChild("Button")
    self.quitBtn:SetTextString("Quit")
    self.quitBtn:SetAnchorMode(AnchorMode.Mid)
    self.quitBtn:SetPosition(0.0, 70.0)
    self.quitBtn:SetDimensions(200.0, 40.0)

    -- Set up vertical navigation
    self.playBtn:SetNavigation(nil, self.optionsBtn, nil, nil)    -- down -> options
    self.optionsBtn:SetNavigation(self.playBtn, self.quitBtn, nil, nil) -- up -> play, down -> quit
    self.quitBtn:SetNavigation(self.optionsBtn, nil, nil, nil)    -- up -> options

    -- Select first button for gamepad/keyboard users
    Button.SetSelected(self.playBtn)
end
```

**C++**
```cpp
mPlayBtn->SetNavUp(nullptr);
mPlayBtn->SetNavDown(mOptionsBtn);
mOptionsBtn->SetNavUp(mPlayBtn);
mOptionsBtn->SetNavDown(mQuitBtn);
mQuitBtn->SetNavUp(mOptionsBtn);
mQuitBtn->SetNavDown(nullptr);

Button::SetSelectedButton(mPlayBtn);
```

Navigation parameters for `SetNavigation()` are: `(up, down, left, right)`. Pass `nil`/`nullptr` for directions that have no target.

---

## Enabling / Disabling Input Types

Control which input methods buttons respond to globally:

```lua
Button.EnableMouseHandling(true)
Button.EnableGamepadHandling(true)
Button.EnableKeyboardHandling(true)
```

```cpp
// C++
Button::SetHandleMouse(true);
Button::SetHandleGamepad(true);
Button::SetHandleKeyboard(true);
```

Disable mouse handling when you want gamepad-only UI, or vice versa.

---

## State Change Signal

Connect to the `"StateChanged"` signal to react when a button's visual state changes (e.g., for sound effects):

```lua
btn:ConnectSignal("StateChanged", self, MyMenu.OnButtonStateChanged)

function MyMenu:OnButtonStateChanged()
    -- Play hover sound, update visuals, etc.
end
```

---

## Right-Click Support

Enable right-click activation:

```cpp
// C++
btn->EnableRightClickPress(true);

// Lua (via C++ only — not exposed to Lua)
```

---

## Example: Menu with Multiple Buttons

```lua
function MainMenu:Start()
    self:SetAnchorMode(AnchorMode.FullStretch)
    self:SetRatios(0.0, 0.0, 1.0, 1.0)

    -- Background
    local bg = self:CreateChild("Quad")
    bg:SetAnchorMode(AnchorMode.FullStretch)
    bg:SetRatios(0.0, 0.0, 1.0, 1.0)
    bg:SetTexture(LoadAsset("T_MenuBg"))

    -- Title
    local title = self:CreateChild("Text")
    title:SetAnchorMode(AnchorMode.TopMid)
    title:SetPosition(0.0, 40.0)
    title:SetDimensions(400.0, 60.0)
    title:SetTextSize(48.0)
    title:SetText("My Game")
    title:SetHorizontalJustification(Justification.Center)
    title:SetColor(Vec(1, 1, 1, 1))

    -- Buttons
    local buttonNames = {"Play", "Options", "Quit"}
    self.buttons = {}

    for i, name in ipairs(buttonNames) do
        local btn = self:CreateChild("Button")
        btn:SetTextString(name)
        btn:SetAnchorMode(AnchorMode.Mid)
        btn:SetPosition(0.0, (i - 1) * 60.0 - 30.0)
        btn:SetDimensions(220.0, 45.0)
        btn:ConnectSignal("Activated", self, MainMenu.OnButtonClicked)
        self.buttons[i] = btn
    end

    -- Navigation
    for i, btn in ipairs(self.buttons) do
        local up = self.buttons[i - 1]
        local down = self.buttons[i + 1]
        btn:SetNavigation(up, down, nil, nil)
    end

    Button.SetSelected(self.buttons[1])
end

function MainMenu:OnButtonClicked()
    local selected = Button.GetSelected()
    if selected == self.buttons[1] then
        -- Play
    elseif selected == self.buttons[2] then
        -- Options
    elseif selected == self.buttons[3] then
        -- Quit
    end
end
```

---

## Further Reading

- [Widget System Overview](Overview.md)
- [Button Lua API Reference](../../Lua/Nodes/Widgets/Button.md)
- [Signal System](../../Lua/Misc/Signal.md)
- [Building Complete UIs](BuildingUI.md)
