# MaterialLite

A material that is supported on all platforms.

Inheritance:
* [Asset](Asset.md)
* [Material](Material.md)

---
### MaterialLite.Create(srcMat)
Create a new MaterialLite instance. A source MaterialLite can be passed in to initialize the new material's parameters.

Sig: `matLite = MaterialLite.Create(srcMat=nil)`
 - Arg: `MaterialLite srcMat` Optional source material 
 - Ret: `MaterialLite matLite` Newly created material
---
### SetTexture
Set the texture at a given slot. MaterialLite supports up to 4 textures.

Sig: `MaterialLite:SetTexture(slot, texture)`
 - Arg: `integer slot` Texture slot
 - Arg: `Texture texture` Texture asset
---
### GetTexture
Get the texture assigned to a given slot. MaterialLite supports up to 4 textures.

Sig: `texture = MaterialLite:GetTexture(slot)`
 - Arg: `integer slot` Texture slot
 - Ret: `Texture texture` Assigned texture asset
---
### GetShadingModel
Get the shading model.

See [ShadingModel](../Misc/Enums.md#shadingmodel)

Sig: `shadingModel = MaterialLite:GetShadingModel()`
 - Ret: `ShadingModel(integer) shadingModel` Material shading model
---
### SetShadingModel
Set the shading model.

See [ShadingModel](../Misc/Enums.md#shadingmodel)

Sig: `MaterialLite:SetShadingModel(shadingModel)`
 - Arg: `ShadingModel(integer) shadingModel` Material shading model
---
### SetBlendMode
Set the blend mode.

See [BlendMode](../Misc/Enums.md#blendmode)

Sig: `MaterialLite:SetBlendMode(blendMode)`
 - Arg: `BlendMode(integer) blendMode` Material blend mode
---
### GetUvOffset
Get the UV offset for a given UV channel.

Sig: `offset = MaterialLite:GetUvOffset(index=1)`
 - Arg: `integer index` UV index (either 1 or 2)
 - Ret: `Vector offset` UV offset
---
### SetUvOffset
Set the UV offset for a given UV channel.

Sig: `MaterialLite:SetUvOffset(offset, index=1)`
 - Arg: `Vector offset` UV offset
 - Arg: `integer index` UV index (either 1 or 2)
---
### GetUvScale
Get the UV scale for a given UV channel.

Sig: `scale = MaterialLite:GetUvScale(index=1)`
 - Arg: `integer index` UV index (either 1 or 2)
 - Ret: `Vector scale` UV scale
---
### SetUvScale
Set the UV scale for a given UV channel.

Sig: `MaterialLite:SetUvScale(scale, index=1)`
 - Arg: `Vector scale` UV scale
 - Arg: `integer index` UV index (either 1 or 2)
---
### GetColor
Get the material color.

Sig: `color = MaterialLite:GetColor()`
 - Ret: `Vector color` Material color
---
### SetColor
Set the material color.

Sig: `MaterialLite:SetColor(color)`
 - Arg: `Vector color` Material color
---
### GetFresnelColor
Get the fresnel color.

Sig: `color = MaterialLite:GetFresnelColor()`
 - Ret: `Vector color` Fresnel color
---
### SetFresnelColor
Set the fresnel color.

Sig: `MaterialLite:SetFresnelColor(color)`
 - Arg: `Vector color` Fresnel color
---
### GetFresnelPower
Get the fresnel power.

Sig: `power = MaterialLite:GetFresnelPower()`
 - Ret: `number power` Fresnel power
---
### SetFresnelPower
Set the fresnel power.

Sig: `MaterialLite:SetFresnelPower(power)`
 - Arg: `number power` Fresnel power
---
### GetEmission
Get the emission intensity. This is only used by the experimental light baking and path tracing mode.

Sig: `emission = MaterialLite:GetEmission()`
 - Ret: `number emission` Emission intensity
---
### SetEmission
Set the emission intensity. This is only used by the experimental light baking and path tracing mode.

Sig: `MaterialLite:SetEmission(emission)`
 - Arg: `number emission` Emission intensity
---
### GetWrapLighting
Get the wrap lighting intensity. This is only supported on Vulkan platforms.

Sig: `wrap = MaterialLite:GetWrapLighting()`
 - Ret: `number wrap` Wrap lighting intensity
---
### SetWrapLighting
Set the wrap lighting intensity. This is only supported on Vulkan platforms.

Sig: `MaterialLite:SetWrapLighting(wrap)`
 - Arg: `number wrap` Wrap lighting intensity
---
### GetSpecular
Get the specular intensity.

Sig: `specular = MaterialLite:GetSpecular()`
 - Ret: `number specular` Specular intensity
---
### SetSpecular
Set the specular intensity.

Sig: `MaterialLite:SetSpecular(specular)`
 - Arg: `number specular` Specular intensity
---
### GetOpacity
Get the opacity (used by Translucent and Additive materials).

Sig: `opacity = MaterialLite:GetOpacity()`
 - Ret: `number opacity` Material opacity (0 - 1)
---
### SetOpacity
Set the opacity (used by Translucent and Additive materials).

Sig: `MaterialLite:SetOpacity(opacity)`
 - Arg: `number opacity` Material opacity (0 - 1)
---
### SetMaskCutoff
Set the mask cutoff (used by Masked materials).

Sig: `MaterialLite:SetMaskCutoff(cutoff)`
 - Arg: `number cutoff` Mask cutoff (0 - 1)
---
### SetSortPriority
Set the sort priority. This is used mainly by translucent rendering. Objects will be sorted by distance from the camera if their priorities are the same.

Sig: `MaterialLite:SetSortPriority(priority)`
 - Arg: `integer priority` Sort priority
---
### SetDepthTestDisabled
Set whether depth testing is disabled for this material. TODO: This should probably be changed to "SetDepthTestingEnabled()".

Sig: `MaterialLite:SetDepthTestDisabled(disabled)`
 - Arg: `boolean disabled` True to DISABLE depth testing
---
### GetUvMap
Get the assigned UV map for a given texture slot.

Sig: `uvMap = MaterialLite:GetUvMap(slot)`
 - Arg: `integer slot` Texture slot
 - Ret: `integer uvMap` Assigned UV map (either 1 or 2)
---
### SetUvMap
Set the assigned UV map for a given texture slot.

Sig: `MaterialLite:SetUvMap(slot, uvMap)`
 - Arg: `integer slot` Texture slot
 - Arg: `integer uvMap` UV map index (either 1 or 2)
---
### GetTevMode
Get the assigned TEV mode for a given texture slot. The TEV controls how multiple textures are combined together.

See [TevMode](../Misc/Enums.md#tevmode)

Sig: `tevMode = MaterialLite:GetTevMode(slot)`
 - Arg: `integer slot` Texture slot
 - Ret: `TevMode(integer) tevMode` TEV mode
---
### SetTevMode
Set the assigned TEV mode for a given texture slot. The TEV controls how multiple textures are combined together.

See [TevMode](../Misc/Enums.md#tevmode)

Sig: `MaterialLite:SetTevMode(slot, tevMode)`
 - Arg: `integer slot` Texture slot
 - Arg: `TevMode(integer) tevMode` TEV mode
---