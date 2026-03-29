# ListView System

The ListView system provides a dynamic, data-driven list widget that handles scrolling, item creation, and user interaction. It's designed for displaying collections of data items with Lua-driven customization.

## Architecture

```
ListViewWidget
├── ScrollContainer (transient, handles scrolling)
│   └── ArrayWidget (transient, handles layout)
│       ├── ListViewItemWidget 0
│       │   └── [Template Instance] (user content)
│       ├── ListViewItemWidget 1
│       │   └── [Template Instance]
│       └── ...
```

### Components

- **ListViewWidget**: Main container that manages data, selection, and item lifecycle
- **ListViewItemWidget**: Wrapper for each item that handles pointer events
- **ScrollContainer**: Provides scrolling with momentum (created automatically)
- **ArrayWidget**: Handles vertical/horizontal layout (created automatically)

## Editor Setup

1. Add a **ListViewWidget** to your scene
2. Configure properties in the editor:
   - **Item Template**: Scene asset to instantiate for each item
   - **Spacing**: Gap between items (pixels)
   - **Orientation**: Vertical or Horizontal
   - **Item Width/Height**: Fixed size (0 = auto-size to content)
3. Attach a Lua script to handle callbacks

## Item Template Creation

Create a Scene asset to use as your item template:

1. Create a new Scene
2. Add a root Widget (Canvas or any widget type)
3. Add child widgets for your item content (Text, Quad, Button, etc.)
4. Save the Scene asset
5. Assign it to `Item Template` on your ListViewWidget

## Lua Callbacks

Attach a script to ListViewWidget with these callback functions:

### OnItemGenerate(index, data, itemWidget)
Called when a new item is created. Use this to populate item content.

```lua
function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    local nameText = content:FindChild("NameText")
    local iconQuad = content:FindChild("Icon")

    nameText:SetText(data.name)
    iconQuad:SetTexture(data.icon)
end
```

### OnItemUpdate(index, data, itemWidget)
Called when `UpdateItem()` is called. Use this to refresh item content.

```lua
function OnItemUpdate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    local nameText = content:FindChild("NameText")
    nameText:SetText(data.name)
end
```

### OnItemClicked(index, data)
Called when an item is clicked/tapped.

```lua
function OnItemClicked(index, data)
    print("Clicked: " .. data.name)
end
```

### OnItemHoverEnter(index, data)
Called when the mouse enters an item.

### OnItemHoverExit(index, data)
Called when the mouse leaves an item.

### OnSelectionChanged(index, data)
Called when the selected item changes. `index` is -1 if selection was cleared.

```lua
function OnSelectionChanged(index, data)
    if index >= 0 then
        print("Selected: " .. data.name)
    else
        print("Selection cleared")
    end
end
```

## API Reference

### ListViewWidget

#### Template

| Method | Description |
|--------|-------------|
| `SetItemTemplate(scene)` | Set the Scene asset to use as item template |
| `GetItemTemplate()` | Get the current item template Scene |

#### Data Management

| Method | Description |
|--------|-------------|
| `SetData(table)` | Set the data array, rebuilds all items |
| `AddItem(data)` | Append an item to the end |
| `AddItem(data, index)` | Insert an item at index |
| `RemoveItem(index)` | Remove item at index |
| `UpdateItem(index, data)` | Update data at index (fires OnItemUpdate) |
| `Clear()` | Remove all items |
| `GetItemCount()` | Get total number of items |
| `GetItemData(index)` | Get data at index |

#### Layout

| Method | Description |
|--------|-------------|
| `SetSpacing(float)` | Set gap between items |
| `GetSpacing()` | Get current spacing |
| `SetOrientation("Vertical"/"Horizontal")` | Set layout direction |
| `GetOrientation()` | Get current orientation |
| `SetItemWidth(float)` | Set fixed item width (0 = auto) |
| `SetItemHeight(float)` | Set fixed item height (0 = auto) |
| `GetItemWidth()` | Get item width setting |
| `GetItemHeight()` | Get item height setting |

#### Selection

| Method | Description |
|--------|-------------|
| `SetSelectedIndex(index)` | Select item at index (-1 to clear) |
| `GetSelectedIndex()` | Get selected index (-1 if none) |
| `GetSelectedData()` | Get data of selected item |
| `ClearSelection()` | Clear selection |

#### Access

| Method | Description |
|--------|-------------|
| `GetItem(index)` | Get ListViewItemWidget at index |
| `ScrollToItem(index)` | Scroll to make item visible |
| `GetScrollContainer()` | Get internal ScrollContainer |
| `GetArrayWidget()` | Get internal ArrayWidget |

### ListViewItemWidget

| Method | Description |
|--------|-------------|
| `GetIndex()` | Get this item's index in the list |
| `GetListView()` | Get parent ListViewWidget |
| `GetContentWidget()` | Get the instantiated template root widget |
| `SetSelected(bool)` | Set selection state |
| `IsSelected()` | Check if selected |
| `IsHovered()` | Check if mouse is over item |

## Signals

### ListViewWidget Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| `ItemClicked` | index, data | Item was clicked |
| `ItemHoverEnter` | index, data | Mouse entered item |
| `ItemHoverExit` | index, data | Mouse left item |
| `SelectionChanged` | index, data | Selection changed |

### ListViewItemWidget Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| `Clicked` | item | Item was clicked |
| `HoverEnter` | item | Mouse entered |
| `HoverExit` | item | Mouse left |
| `Selected` | item | Item was selected |
| `Deselected` | item | Item was deselected |

## Editor Properties

### ListViewWidget Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| Item Template | Scene | null | Scene asset for item template |
| Spacing | Float | 0 | Gap between items |
| Orientation | Enum | Vertical | Layout direction |
| Item Width | Float | 0 | Fixed width (0 = auto) |
| Item Height | Float | 0 | Fixed height (0 = auto) |
