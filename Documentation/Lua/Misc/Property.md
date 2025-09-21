# Property

Functions for creating script properties that are automatically exposed to the editor.

---
### Property.Create
Creates a property that will appear in the editor on any nodes containing this script. This function should be called during the script's `Create()` method and assigned to a variable on `self`. The property will automatically be detected and exposed in the editor with the appropriate UI controls.

See [DatumType](./Enums.md#datumtype)

**Basic Usage:**
```lua
function MyScript:Create()
    self.gravity = Property.Create(DatumType.Float, 200)
    self.name = Property.Create(DatumType.String, "Michael")
    self.myAsset = Property.Create(DatumType.Asset, nil, "my awesome asset")
end
```

Sig: `value = Property.Create(type, defaultValue, displayName)`
 - Arg: `DatumType type` The data type of the property (see DatumType enum)
 - Arg: `any defaultValue` The default value for the property
 - Arg: `string displayName` *(optional)* Custom name to display in the editor. If not provided, the variable name will be used
 - Ret: `any value` Returns the default value
---

### Property.CreateArray
Creates an array property that will appear in the editor as a list with + and - buttons to add/remove elements. This function should be called during the script's `Create()` method and assigned to a variable on `self`. Each element in the array will have the appropriate UI control based on the specified type.

See [DatumType](./Enums.md#datumtype)

**Basic Usage:**
```lua
function MyScript:Create()
    self.myNumberList = Property.CreateArray(DatumType.Float, {1.0, 2.5, 3.7})
    self.myAssetList = Property.CreateArray(DatumType.Asset, {})
    self.nameList = Property.CreateArray(DatumType.String, {"Player1", "Player2"}, "Player Names")
end
```

Sig: `value = Property.CreateArray(type, arrayValues, displayName)`
 - Arg: `DatumType type` The data type of each element in the array (see DatumType enum)
 - Arg: `table arrayValues` Table containing the initial values for the array
 - Arg: `string displayName` *(optional)* Custom name to display in the editor. If not provided, the variable name will be used
 - Ret: `table value` Returns the array of values as a Lua table
---
