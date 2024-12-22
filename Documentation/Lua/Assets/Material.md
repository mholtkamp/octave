# Material

Material assets are created in the Editor and cannot be imported from a source file.

There are 2 different material systems in Octave.
 - MaterialLite: Can be used on ALL platforms. The downside is that they only support a limited number of features.
 - MaterialBase / MaterialInstance: These are only supported on Vulkan platforms. MaterialBase allows you to provide a custom Vertex / Fragment shader in a GLSL file. MaterialInstance allows you to make a new instance of the base material with different parameter values.

Inheritance:
* [Asset](Asset.md)

---
### IsBase
Check if this material is a MaterialBase.

Sig: `base = Material:IsBase()`
 - Ret: `boolean base` Is base material
---
### IsInstance
Check if this material is a MaterialInstance.

Sig: `instance = Material:IsInstance()`
 - Ret: `boolean instance` Is instance material
---
### IsLite
Check if this material is a MaterialLite.

Sig: `lite = Material:IsLite()`
 - Ret: `boolean lite` Is lite material
---
### SetScalarParameter
Set a scalar parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `Material:SetScalarParameter(name, value)`
 - Arg: `string name` Parameter name
 - Arg: `number value` Parameter value
---
### SetVectorParameter
Set a vector parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `Material:SetVectorParameter(name, value)`
 - Arg: `string name` Parameter name
 - Arg: `Vector value` Parameter value
---
### SetTextureParameter
Set a texture parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `Material:SetTextureParameter(name, value)`
 - Arg: `string name` Parameter name
 - Arg: `Texture value` Parameter value
---
### GetScalarParameter
Get a scalar parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `value = Material:GetScalarParameter(name)`
 - Arg: `string name` Parameter name
 - Ret: `number value` Parameter value
---
### GetVectorParameter
Get a vector parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `value = Material:GetVectorParameter(name)`
 - Arg: `string name` Parameter name
 - Ret: `Vector value` Parameter value
---
### GetTextureParameter
Get a texture parameter on the material. Only used by MaterialBase and MaterialInstance.

Sig: `value = Material:GetTextureParameter(name)`
 - Arg: `string name` Parameter name
 - Ret: `Texture value` Parameter value
---
### GetBlendMode
Get the material's blend mode.

See [BlendMode](../Misc/Enums.md#blendmode)

Sig: `blendMode = Material:GetBlendMode()`
 - Ret: `BlendMode(integer) blendMode` Material blending mode
---
### GetMaskCutoff
Get the material's alpha mask cutoff. Shaded fragments will be discarded if their alpha value is below this threshold.

Sig: `cutoff = Material:GetMaskCutoff()`
 - Ret: `number cutoff` Alpha discard threshold (0 to 1)
---
### GetSortPriority
Get the material's sort priority. This is used mainly by translucent rendering. Objects will be sorted by distance from the camera if their priorities are the same.

Sig: `priority = Material:GetSortPriority()`
 - Ret: `integer priority` Sort priority
---
### IsDepthTestDisabled
Check if depth testing is disabled for this material. TODO: This should probably be changed to "IsDepthTestingEnabled()".

Sig: `disabled = Material:IsDepthTestDisabled()`
 - Ret: `boolean disabled` True if depth testing is DISABLED
---
### ShouldApplyFog
Check if fog should be applied to the resulting shaded fragment.

Sig: `applyFog = Material:ShouldApplyFog()`
 - Ret: `boolean applyFog` Should apply fog
---
### GetCullMode
Get the cull mode.

See [CullMode](../Misc/Enums.md#cullmode)

Sig: `cullMode = Material:GetCullMode()`
 - Ret: `CullMode(integer) cullMode` Triangle cull mode
---
