# XML/CSS UI System

Octave supports declarative UI authoring through XML layouts and CSS stylesheets. This system builds on the existing Widget hierarchy, letting you define UI trees in markup instead of (or alongside) code.

---

## Quick Start

**1. Create an XML file** (`UI/MainMenu.xml`):
```xml
<ui>
  <link href="UI/menu.css" />

  <div id="root" class="fullscreen">
    <img class="background" src="T_MenuBg" />

    <flex class="button-column">
      <button id="play-btn" class="primary-btn" on-click="OnPlayClicked">
        Play
      </button>
      <button id="settings-btn" class="primary-btn" on-click="OnSettingsClicked">
        Settings
      </button>
    </flex>

    <text class="version-label">v{{version}}</text>
  </div>
</ui>
```

**2. Create a CSS file** (`UI/menu.css`):
```css
.fullscreen {
  anchor: full-stretch;
}

.background {
  anchor: full-stretch;
}

.button-column {
  anchor: center;
  flex-direction: column;
  gap: 12px;
  width: 200px;
  height: 200px;
}

.primary-btn {
  width: 180px;
  height: 48px;
  background-color: #2a5db0;
  --hovered-color: #3a6dc0;
  --pressed-color: #1a4da0;
  color: #ffffff;
  font-size: 18px;
  text-align: center;
}

.version-label {
  anchor: bottom-right;
  color: #888888;
  font-size: 12px;
  left: -10px;
  top: -10px;
}
```

**3. Attach a Lua script to a Canvas widget in your scene:**
```lua
-- Script attached to a Canvas node in the scene
MainMenu = {}

function MainMenu:Start()
    self.ui = UI.Load("UI/MainMenu.xml")
    self.ui:Mount(self) -- mount onto this Canvas widget
    self.ui:SetData("version", "1.0.3")
end

function MainMenu:Tick(deltaTime)
    self.ui:Tick()
end

function MainMenu:Stop()
    self.ui:Unmount()
    self.ui:Destroy()
end

function OnPlayClicked()
    Engine.GetWorld():LoadScene("Levels/Level1.scn")
end

function OnSettingsClicked()
    -- open settings
end
```

---

## XML Elements

Each XML element maps to a Widget type in the engine:

| Element | Widget Type | Description |
|---------|-------------|-------------|
| `<div>` | Canvas | Invisible container for grouping |
| `<flex>` | ArrayWidget | Auto-layout container (row or column) |
| `<img>` | Quad | Textured rectangle |
| `<text>` | Text | Rendered text |
| `<p>` | Text | Alias for `<text>` |
| `<span>` | Text | Alias for `<text>` |
| `<button>` | Button | Interactive button (has built-in Quad + Text) |
| `<canvas>` | Canvas | Explicit canvas container |
| `<poly>` | Poly | Polygon / line shape |
| `<polyrect>` | PolyRect | Rectangular outline |
| `<include>` | *(special)* | Imports another XML file |
| `<link>` | *(special)* | Links an external CSS stylesheet |
| `<style>` | *(special)* | Inline CSS block |

You can also use PascalCase widget type names directly: `<Canvas>`, `<Quad>`, `<Text>`, `<Button>`, `<ArrayWidget>`.

### Special Elements

**`<ui>`** - Root element. All content goes inside this.

**`<link href="path.css" />`** - Loads and applies an external CSS file. Path is relative to the XML file.

**`<style>`** - Inline CSS:
```xml
<style>
  .my-class { color: #ff0000; font-size: 24px; }
</style>
```

**`<include src="other.xml" />`** - Includes another XML document as a subtree. Used for component composition:
```xml
<include src="UI/PlayerCard.xml" />
```

---

## XML Attributes

### Common Attributes

| Attribute | Description | Example |
|-----------|-------------|---------|
| `id` | Unique identifier (sets widget name, enables FindById) | `id="play-btn"` |
| `class` | Space-separated CSS class names | `class="primary-btn large"` |
| `style` | Inline CSS styles | `style="width: 100px; color: red"` |
| `name` | Widget name (alternative to id) | `name="MyWidget"` |
| `visible` | Show/hide | `visible="false"` |
| `active` | Enable/disable | `active="true"` |
| `width` | Widget width | `width="200"` or `width="50%"` |
| `height` | Widget height | `height="100"` |
| `anchor` | Anchor mode | `anchor="center"` |
| `opacity` | Opacity (0-1) | `opacity="0.5"` |
| `color` | Text/widget color | `color="#ff0000"` |

### Element-Specific Attributes

| Attribute | Applies To | Description |
|-----------|-----------|-------------|
| `src` | `<img>`, `<button>` | Texture asset name |
| `font` | `<text>`, `<button>` | Font asset name |
| `font-size` | `<text>`, `<button>` | Font size in pixels |
| `text` | `<text>`, `<button>` | Text content (alternative to inner text) |
| `word-wrap` | `<text>` | Enable word wrap (`true`/`false`) |
| `direction` | `<flex>` | Layout direction (`row`/`column`) |
| `spacing` | `<flex>` | Gap between children (pixels) |

### Event Attributes

| Attribute | Description |
|-----------|-------------|
| `on-click` | Lua function called when button is clicked |
| `on-hover` | Lua function called on hover enter |
| `on-press` | Lua function called on press down |
| `on-release` | Lua function called on release |

```xml
<button id="my-btn" on-click="HandleMyButtonClick">Click Me</button>
```

---

## CSS Reference

### Selectors

| Selector | Specificity | Example |
|----------|-------------|---------|
| Element | 1 | `button { ... }` |
| `.class` | 10 | `.primary-btn { ... }` |
| `#id` | 100 | `#play-btn { ... }` |

Higher specificity wins. Inline `style=""` attribute always wins.

### Layout Properties

| Property | Values | Maps To |
|----------|--------|---------|
| `width` | `Npx`, `N%` | `SetWidth()` / `SetWidthRatio()` |
| `height` | `Npx`, `N%` | `SetHeight()` / `SetHeightRatio()` |
| `left` | `Npx`, `N%` | `SetX()` / `SetXRatio()` |
| `top` | `Npx`, `N%` | `SetY()` / `SetYRatio()` |
| `right` | `Npx` | `SetRightMargin()` |
| `bottom` | `Npx` | `SetBottomMargin()` |
| `margin` | `Npx` | `SetMargins()` (all sides) |
| `margin-left/top/right/bottom` | `Npx` | Individual margins |
| `anchor` | See below | `SetAnchorMode()` |
| `display` | `none`, `flex` | Visibility / ArrayWidget |
| `overflow` | `hidden` | `EnableScissor(true)` |

### Anchor Values

| CSS Value | AnchorMode |
|-----------|-----------|
| `top-left` | TopLeft |
| `top-center` | TopMid |
| `top-right` | TopRight |
| `center-left` | MidLeft |
| `center` | Mid |
| `center-right` | MidRight |
| `bottom-left` | BottomLeft |
| `bottom-center` | BottomMid |
| `bottom-right` | BottomRight |
| `top-stretch` | TopStretch |
| `bottom-stretch` | BottomStretch |
| `left-stretch` | LeftStretch |
| `right-stretch` | RightStretch |
| `center-h-stretch` | MidHorizontalStretch |
| `center-v-stretch` | MidVerticalStretch |
| `full-stretch` | FullStretch |

### Visual Properties

| Property | Values | Maps To |
|----------|--------|---------|
| `color` | `#hex`, `rgba()`, named | Text color (or widget color) |
| `background-color` | `#hex`, `rgba()`, named | Widget/Quad/Button color |
| `background-image` | `url("AssetName")` | Texture on Quad/Button |
| `opacity` | `0.0`-`1.0` | `SetOpacityFloat()` |

### Text Properties

| Property | Values | Maps To |
|----------|--------|---------|
| `font-family` | Asset name | `SetFont()` |
| `font-size` | `Npx` | `SetTextSize()` |
| `text-align` | `left`, `center`, `right` | `SetHorizontalJustification()` |
| `vertical-align` | `top`, `center`, `bottom` | `SetVerticalJustification()` |
| `word-wrap` | `break-word` | `EnableWordWrap(true)` |

### Transform Properties

| Property | Values | Maps To |
|----------|--------|---------|
| `rotation` | `N` (degrees) | `SetRotation()` |
| `scale` | `N` | `SetScale(N, N)` |
| `transform-origin` | `N` | `SetPivot()` |

### Flex Container Properties

| Property | Values | Maps To |
|----------|--------|---------|
| `flex-direction` | `row`, `column` | ArrayWidget orientation |
| `gap` | `Npx` | ArrayWidget spacing |
| `align-items` | `center` | `SetCentered(true)` |

### Button State Properties (Custom)

| Property | Description |
|----------|-------------|
| `--normal-color` | Color in Normal state |
| `--hovered-color` | Color in Hovered state |
| `--pressed-color` | Color in Pressed state |
| `--locked-color` | Color in Locked state |
| `--normal-texture` | Texture in Normal state |
| `--hovered-texture` | Texture in Hovered state |
| `--pressed-texture` | Texture in Pressed state |
| `--locked-texture` | Texture in Locked state |

### Color Formats

```css
color: #RGB;              /* Short hex */
color: #RRGGBB;           /* Full hex */
color: #RRGGBBAA;         /* Hex with alpha */
color: rgb(255, 128, 0);  /* RGB 0-255 */
color: rgba(255, 128, 0, 0.5);  /* RGBA, alpha 0-1 */
color: white;             /* Named: white, black, red, green, blue, yellow, gray, transparent */
```

---

## Lua API

### Module Functions

```lua
-- Load a UI document from an XML file
local doc = UI.Load("UI/MainMenu.xml")
```

### Instance Methods

```lua
-- Build the widget tree (called automatically by Mount if needed)
local rootWidget = doc:Instantiate()

-- Attach to a parent widget
doc:Mount(parentWidget)

-- Detach from parent
doc:Unmount()

-- Query widgets by id or class
local btn = doc:FindById("play-btn")       -- returns Widget or nil
local items = doc:FindByClass("list-item") -- returns table of Widgets

-- Get the root Canvas widget
local root = doc:GetRootWidget()

-- Data binding: replaces {{key}} placeholders in text content
doc:SetData("score", "100")
doc:SetData("playerName", "Alice")

-- Register event callbacks (by function name or closure)
doc:SetCallback("play-btn", "click", "OnPlayClicked")
doc:SetCallback("quit-btn", "click", function(btn)
    QuitGame()
end)

-- Process events (call each frame)
doc:Tick()

-- Explicit cleanup
doc:Destroy()
```

### Data Binding

Use `{{variableName}}` in text content. Call `SetData()` to update values.

```xml
<text>Score: {{score}} / {{maxScore}}</text>
<text>Welcome, {{playerName}}!</text>
```

```lua
doc:SetData("score", tostring(currentScore))
doc:SetData("maxScore", "999")
doc:SetData("playerName", playerName)
```

---

## Component Composition

Use `<include>` to build reusable components:

**`UI/PlayerCard.xml`:**
```xml
<ui>
  <div class="player-card">
    <img class="avatar" src="T_DefaultAvatar" />
    <text class="player-name">{{name}}</text>
    <text class="player-score">Score: {{score}}</text>
  </div>
</ui>
```

**`UI/HUD.xml`:**
```xml
<ui>
  <link href="UI/hud.css" />

  <div id="hud" class="fullscreen">
    <include src="UI/PlayerCard.xml" />
    <text id="timer" class="timer-text">{{time}}</text>
  </div>
</ui>
```

---

## Platform Notes

- **3DS**: The XML and CSS parsers use minimal memory. IrrXML is a forward-only reader. The CSS parser is a hand-written tokenizer with no regex. All widget rendering uses existing platform backends.
- **All Platforms**: No new rendering code. XML/CSS maps directly onto the existing Widget hierarchy.

---

## File Extensions

| Extension | Description |
|-----------|-------------|
| `.xml` | UI layout file (imported as UIDocument asset) |
| `.css` | CSS stylesheet (loaded via `<link>` in XML) |

---

## Editor Workflow

### Creating a UI Document
Right-click in the Assets panel → **Create Asset** → **UI Document**. Enter a name (e.g. `UI_HUD`). This creates both a `.oct` asset and a `.xml` source file in the same directory.

### Creating a CSS Stylesheet
Right-click in the Assets panel → **Create Asset** → **CSS Stylesheet**. Enter a name (e.g. `style`). Creates a `.css` file in the current directory.

### Editing a UIDocument
- **Double-click** the UIDocument asset in the Assets panel.
- **Right-click** → **Edit** in the Assets panel context menu.
- Select the asset, then click the **Edit** button in the Properties panel.

All three methods open the `.xml` source file in either the external editor (configured in **Preferences → External → Editors**) or the built-in Script Editor (if "Use Internal Editor" is enabled).

### Editing CSS
CSS files are plain text files on disk. Open them with your external editor or the built-in Script Editor. Changes take effect on the next `Instantiate()` call since CSS is re-read from disk each time.

### Save Sync
When editing a `.xml` file in the **built-in Script Editor**, saving automatically reimports the corresponding UIDocument `.oct` asset. No manual reimport is needed.

---

## Refreshing UI Assets (Ctrl+R)

Pressing **Ctrl+R** syncs all UIDocument `.xml` ↔ `.oct` files:

- If the `.xml` file is **newer** than the `.oct` → the XML is reimported into the asset (forward sync).
- If the `.oct` file is **newer** and its content differs from the `.xml` → the asset's XML is written back to the `.xml` file (reverse sync).
- **CSS files** are always re-read from disk on `Instantiate()`, so external edits take effect on the next UI load without any special refresh step.

This runs automatically as part of the existing Ctrl+R refresh (after script reload and asset directory rescan).

---

## XSD Schema (XML IntelliSense)

An XSD schema is generated for editor autocomplete, validation, and documentation of `.xml` UI files. Works with VS Code (Red Hat XML extension), IntelliJ, and other XML editors.

**Generated file:** `Engine/Generated/XML/OctaveUIDocument.xsd`

**Regenerate:**
```bash
python Tools/generate_ui_xsd.py [--verbose]
```

### VS Code Setup

Add to `.vscode/settings.json`:
```json
{
  "xml.fileAssociations": [
    { "pattern": "**/*.xml", "systemId": "Engine/Generated/XML/OctaveUIDocument.xsd" }
  ]
}
```

Or add inline per-file:
```xml
<ui xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:noNamespaceSchemaLocation="../../Engine/Generated/XML/OctaveUIDocument.xsd">
```

The schema encodes all elements, attributes (common, event, element-specific), and enum values (anchor modes, boolean, flex direction, word wrap). Update the XSD by editing the Python data structures in `Tools/generate_ui_xsd.py` and re-running the script.

---

## Architecture

```
XML File ──► UILoader ──► Widget Tree
               │
CSS File ──► UIStyleSheet ──► Style Rules ──► ApplyCSSProperty()
               │
Lua ──► UIDocument ──► Mount/Query/Bind/Events
```

**Key source files:**
- `Engine/Source/Engine/UI/UIDocument.h/.cpp` - Asset type
- `Engine/Source/Engine/UI/UILoader.h/.cpp` - XML parser
- `Engine/Source/Engine/UI/UIStyleSheet.h/.cpp` - CSS parser
- `Engine/Source/Engine/UI/UITypes.h/.cpp` - Value parsing, property mapping
- `Engine/Source/LuaBindings/UIDocument_Lua.h/.cpp` - Lua bindings
- `Tools/generate_ui_xsd.py` - XSD schema generator
- `Engine/Generated/XML/OctaveUIDocument.xsd` - Generated XSD schema
