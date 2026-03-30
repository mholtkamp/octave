# ToolTip Examples

This document provides practical examples for using the ToolTip system.

## Basic Static Tooltip

The simplest use case - set tooltip in the editor or via Lua:

```lua
function Start()
    local button = self:FindChild("AttackButton")
    button:SetTooltipName("Attack")
    button:SetTooltipDescription("Deal damage to the enemy.\nDamage: 10")
end
```

## Inventory Item Tooltips

Dynamic tooltips that show item information:

```lua
-- Item data
local items = {
    sword = { name = "Iron Sword", damage = 15, value = 100 },
    potion = { name = "Health Potion", heal = 50, value = 25 },
    shield = { name = "Wooden Shield", defense = 5, value = 50 }
}

-- Setup dynamic tooltip callback
function Start()
    ToolTip.SetOnShowCallback(function(widget)
        local slotId = widget:GetName()
        local item = GetInventoryItem(slotId)

        if item then
            local tooltip = ToolTip.GetWidget()
            tooltip:SetTooltipTitle(item.name)

            local desc = ""
            if item.damage then
                desc = desc .. "Damage: " .. item.damage .. "\n"
            end
            if item.heal then
                desc = desc .. "Heals: " .. item.heal .. " HP\n"
            end
            if item.defense then
                desc = desc .. "Defense: " .. item.defense .. "\n"
            end
            desc = desc .. "Value: " .. item.value .. " gold"

            tooltip:SetTooltipText(desc)
        end
    end)
end

function GetInventoryItem(slotId)
    -- Return item data for the slot
    return inventoryData[slotId]
end
```

## Skill Button Tooltips with Cooldowns

Show skill information with real-time cooldown status:

```lua
local skills = {
    fireball = { name = "Fireball", damage = 30, manaCost = 20, cooldown = 5.0 },
    heal = { name = "Heal", heal = 50, manaCost = 30, cooldown = 10.0 }
}

local skillCooldowns = {}

function Start()
    -- Set basic tooltip info
    for skillId, skill in pairs(skills) do
        local button = self:FindChild(skillId .. "Button")
        button:SetTooltipName(skill.name)
        -- Description will be updated dynamically
    end

    -- Dynamic tooltip with cooldown
    ToolTip.SetOnShowCallback(function(widget)
        local skillId = widget:GetName():gsub("Button", "")
        local skill = skills[skillId]

        if skill then
            local tooltip = ToolTip.GetWidget()
            local desc = ""

            if skill.damage then
                desc = desc .. "Damage: " .. skill.damage .. "\n"
            end
            if skill.heal then
                desc = desc .. "Heals: " .. skill.heal .. " HP\n"
            end
            desc = desc .. "Mana Cost: " .. skill.manaCost .. "\n"

            local cd = skillCooldowns[skillId] or 0
            if cd > 0 then
                desc = desc .. "Cooldown: " .. string.format("%.1f", cd) .. "s"
            else
                desc = desc .. "Ready!"
            end

            tooltip:SetTooltipText(desc)
        end
    end)
end
```

## Custom Styled Tooltips

Customize tooltip appearance for different themes:

```lua
-- Dark theme tooltip
function SetDarkTheme()
    local tooltip = ToolTip.GetWidget()
    tooltip:SetBackgroundColor(Vec4.New(0.05, 0.05, 0.1, 0.95))
    tooltip:SetTitleColor(Vec4.New(1, 0.9, 0.6, 1))  -- Gold title
    tooltip:SetTextColor(Vec4.New(0.7, 0.7, 0.7, 1))
    tooltip:SetCornerRadius(8)
    tooltip:SetPadding(12, 10, 12, 10)
end

-- Light theme tooltip
function SetLightTheme()
    local tooltip = ToolTip.GetWidget()
    tooltip:SetBackgroundColor(Vec4.New(0.95, 0.95, 0.95, 0.98))
    tooltip:SetTitleColor(Vec4.New(0.1, 0.1, 0.1, 1))
    tooltip:SetTextColor(Vec4.New(0.3, 0.3, 0.3, 1))
    tooltip:SetCornerRadius(4)
    tooltip:SetPadding(8, 6, 8, 6)
end

-- Rare item tooltip (different color scheme)
function SetRareItemStyle()
    local tooltip = ToolTip.GetWidget()
    tooltip:SetBackgroundColor(Vec4.New(0.1, 0.05, 0.2, 0.95))  -- Purple tint
    tooltip:SetTitleColor(Vec4.New(0.8, 0.5, 1, 1))  -- Purple title
end
```

## Conditional Tooltips

Show tooltips only under certain conditions:

```lua
function Start()
    ToolTip.SetOnShowCallback(function(widget)
        -- Only show detailed tooltips when holding Shift
        if Input.IsKeyDown(Key.LeftShift) then
            local tooltip = ToolTip.GetWidget()
            tooltip:SetTooltipText(GetDetailedInfo(widget))
        else
            local tooltip = ToolTip.GetWidget()
            tooltip:SetTooltipText(GetBasicInfo(widget))
        end
    end)
end

function GetBasicInfo(widget)
    return "Hold Shift for details"
end

function GetDetailedInfo(widget)
    return "Detailed stats:\n- Attack: 15\n- Defense: 10\n- Speed: 8"
end
```

## Disabling Tooltips Temporarily

Disable tooltips during certain game states:

```lua
function OnBattleStart()
    -- Disable tooltips during active combat
    ToolTip.SetEnabled(false)
end

function OnBattleEnd()
    -- Re-enable tooltips
    ToolTip.SetEnabled(true)
end

function OnOpenMenu()
    -- Show tooltips in menus
    ToolTip.SetEnabled(true)
end
```

## Adjusting Tooltip Delay

Different delays for different contexts:

```lua
-- Quick tooltips for combat (experienced players)
function SetCombatMode()
    ToolTip.SetShowDelay(0.2)
end

-- Slower tooltips for tutorials (new players)
function SetTutorialMode()
    ToolTip.SetShowDelay(1.0)
end

-- Default exploration mode
function SetExplorationMode()
    ToolTip.SetShowDelay(0.5)
end
```

## Multi-line Formatted Tooltips

Create well-formatted multi-line tooltips:

```lua
function FormatWeaponTooltip(weapon)
    local lines = {}

    -- Basic stats
    table.insert(lines, "Damage: " .. weapon.minDamage .. "-" .. weapon.maxDamage)
    table.insert(lines, "Speed: " .. weapon.attackSpeed)
    table.insert(lines, "")  -- Empty line for spacing

    -- Special effects
    if weapon.effects then
        table.insert(lines, "Effects:")
        for _, effect in ipairs(weapon.effects) do
            table.insert(lines, "  - " .. effect)
        end
    end

    -- Requirements
    if weapon.levelReq > 1 then
        table.insert(lines, "")
        table.insert(lines, "Requires Level " .. weapon.levelReq)
    end

    return table.concat(lines, "\n")
end
```

## Tooltip with Rich Information

Complex tooltip showing multiple data types:

```lua
function ShowCharacterTooltip(character)
    local tooltip = ToolTip.GetWidget()

    -- Character name with level
    tooltip:SetTooltipTitle(character.name .. " (Lv. " .. character.level .. ")")

    -- Build description
    local desc = string.format(
        "HP: %d/%d\nMP: %d/%d\n\n" ..
        "Attack: %d\nDefense: %d\nSpeed: %d\n\n" ..
        "Class: %s\nStatus: %s",
        character.hp, character.maxHp,
        character.mp, character.maxMp,
        character.attack, character.defense, character.speed,
        character.class,
        character.status or "Normal"
    )

    tooltip:SetTooltipText(desc)

    -- Style based on character type
    if character.isEnemy then
        tooltip:SetTitleColor(Vec4.New(1, 0.3, 0.3, 1))  -- Red for enemies
    elseif character.isAlly then
        tooltip:SetTitleColor(Vec4.New(0.3, 1, 0.3, 1))  -- Green for allies
    else
        tooltip:SetTitleColor(Vec4.New(1, 1, 1, 1))  -- White for neutral
    end
end
```
