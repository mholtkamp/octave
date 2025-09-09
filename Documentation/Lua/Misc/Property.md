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
