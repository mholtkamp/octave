# DialogWindow Widget

The DialogWindow widget extends Window with a button bar at the bottom containing Confirm and Reject buttons. It's designed for confirmation dialogs, prompts, and any UI requiring explicit user decisions.

## Features

- **All Window Features**: Title bar, close button, draggable, resizable
- **Button Bar**: Customizable footer with confirm/reject buttons
- **Flexible Alignment**: Left, Center, Right, or Spread button positioning
- **Full Theming**: Colors, textures, and dimensions for both buttons

## Basic Usage

1. Add a DialogWindow widget to your scene
2. Set title and configure button text
3. Add content widgets as children
4. Implement OnConfirm/OnReject callbacks in attached script

## Properties

### Confirm Button

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| Confirm Text | String | "Confirm" | Confirm button label |
| Confirm Normal Color | Color | (0.2, 0.5, 0.2, 1.0) | Button normal state color |
| Confirm Hovered Color | Color | (0.3, 0.6, 0.3, 1.0) | Button hovered state color |
| Confirm Pressed Color | Color | (0.15, 0.4, 0.15, 1.0) | Button pressed state color |
| Confirm Texture | Texture | null | Optional button texture |
| Show Confirm Button | Bool | true | Show/hide confirm button |

### Reject Button

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| Reject Text | String | "Cancel" | Reject button label |
| Reject Normal Color | Color | (0.5, 0.2, 0.2, 1.0) | Button normal state color |
| Reject Hovered Color | Color | (0.6, 0.3, 0.3, 1.0) | Button hovered state color |
| Reject Pressed Color | Color | (0.4, 0.15, 0.15, 1.0) | Button pressed state color |
| Reject Texture | Texture | null | Optional button texture |
| Show Reject Button | Bool | true | Show/hide reject button |

### Button Bar Layout

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| Button Bar Height | Float | 40.0 | Height of the button bar |
| Button Spacing | Float | 10.0 | Space between buttons |
| Button Width | Float | 80.0 | Width of each button |
| Button Bar Padding | Float | 8.0 | Padding inside button bar |
| Button Bar Alignment | Enum | Right | Button positioning: Left, Center, Right, Spread |
| Button Bar Color | Color | (0.18, 0.18, 0.25, 1.0) | Button bar background color |

## Button Bar Alignments

- **Left**: Buttons aligned to left edge
- **Center**: Buttons centered in bar
- **Right**: Buttons aligned to right edge (default)
- **Spread**: Buttons spread across full width

## Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| Confirm | self | Confirm button clicked |
| Reject | self | Reject button clicked |

*Inherits all signals from Window: Show, Hide, Close, DragStart, DragEnd, ResizeStart, ResizeEnd, Resized*

## Script Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| OnConfirm | self | Called when confirm is triggered |
| OnReject | self | Called when reject is triggered |

*Inherits all callbacks from Window*

## Internal Widget Access

For advanced customization:

- `GetButtonBar()` - Button bar Canvas
- `GetConfirmButton()` - Confirm Button widget
- `GetRejectButton()` - Reject Button widget

*Plus all Window internal widgets: GetTitleBar(), GetTitleText(), GetCloseButton(), etc.*

## Common Patterns

### Yes/No Dialog
```lua
dialog:SetConfirmText("Yes")
dialog:SetRejectText("No")
dialog:SetButtonBarAlignment(2) -- Right
```

### Single Button (OK only)
```lua
dialog:SetConfirmText("OK")
dialog:SetShowRejectButton(false)
dialog:SetButtonBarAlignment(1) -- Center
```

### Destructive Action
```lua
dialog:SetConfirmText("Delete")
dialog:SetConfirmNormalColor(Vector.New(0.8, 0.2, 0.2, 1.0))
dialog:SetRejectText("Cancel")
```

### Custom Button Sizing
```lua
dialog:SetButtonWidth(120)
dialog:SetButtonBarHeight(50)
dialog:SetButtonBarPadding(12)
```
