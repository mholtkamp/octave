# Window Widget

The Window widget is a container that provides a standard window UI with title bar, close button, content area, and optional resize handle. It's designed as a base for creating dialogs, panels, and other windowed UI elements.

## Features

- **Title Bar**: Draggable header with customizable title text
- **Close Button**: Optional button to close/hide the window
- **Content Area**: ScrollContainer-based content that automatically handles overflow
- **Resize Handle**: Optional corner handle for resizing the window
- **WindowManager Integration**: Register windows by ID for programmatic control

## Basic Usage

1. Add a Window widget to your scene
2. Add child widgets - they are automatically placed in the content area
3. Set a Window Id to register with WindowManager for script access

## Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| Window Id | String | "" | Unique identifier for WindowManager registration |
| Title | String | "Window" | Text displayed in the title bar |
| Title Alignment | Enum | Center | Title text alignment: Left, Center, Right |
| Title Font Size | Float | 14.0 | Font size for the title text |
| Title Offset | Vector2 | (0, 0) | Offset for title text position |
| Title Bar Height | Float | 24.0 | Height of the title bar in pixels |
| Content Padding Left | Float | 0.0 | Left padding for content area |
| Content Padding Top | Float | 0.0 | Top padding for content area |
| Content Padding Right | Float | 0.0 | Right padding for content area |
| Content Padding Bottom | Float | 0.0 | Bottom padding for content area |
| Draggable | Bool | true | Allow dragging by title bar |
| Resizable | Bool | true | Show resize handle in bottom-right corner |
| Show Close Button | Bool | true | Show close button in title bar |
| Start Hidden | Bool | false | Hide window when play mode starts |
| Min Size | Vector2 | (100, 80) | Minimum window dimensions |
| Max Size | Vector2 | (0, 0) | Maximum dimensions (0 = unlimited) |
| Resize Handle Size | Float | 12.0 | Size of the resize handle |
| Title Bar Color | Color | (0.2, 0.2, 0.3, 1.0) | Title bar background color |
| Background Color | Color | (0.15, 0.15, 0.2, 0.95) | Window background color |
| Background Texture | Texture | null | Optional window background texture |
| Title Bar Texture | Texture | null | Optional title bar background texture |
| Close Button Texture | Texture | null | Optional close button texture |
| Close Button Normal Color | Color | (0.6, 0.2, 0.2, 1.0) | Close button normal state color |
| Close Button Hovered Color | Color | (0.8, 0.3, 0.3, 1.0) | Close button hovered state color |
| Close Button Pressed Color | Color | (0.5, 0.15, 0.15, 1.0) | Close button pressed state color |

## Content Widgets

Use `SetContentWidget()` or `GetContentContainer():AddChild()` to add content to the window:

```lua
-- Method 1: SetContentWidget (single content widget)
local content = Node.Create("ArrayWidget")
window:SetContentWidget(content)

-- Method 2: Access content container directly (multiple widgets)
local container = window:GetContentContainer()
container:AddChild(button1)
container:AddChild(button2)
```

The content is placed inside a ScrollContainer, so it scrolls if it exceeds the window size.

## Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| Show | self | Window is shown |
| Hide | self | Window is hidden |
| Close | self | Close button clicked or Close() called |
| DragStart | self | Drag started |
| DragEnd | self | Drag ended |
| ResizeStart | self | Resize started |
| ResizeEnd | self | Resize ended |
| Resized | self | Window size changed during resize |

## Script Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| OnShow | self | Called when window is shown |
| OnHide | self | Called when window is hidden |
| OnClose | self | Called when close is requested |
| OnDragStart | self | Called when drag starts |
| OnDragEnd | self | Called when drag ends |
| OnResizeStart | self | Called when resize starts |
| OnResizeEnd | self | Called when resize ends |
| OnResized | self | Called during resize |

## WindowManager

Windows with a Window Id are automatically registered with the global WindowManager when they start. This allows controlling windows from anywhere:

```lua
-- Show a window by ID
WindowManager.ShowWindow("inventory")

-- Hide a window by ID
WindowManager.HideWindow("settings")

-- Check if a window exists
if WindowManager.HasWindow("dialog") then
    -- ...
end

-- Get a window to modify it
local win = WindowManager.FindWindow("shop")
if win then
    win:SetTitle("Item Shop")
end

-- Bring a window to front
WindowManager.BringToFront("chat")
```

## Internal Widget Access

For advanced customization, you can access internal widgets:

- `GetTitleBar()` - Title bar Canvas
- `GetTitleText()` - Title Text widget
- `GetCloseButton()` - Close Button widget
- `GetContentContainer()` - ScrollContainer for content
- `GetResizeHandle()` - Resize handle Quad
- `GetBackground()` - Background Quad
