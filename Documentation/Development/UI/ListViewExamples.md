# ListView Examples

## Basic Inventory List

### Item Template Setup
Create a Scene with this structure:
```
Canvas (root, 200x40)
├── Background (Quad, full stretch)
├── Icon (Quad, 32x32, anchored left)
└── NameText (Text, anchored left with offset)
```

### Lua Script

```lua
-- Inventory.lua attached to ListViewWidget

local inventoryData = {}

function Start()
    -- Load item template
    local template = GetAsset("UI/InventoryItem.scn")
    self:SetItemTemplate(template)

    -- Initial data
    inventoryData = {
        { name = "Sword", icon = GetAsset("Icons/sword.png"), count = 1 },
        { name = "Shield", icon = GetAsset("Icons/shield.png"), count = 1 },
        { name = "Potion", icon = GetAsset("Icons/potion.png"), count = 5 },
    }

    self:SetData(inventoryData)
end

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()

    local icon = content:FindChild("Icon")
    local nameText = content:FindChild("NameText")

    icon:SetTexture(data.icon)
    nameText:SetText(data.name .. " x" .. data.count)
end

function OnItemClicked(index, data)
    print("Selected item: " .. data.name)
    UseItem(index)
end

function OnSelectionChanged(index, data)
    if index >= 0 then
        ShowItemDetails(data)
    end
end

function UseItem(index)
    local item = inventoryData[index + 1]  -- Lua is 1-indexed
    if item.count > 1 then
        item.count = item.count - 1
        self:UpdateItem(index, item)
    else
        table.remove(inventoryData, index + 1)
        self:RemoveItem(index)
    end
end

function AddItem(itemData)
    table.insert(inventoryData, itemData)
    self:AddItem(itemData)
end
```

## Horizontal Gallery

### Lua Script

```lua
-- Gallery.lua

function Start()
    local template = GetAsset("UI/GalleryThumbnail.scn")
    self:SetItemTemplate(template)

    -- Horizontal layout
    self:SetOrientation("Horizontal")
    self:SetSpacing(10)
    self:SetItemWidth(120)
    self:SetItemHeight(120)

    -- Load images
    local images = {
        { path = "Images/photo1.png", title = "Sunset" },
        { path = "Images/photo2.png", title = "Mountains" },
        { path = "Images/photo3.png", title = "Ocean" },
    }

    self:SetData(images)
end

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    local thumbnail = content:FindChild("Thumbnail")
    local title = content:FindChild("Title")

    thumbnail:SetTexture(GetAsset(data.path))
    title:SetText(data.title)
end

function OnItemClicked(index, data)
    OpenFullscreenViewer(data.path)
end
```

## Selection Highlighting

### Lua Script

```lua
-- SelectableList.lua

local normalColor = Vec4.New(0.2, 0.2, 0.2, 1.0)
local selectedColor = Vec4.New(0.3, 0.5, 0.8, 1.0)
local hoverColor = Vec4.New(0.3, 0.3, 0.3, 1.0)

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    local bg = content:FindChild("Background")

    -- Set initial color
    bg:SetColor(normalColor)

    -- Store reference for hover/selection updates
    data._background = bg
end

function OnItemHoverEnter(index, data)
    if self:GetSelectedIndex() ~= index then
        data._background:SetColor(hoverColor)
    end
end

function OnItemHoverExit(index, data)
    if self:GetSelectedIndex() ~= index then
        data._background:SetColor(normalColor)
    end
end

function OnSelectionChanged(index, data)
    -- Reset all backgrounds
    for i = 0, self:GetItemCount() - 1 do
        local itemData = self:GetItemData(i)
        if itemData._background then
            itemData._background:SetColor(normalColor)
        end
    end

    -- Highlight selected
    if index >= 0 and data._background then
        data._background:SetColor(selectedColor)
    end
end
```

## Dynamic Data Updates

### Lua Script

```lua
-- LiveFeed.lua

local feedData = {}

function Start()
    self:SetItemTemplate(GetAsset("UI/FeedItem.scn"))
    self:SetSpacing(5)
end

function Tick(dt)
    -- Check for new data periodically
    if HasNewFeedItems() then
        local newItems = GetNewFeedItems()
        for _, item in ipairs(newItems) do
            AddFeedItem(item)
        end
    end
end

function AddFeedItem(item)
    -- Add to front of list
    table.insert(feedData, 1, item)
    self:AddItem(item, 0)

    -- Limit to 50 items
    while #feedData > 50 do
        table.remove(feedData)
        self:RemoveItem(self:GetItemCount() - 1)
    end
end

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    local message = content:FindChild("Message")
    local timestamp = content:FindChild("Timestamp")

    message:SetText(data.message)
    timestamp:SetText(FormatTime(data.time))
end

function RefreshItem(index)
    local data = feedData[index + 1]
    self:UpdateItem(index, data)
end
```

## Filtering and Sorting

### Lua Script

```lua
-- FilterableList.lua

local allData = {}
local filteredData = {}
local currentFilter = ""
local sortAscending = true

function Start()
    self:SetItemTemplate(GetAsset("UI/ListItem.scn"))

    -- Load all data
    allData = LoadAllItems()
    ApplyFilter()
end

function SetFilter(filterText)
    currentFilter = filterText:lower()
    ApplyFilter()
end

function SetSortOrder(ascending)
    sortAscending = ascending
    ApplyFilter()
end

function ApplyFilter()
    -- Filter
    filteredData = {}
    for _, item in ipairs(allData) do
        if currentFilter == "" or item.name:lower():find(currentFilter) then
            table.insert(filteredData, item)
        end
    end

    -- Sort
    table.sort(filteredData, function(a, b)
        if sortAscending then
            return a.name < b.name
        else
            return a.name > b.name
        end
    end)

    -- Update list
    self:SetData(filteredData)
end

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    content:FindChild("Name"):SetText(data.name)
    content:FindChild("Description"):SetText(data.description)
end
```

## Lazy Loading / Pagination

### Lua Script

```lua
-- PaginatedList.lua

local pageSize = 20
local currentPage = 0
local totalItems = 0
local displayedData = {}

function Start()
    self:SetItemTemplate(GetAsset("UI/PageItem.scn"))
    totalItems = GetTotalItemCount()
    LoadPage(0)
end

function LoadPage(page)
    currentPage = page
    displayedData = FetchItems(page * pageSize, pageSize)
    self:SetData(displayedData)
    self:GetScrollContainer():ScrollToTop()
end

function NextPage()
    if (currentPage + 1) * pageSize < totalItems then
        LoadPage(currentPage + 1)
    end
end

function PrevPage()
    if currentPage > 0 then
        LoadPage(currentPage - 1)
    end
end

function OnItemGenerate(index, data, itemWidget)
    local content = itemWidget:GetContentWidget()
    content:FindChild("Title"):SetText(data.title)
    content:FindChild("Index"):SetText(tostring(currentPage * pageSize + index + 1))
end
```

## Custom Item Events

### Item Template Script

Attach this script to your item template root widget:

```lua
-- ListItem.lua (on item template)

function OnHoverEnter(itemWidget)
    -- Scale up on hover
    self:SetScale(1.05, 1.05)
end

function OnHoverExit(itemWidget)
    -- Scale back to normal
    self:SetScale(1.0, 1.0)
end

function OnSelected(itemWidget)
    -- Play selection animation
    PlayAnimation("select")
end

function OnDeselected(itemWidget)
    -- Reset
    PlayAnimation("deselect")
end
```
