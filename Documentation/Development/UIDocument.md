# UIDocument — Adding UI to Scenes

UIDocument is an Asset that turns an XML layout file into a live widget tree you can mount into any scene. This guide covers how to create, display, and interact with UIDocuments from Lua scripts and NodeGraphs.

> For the full XML element/CSS property reference, see [UI/XMLUISystem.md](UI/XMLUISystem.md).
> For standalone examples, see [UI/Examples.md](UI/Examples.md).

---

## How It Works

A UIDocument does **not** render on its own. It builds a Widget tree from XML and you **mount** that tree as a child of an existing Widget in the scene. The rendering happens automatically once the widgets are part of the scene's node hierarchy.

```
XML File  →  UIDocument.Instantiate()  →  Widget tree
                                              ↓
                              Mount(parentWidget) → attached to scene → rendered
```

---

## Step-by-Step: Getting UI On Screen

### 1. Create the XML File

Create a `.xml` file in your project's asset directory (e.g. `UI/HUD.xml`):

```xml
<ui>
  <style>
    .root { anchor: full-stretch; }
    .label {
      anchor: top-left;
      color: #ffffff;
      font-size: 24px;
      left: 16px;
      top: 16px;
    }
  </style>

  <div id="root" class="root">
    <text id="score" class="label">Score: {{score}}</text>
  </div>
</ui>
```

### 2. Import as an Asset

In the editor: right-click the Assets panel → **Create Asset** → **UI Document**. Point it at your `.xml` file, or just place the `.xml` in your asset directory and let the engine import it.

### 3. Create a Canvas Node in the Scene

UIDocuments mount onto **Widget** nodes. Add a **Canvas** node to your scene:

1. In the Scene panel, right-click → **Add Node** → **Widget** → **Canvas**
2. Name it something like `UIRoot`
3. Set its anchor to **FullStretch** so it covers the screen

### 4. Assign the UIDocument (Properties Panel — No Code Required)

Select the Canvas node. In the **Properties** panel, under the **Canvas** category, you'll see a **UI Document** asset slot. Click it and select your UIDocument asset. When the scene plays, the Canvas will automatically instantiate, mount, and tick the UIDocument — **no script needed**.

This is the simplest way to get a UI on screen. For dynamic behavior (data binding, callbacks), add a Lua script.

### 5. (Optional) Attach a Lua Script for Interactivity

For data binding, event callbacks, or runtime widget manipulation, add a **Script** component to the Canvas node.

**`Scripts/HUD.lua`:**
```lua
HUD = {}

function HUD:Start()
    -- Load the XML file
    self.ui = UI.Load("UI/HUD.xml")

    -- Mount onto self (this Canvas widget)
    self.ui:Mount(self)

    -- Set initial data
    self.ui:SetData("score", "0")
end

function HUD:Tick(deltaTime)
    -- Process button events each frame
    self.ui:Tick()
end

function HUD:Stop()
    self.ui:Unmount()
    self.ui:Destroy()
end
```

Attach this script to the Canvas node in the editor. When the scene plays, the UI will appear.

---

## Mounting Options

The `Mount(parentWidget)` call attaches the UIDocument's widget tree as a child of the given Widget. There are several ways to get a parent widget:

### Option A: Script on a Canvas (Recommended)

Attach your script directly to a Canvas widget in the scene. Use `self` as the mount target:

```lua
function MyUI:Start()
    self.ui = UI.Load("UI/Menu.xml")
    self.ui:Mount(self)
end
```

### Option B: Find a Widget by Name

If the Canvas is elsewhere in the scene, find it by name:

```lua
function MyUI:Start()
    local world = Engine.GetWorld()
    local canvas = world:FindNode("UIRoot")

    self.ui = UI.Load("UI/Menu.xml")
    self.ui:Mount(canvas)
end
```

### Option C: Spawn a Canvas at Runtime

Create a Canvas dynamically and add it to the scene:

```lua
function MyUI:Start()
    local world = Engine.GetWorld()
    local canvas = world:SpawnNode("Canvas")
    canvas:SetName("DynamicUI")
    canvas:SetAnchorMode(AnchorMode.FullStretch)

    self.ui = UI.Load("UI/Menu.xml")
    self.ui:Mount(canvas)
end
```

---

## Full Lua API Reference

### Loading

```lua
-- Load a UIDocument from an XML file path
local doc = UI.Load("UI/MainMenu.xml")
```

### Lifecycle

```lua
-- Build the widget tree from XML (called automatically by Mount if needed)
local rootWidget = doc:Instantiate()

-- Attach the widget tree to a parent Widget
doc:Mount(parentWidget)

-- Detach from parent (widgets still exist, can re-mount later)
doc:Unmount()

-- Clean up completely (unmounts + destroys widget tree)
doc:Destroy()
```

### Querying Widgets

```lua
-- Find a widget by its id attribute (returns Widget or nil)
local btn = doc:FindById("play-btn")

-- Find all widgets with a given class (returns table of Widgets)
local items = doc:FindByClass("list-item")

-- Get the root Canvas of the document
local root = doc:GetRootWidget()
```

### Data Binding

Use `{{variableName}}` placeholders in XML text content:

```xml
<text>Score: {{score}} / {{maxScore}}</text>
<text>Welcome, {{playerName}}!</text>
```

Update them from Lua:

```lua
doc:SetData("score", tostring(currentScore))
doc:SetData("maxScore", "999")
doc:SetData("playerName", "Alice")
```

Every call to `SetData` immediately updates all text widgets containing that placeholder.

### Event Callbacks

#### In XML (declarative)

```xml
<button id="play-btn" on-click="OnPlayClicked">Play</button>
```

The `on-click` value names a **global Lua function**:

```lua
function OnPlayClicked()
    Engine.GetWorld():LoadScene("Levels/Level1.scn")
end
```

#### In Lua (programmatic)

Register callbacks by element ID and event name. You can pass a global function name or a closure:

```lua
-- By function name
doc:SetCallback("play-btn", "click", "OnPlayClicked")

-- By closure (preferred for local state)
doc:SetCallback("quit-btn", "click", function()
    Engine.Quit()
end)
```

#### Supported Events

| Event Name | Fires When |
|-----------|------------|
| `click` | Button was pressed then released |

> `hover`, `press`, and `release` events are parsed from XML but not yet dispatched in the current version.

#### Processing Events

You **must** call `doc:Tick()` every frame for button click detection to work:

```lua
function MyUI:Tick(deltaTime)
    self.ui:Tick()
end
```

### Manipulating Widgets at Runtime

After mounting, you can modify widgets directly through the Widget API:

```lua
-- Resize a health bar
local healthFill = doc:FindById("health-fill")
healthFill:SetWidthRatio(currentHP / maxHP)

-- Change text color
local label = doc:FindById("warning-label")
label:SetColor(1.0, 0.0, 0.0, 1.0)

-- Show/hide elements
local panel = doc:FindById("inventory-panel")
panel:SetVisible(false)

-- Change opacity
local overlay = doc:FindById("fade-overlay")
overlay:SetOpacityFloat(0.5)
```

---

## Using NodeGraph (Visual Scripting)

All UIDocument operations are available as NodeGraph nodes under the **UI** category:

| Node | Type | Pins |
|------|------|------|
| **Load UI Document** | Flow | In: Exec, Path(String) → Out: Exec, Document(Asset) |
| **Instantiate UI** | Flow | In: Exec, Document(Asset) → Out: Exec, Root(Widget) |
| **Mount UI** | Flow | In: Exec, Document(Asset), Parent(Widget) → Out: Exec |
| **Unmount UI** | Flow | In: Exec, Document(Asset) → Out: Exec |
| **Set UI Data** | Flow | In: Exec, Document(Asset), Key(String), Value(String) → Out: Exec |
| **UI Tick** | Flow | In: Exec, Document(Asset) → Out: Exec |
| **UI Find By Id** | Pure | In: Document(Asset), Id(String) → Out: Widget |
| **UI Get Root Widget** | Pure | In: Document(Asset) → Out: Root(Widget) |

### Example NodeGraph Flow

```
[Start Event]
    → [Load UI Document] (Path: "UI/HUD.xml")
        → [Mount UI] (Document: ←, Parent: Self)
            → (done)

[Tick Event]
    → [UI Tick] (Document: variable)
        → [Set UI Data] (Document: variable, Key: "score", Value: ←from game logic)
            → (done)
```

1. On **Start**, use `Load UI Document` with the XML path
2. Feed the Document output into `Mount UI`, with the parent set to a Canvas widget (use `Self` or `Get Node` to reference a Canvas in the scene)
3. On **Tick**, wire `UI Tick` to process button events
4. Use `Set UI Data` to update `{{}}` bindings

---

## Complete Examples

### Example 1: HUD with Health Bar and Score

**`UI/HUD.xml`:**
```xml
<ui>
  <style>
    .hud { anchor: full-stretch; }
    .health-bg {
      anchor: top-left;
      width: 200px; height: 24px;
      left: 16px; top: 16px;
      background-color: #333333;
    }
    .health-fill {
      anchor: left-stretch;
      background-color: #22cc22;
    }
    .score {
      anchor: top-right;
      color: #ffffff;
      font-size: 20px;
      left: -16px; top: 16px;
    }
  </style>

  <div class="hud">
    <div class="health-bg">
      <img id="health-fill" class="health-fill" />
    </div>
    <text id="score" class="score">Score: {{score}}</text>
  </div>
</ui>
```

**`Scripts/HUD.lua`:**
```lua
HUD = {}

local playerScore = 0
local currentHP = 100
local maxHP = 100

function HUD:Start()
    self.ui = UI.Load("UI/HUD.xml")
    self.ui:Mount(self)
    self.ui:SetData("score", "0")
end

function HUD:Tick(deltaTime)
    self.ui:Tick()

    -- Update health bar fill width
    local fill = self.ui:FindById("health-fill")
    if fill then
        fill:SetWidthRatio(currentHP / maxHP)
    end

    -- Update score display
    self.ui:SetData("score", tostring(playerScore))
end

function HUD:Stop()
    self.ui:Unmount()
    self.ui:Destroy()
end
```

### Example 2: Main Menu with Button Callbacks

**`UI/MainMenu.xml`:**
```xml
<ui>
  <link href="UI/theme.css" />

  <div id="menu" class="fullscreen">
    <img class="bg" src="T_MenuBg" />
    <text class="title">My Game</text>

    <flex class="button-column">
      <button id="btn-play" class="menu-btn" on-click="OnPlay">Play</button>
      <button id="btn-options" class="menu-btn" on-click="OnOptions">Options</button>
      <button id="btn-quit" class="menu-btn quit-btn" on-click="OnQuit">Quit</button>
    </flex>

    <text class="version">v{{version}}</text>
  </div>
</ui>
```

**`Scripts/MainMenu.lua`:**
```lua
MainMenu = {}

function MainMenu:Start()
    self.ui = UI.Load("UI/MainMenu.xml")
    self.ui:Mount(self)
    self.ui:SetData("version", "0.1.0")

    -- Override XML callbacks with closures for local state access
    self.ui:SetCallback("btn-play", "click", function()
        Engine.GetWorld():LoadScene("Levels/Level1.scn")
    end)

    self.ui:SetCallback("btn-quit", "click", function()
        Engine.Quit()
    end)
end

function MainMenu:Tick(deltaTime)
    self.ui:Tick()
end

function MainMenu:Stop()
    self.ui:Unmount()
    self.ui:Destroy()
end
```

### Example 3: Dialog Box (Runtime Creation)

**`Scripts/DialogHelper.lua`:**
```lua
DialogHelper = {}

-- Show a modal dialog, returns the UIDocument handle
function DialogHelper.Show(parentWidget, title, message, onOK, onCancel)
    local dialog = UI.Load("UI/Dialog.xml")
    dialog:Mount(parentWidget)
    dialog:SetData("title", title)
    dialog:SetData("message", message)

    dialog:SetCallback("btn-ok", "click", function()
        dialog:Unmount()
        dialog:Destroy()
        if onOK then onOK() end
    end)

    dialog:SetCallback("btn-cancel", "click", function()
        dialog:Unmount()
        dialog:Destroy()
        if onCancel then onCancel() end
    end)

    return dialog
end
```

Usage from another script:

```lua
function GameOver:Start()
    self.dialog = DialogHelper.Show(
        self,
        "Game Over",
        "You scored " .. tostring(score) .. " points. Play again?",
        function()
            Engine.GetWorld():LoadScene("Levels/Level1.scn")
        end,
        function()
            Engine.GetWorld():LoadScene("UI/MainMenu.scn")
        end
    )
end

function GameOver:Tick(deltaTime)
    if self.dialog then
        self.dialog:Tick()
    end
end
```

### Example 4: Multiple UI Documents on One Canvas

You can mount multiple UIDocuments to the same parent. They layer in mount order:

```lua
MultiUI = {}

function MultiUI:Start()
    -- HUD is always visible (bottom layer)
    self.hud = UI.Load("UI/HUD.xml")
    self.hud:Mount(self)

    -- Inventory starts hidden
    self.inventory = UI.Load("UI/Inventory.xml")
    self.inventoryVisible = false
end

function MultiUI:Tick(deltaTime)
    self.hud:Tick()

    -- Toggle inventory with I key
    if Input.IsKeyJustDown(Key.I) then
        if self.inventoryVisible then
            self.inventory:Unmount()
        else
            self.inventory:Mount(self)
        end
        self.inventoryVisible = not self.inventoryVisible
    end

    if self.inventoryVisible then
        self.inventory:Tick()
    end
end

function MultiUI:Stop()
    self.hud:Unmount()
    self.hud:Destroy()
    if self.inventoryVisible then
        self.inventory:Unmount()
    end
    self.inventory:Destroy()
end
```

### Example 5: Dynamic Data Updates (Real-Time HUD)

```lua
PlayerHUD = {}

function PlayerHUD:Start()
    self.ui = UI.Load("UI/PlayerHUD.xml")
    self.ui:Mount(self)

    self.displayHP = 100
    self.targetHP = 100
end

function PlayerHUD:Tick(deltaTime)
    self.ui:Tick()

    -- Smooth health bar animation
    self.displayHP = Maths.Damp(self.displayHP, self.targetHP, 0.1, deltaTime)

    local fill = self.ui:FindById("health-fill")
    if fill then
        local pct = self.displayHP / 100.0
        fill:SetWidthRatio(pct)

        -- Color shifts green → red as health drops
        local r = Maths.Lerp(0.13, 0.8, 1.0 - pct)
        local g = Maths.Lerp(0.8, 0.13, 1.0 - pct)
        fill:SetColor(r, g, 0.13, 1.0)
    end

    -- Update text displays
    self.ui:SetData("hp", tostring(math.floor(self.displayHP)))
    self.ui:SetData("ammo", tostring(self.ammo or 0))
end

function PlayerHUD:TakeDamage(amount)
    self.targetHP = math.max(self.targetHP - amount, 0)
end
```

---

## CSS Styling Quick Reference

Create a `.css` file and link it from XML with `<link href="path.css" />`.

```css
/* Selectors: element (1), .class (10), #id (100) — higher specificity wins */
.fullscreen { anchor: full-stretch; }

.menu-btn {
  width: 200px;
  height: 48px;
  background-color: #2a5db0;
  --hovered-color: #3a6dc0;
  --pressed-color: #1a4da0;
  color: #ffffff;
  font-size: 18px;
  text-align: center;
}

#special-btn {
  background-color: #b02a2a;
}
```

### Common Properties

| Property | Example | Notes |
|----------|---------|-------|
| `width` / `height` | `200px`, `50%` | Pixels or percent of parent |
| `anchor` | `center`, `full-stretch`, `top-left` | 16 anchor modes |
| `color` | `#fff`, `rgba(255,0,0,0.5)` | Text color |
| `background-color` | `#222`, `transparent` | Widget / button normal color |
| `font-size` | `24px` | Text size |
| `text-align` | `left`, `center`, `right` | Horizontal text alignment |
| `flex-direction` | `row`, `column` | ArrayWidget / `<flex>` direction |
| `gap` | `8px` | Spacing between flex children |
| `opacity` | `0.5` | 0–1 transparency |
| `overflow` | `hidden` | Enable scissor clipping |

### Button State Colors

```css
.my-btn {
  background-color: #334466;   /* Normal state */
  --hovered-color: #445588;
  --pressed-color: #223355;
  --locked-color: #222222;
}
```

---

## Component Composition with `<include>`

Split complex UIs into reusable components:

**`UI/Components/StatBar.xml`:**
```xml
<ui>
  <div class="stat-bar">
    <text class="stat-label">{{label}}</text>
    <div class="stat-bg">
      <img id="stat-fill" class="stat-fill" />
    </div>
  </div>
</ui>
```

**`UI/CharacterSheet.xml`:**
```xml
<ui>
  <link href="UI/rpg.css" />
  <div class="sheet">
    <text class="char-name">{{name}}</text>
    <include src="UI/Components/StatBar.xml" />
  </div>
</ui>
```

---

## Editor Workflow Summary

| Action | How |
|--------|-----|
| Create UIDocument | Assets panel → right-click → **Create Asset** → **UI Document** |
| Create CSS file | Assets panel → right-click → **Create Asset** → **CSS Stylesheet** |
| Edit XML | Double-click the UIDocument asset, or right-click → **Edit** |
| Edit CSS | Open `.css` file in Script Editor or external editor |
| Refresh after edit | **Ctrl+R** syncs `.xml` ↔ `.oct` files automatically |
| Add UI to scene | Add a **Canvas** node → set the **UI Document** property in the Properties panel |
| Add UI with scripting | Add a **Canvas** node → attach a Lua script that calls `UI.Load()` + `Mount(self)` |

---

## Troubleshooting

**UI doesn't appear:**
- Make sure `Mount()` is called with a valid Widget (Canvas) that is in the scene
- Check that the Canvas has `anchor: full-stretch` or appropriate size
- Verify `UI.Load()` path is correct (check the Debug Log for errors)

**Buttons don't respond:**
- Call `doc:Tick()` every frame in your `Tick()` function
- Ensure button elements have an `id` attribute
- Check that callbacks are registered with the correct element ID

**Data bindings don't update:**
- Placeholders must use double braces: `{{variableName}}`
- `SetData()` values must be strings — use `tostring()` for numbers
- Verify the placeholder name matches exactly (case-sensitive)

**CSS not applied:**
- Check `<link href="..." />` path is relative to the XML file location
- Verify CSS syntax (no trailing commas, proper `{ }` blocks)
- Higher specificity selectors override lower ones: `#id` > `.class` > `element`
