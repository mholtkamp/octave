# ToolTip System

The ToolTip system provides automatic tooltip display when hovering over UI widgets. It supports both static tooltips configured in the editor and dynamic tooltips updated via Lua callbacks.

## Architecture

```
ToolTipManager (Singleton)
    - Tracks hovered widgets via ContainsMouse()
    - Manages timing (show delay)
    - Owns/positions the ToolTipWidget
    - Fires Lua callbacks on show/hide

ToolTipWidget (Widget)
    - Background (Quad, transient child)
    - TitleText (Text, transient child)
    - DescriptionText (Text, transient child)
    - Auto-sizes based on content

Widget (base class additions)
    - mTooltipName (string)
    - mTooltipDescription (string)
```

## Widget Tooltip Properties

All widgets now have two tooltip properties that can be set in the editor or via Lua:

- **Tooltip Name**: The title/header text displayed in bold at the top
- **Tooltip Description**: The description text displayed below the title

### Setting Tooltips in the Editor

1. Select any widget in the hierarchy
2. In the Properties panel, find the "Tooltip" category
3. Set "Tooltip Name" and/or "Tooltip Description"

### Setting Tooltips in Lua

```lua
local button = canvas:FindChild("UpgradeButton")
button:SetTooltipName("Upgrade Weapon")
button:SetTooltipDescription("Increases damage by 10%.\nCost: 100 gold")

-- Check if widget has tooltip
if button:HasTooltip() then
    print("Button has tooltip: " .. button:GetTooltipName())
end
```

## ToolTipManager Configuration

The global `ToolTip` table provides configuration options:

```lua
-- Show delay (seconds before tooltip appears)
ToolTip.SetShowDelay(0.5)  -- Default: 0.5 seconds
local delay = ToolTip.GetShowDelay()

-- Cursor offset (position relative to cursor)
ToolTip.SetCursorOffset(16, 16)  -- Default: (16, 16)
local offset = ToolTip.GetCursorOffset()

-- Enable/disable tooltips globally
ToolTip.SetEnabled(true)
local enabled = ToolTip.IsEnabled()
```

## Dynamic Tooltips with Callbacks

For dynamic content, register callbacks that fire when tooltips show/hide:

```lua
-- OnShow callback receives the hovered widget
ToolTip.SetOnShowCallback(function(widget)
    local tooltip = ToolTip.GetWidget()

    -- Update tooltip based on widget
    if widget:GetName() == "InventorySlot" then
        local item = GetItemFromSlot(widget)
        tooltip:SetContent(item.name, item:GetDescription())
    end
end)

-- OnHide callback (optional)
ToolTip.SetOnHideCallback(function(widget)
    -- Clean up if needed
end)

-- Clear callbacks
ToolTip.SetOnShowCallback(nil)
ToolTip.SetOnHideCallback(nil)
```

## ToolTipWidget Customization

Access the ToolTipWidget for advanced styling:

```lua
local tooltip = ToolTip.GetWidget()

-- Background styling
tooltip:SetBackgroundColor(Vec4.New(0.1, 0.1, 0.15, 0.95))
tooltip:SetCornerRadius(4)

-- Text styling
tooltip:SetTitleFontSize(14)
tooltip:SetTextFontSize(12)
tooltip:SetTitleColor(Vec4.New(1, 1, 1, 1))
tooltip:SetTextColor(Vec4.New(0.8, 0.8, 0.8, 1))

-- Layout
tooltip:SetPadding(8, 6, 8, 6)  -- left, top, right, bottom
tooltip:SetMaxWidth(300)
tooltip:SetTitleTextSpacing(4)
```

## Manual Control

Show/hide tooltips programmatically:

```lua
-- Force show tooltip for a specific widget
ToolTip.Show(widget)

-- Hide tooltip immediately
ToolTip.Hide()

-- Check current state
local visible = ToolTip.IsVisible()
local hovered = ToolTip.GetHoveredWidget()
```

## Custom Tooltip Templates

You can register custom `ToolTipWidget` instances as templates. When a widget's `TooltipName` matches a registered template name, that template is used instead of the default tooltip.

### Registering Templates

```lua
function Start()
    -- Create and style custom tooltips in the scene, then register them
    local enemyTooltip = self:FindChild("EnemyTooltip")
    local buttonTooltip = self:FindChild("ButtonTooltip")
    local speechBubble = self:FindChild("SpeechBubble")

    -- Register templates by name
    ToolTip.RegisterTemplate("enemy", enemyTooltip)
    ToolTip.RegisterTemplate("button", buttonTooltip)
    ToolTip.RegisterTemplate("speech", speechBubble)
end
```

### Using Templates

Set the widget's `TooltipName` to match a registered template:

```lua
-- These widgets will use the "enemy" template
enemyHealthBar:SetTooltipName("enemy")
enemyPortrait:SetTooltipName("enemy")

-- This widget uses the "button" template
buyButton:SetTooltipName("button")

-- This uses the "speech" template
npcAvatar:SetTooltipName("speech")
```

For default tooltips (no template), set both name and description:
```lua
widget:SetTooltipName("Item Name")
widget:SetTooltipDescription("Item description here")
```

### Template API

| Method | Description |
|--------|-------------|
| `ToolTip.RegisterTemplate(name, widget)` | Register a ToolTipWidget as a template |
| `ToolTip.UnregisterTemplate(name)` | Unregister by name |
| `ToolTip.UnregisterTemplate(widget)` | Unregister by widget |
| `ToolTip.GetTemplate(name)` | Get registered template by name |
| `ToolTip.HasTemplate(name)` | Check if template is registered |

### Template Design Tips

1. **Create templates in the scene** - Design your custom tooltips as ToolTipWidget nodes in your UI scene
2. **Start hidden** - Templates should start invisible; the system shows/hides them automatically
3. **Use OnShowCallback for dynamic content** - Update template content based on the hovered widget:

```lua
ToolTip.SetOnShowCallback(function(widget)
    local templateName = widget:GetTooltipName()

    if templateName == "enemy" then
        local template = ToolTip.GetTemplate("enemy")
        local enemy = GetEnemyData(widget)
        template:SetTooltipTitle(enemy.name)
        template:SetTooltipText("HP: " .. enemy.hp .. "/" .. enemy.maxHp)
    end
end)
```

## API Reference

### Widget Tooltip Methods

| Method | Description |
|--------|-------------|
| `widget:SetTooltipName(name)` | Set tooltip title |
| `widget:GetTooltipName()` | Get tooltip title |
| `widget:SetTooltipDescription(desc)` | Set tooltip description |
| `widget:GetTooltipDescription()` | Get tooltip description |
| `widget:HasTooltip()` | Returns true if widget has tooltip content |

### ToolTip Global Table

| Method | Description |
|--------|-------------|
| `ToolTip.SetShowDelay(seconds)` | Set delay before showing (default: 0.5s) |
| `ToolTip.GetShowDelay()` | Get current show delay |
| `ToolTip.SetCursorOffset(x, y)` | Set tooltip offset from cursor |
| `ToolTip.GetCursorOffset()` | Get cursor offset as Vec2 |
| `ToolTip.SetEnabled(bool)` | Enable/disable tooltip system |
| `ToolTip.IsEnabled()` | Check if tooltips are enabled |
| `ToolTip.Show(widget)` | Force show tooltip for widget |
| `ToolTip.Hide()` | Force hide tooltip |
| `ToolTip.GetWidget()` | Get the ToolTipWidget instance |
| `ToolTip.IsVisible()` | Check if tooltip is currently visible |
| `ToolTip.GetHoveredWidget()` | Get currently hovered widget with tooltip |
| `ToolTip.SetOnShowCallback(func)` | Set callback for tooltip show |
| `ToolTip.SetOnHideCallback(func)` | Set callback for tooltip hide |
| `ToolTip.RegisterTemplate(name, widget)` | Register a custom tooltip template |
| `ToolTip.UnregisterTemplate(name/widget)` | Unregister a template |
| `ToolTip.GetTemplate(name)` | Get registered template by name |
| `ToolTip.HasTemplate(name)` | Check if template exists |

### ToolTipWidget Methods

| Method | Description |
|--------|-------------|
| `SetContent(name, desc)` | Set both title and description |
| `SetTooltipTitle(name)` | Set title text |
| `GetTooltipTitle()` | Get title text |
| `SetTooltipText(desc)` | Set description text |
| `GetTooltipText()` | Get description text |
| `SetBackgroundColor(vec4)` | Set background color |
| `SetCornerRadius(float)` | Set corner radius |
| `SetTitleFontSize(float)` | Set title font size |
| `SetTextFontSize(float)` | Set description font size |
| `SetTitleColor(vec4)` | Set title text color |
| `SetTextColor(vec4)` | Set description text color |
| `SetPadding(l, t, r, b)` | Set padding |
| `SetMaxWidth(float)` | Set maximum tooltip width |
| `SetTitleTextSpacing(float)` | Set spacing between title and text |
