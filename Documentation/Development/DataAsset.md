# DataAsset

DataAssets are data containers similar to Unity's ScriptableObjects. They allow you to define custom data structures that can be created, edited in the inspector, saved as assets, and referenced by other assets or nodes.

## Use Cases

- Game configuration (enemy stats, item databases, dialogue)
- Shared data between scenes/nodes
- Designer-editable settings without code changes
- Reusable data templates

## Creating a DataAsset

### Step 1: Create a Lua Definition Script

Create a Lua script that defines the properties your DataAsset will have:

```lua
-- EnemyStats.lua (place in your project's Scripts folder)
function GetProperties()
    return {
        { name = "health", type = DatumType.Integer, default = 100 },
        { name = "damage", type = DatumType.Integer, default = 10 },
        { name = "speed", type = DatumType.Float, default = 5.0 },
        { name = "deathSound", type = DatumType.Asset },
    }
end
```

### Step 2: Create the DataAsset in Editor

1. Right-click in the Asset Browser
2. Select **Create Asset > Data Asset**
3. Name your asset (e.g., `DA_Goblin`)
4. In the Inspector, set the **Script File** property to your Lua script (e.g., `EnemyStats.lua`)
5. The properties defined in your script will appear in the Inspector
6. Configure the values and save

## Property Types

All standard `DatumType` values are supported:

| Type | Lua Name | Default Value |
|------|----------|---------------|
| Integer | `DatumType.Integer` | `0` |
| Float | `DatumType.Float` | `0.0` |
| Bool | `DatumType.Bool` | `false` |
| String | `DatumType.String` | `""` |
| Vector2D | `DatumType.Vector2D` | `{0, 0}` |
| Vector | `DatumType.Vector` | `{0, 0, 0}` |
| Color | `DatumType.Color` | `{1, 1, 1, 1}` |
| Asset | `DatumType.Asset` | `nil` |
| Byte | `DatumType.Byte` | `0` |
| Short | `DatumType.Short` | `0` |

### Array Properties

Add `array = true` to create an array property:

```lua
{ name = "spawnPoints", type = DatumType.Vector, array = true }
```

## Accessing DataAssets from Lua

### Loading a DataAsset

```lua
local enemyStats = Asset.Load("DA_Goblin")
```

### Getting Property Values

```lua
local health = enemyStats:Get("health")
local damage = enemyStats:Get("damage")
local speed = enemyStats:Get("speed")
local deathSound = enemyStats:Get("deathSound")
```

### Setting Property Values (Runtime Only)

```lua
enemyStats:Set("health", 150)
enemyStats:Set("speed", 7.5)
```

## Drag-and-Drop Support

DataAssets work seamlessly with the editor's drag-and-drop system. You can:

1. **Drag DataAssets to Script properties**: Any script property using `DatumType.Asset` can accept a DataAsset
2. **Reference in Inspector**: Select assets from the asset picker dropdown
3. **Cross-reference**: DataAssets can reference other DataAssets

### Example: Artist-Friendly Workflow

Create a script that accepts a DataAsset:

```lua
-- EnemyScript.lua
function GetProperties()
    return {
        { name = "statsAsset", type = DatumType.Asset },  -- Artists drag DataAsset here
        { name = "spawnCount", type = DatumType.Integer, default = 1 },
    }
end

function Enemy:Create()
    local stats = self.statsAsset
    self.health = stats:Get("health")
    self.damage = stats:Get("damage")
end
```

Artists can then drag-and-drop `DA_Goblin` or `DA_Skeleton` onto the `statsAsset` property to configure different enemy types.

## Template Script

A template script is provided at `Engine/Scripts/DataAssetTemplate.lua`. Copy this to your project's Scripts folder as a starting point.

## Best Practices

1. **Naming Convention**: Prefix DataAsset names with `DA_` (e.g., `DA_EnemyStats`, `DA_ItemDatabase`)
2. **Organization**: Keep definition scripts organized in a `Scripts/DataAssets/` folder
3. **Reusability**: Design DataAssets to be reusable across multiple nodes/scenes
4. **Documentation**: Add comments to your Lua definition scripts describing each property
