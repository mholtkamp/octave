# Light3D

Base class for lights.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)

---
### SetColor
Set the light color.

Sig: `Light3D:SetColor(color)`
 - Arg: `Vector color` Color
---
### GetColor
Get the light color.

Sig: `color = Light3D:GetColor()`
 - Ret: `Vector color` Color
---
### SetIntensity
Set the light intensity.

Sig: `Light3D:SetIntensity(intensity)`
 - Arg: `number intensity`
---
### GetIntensity
Get the light intensity.

Sig: `intensity = Light3D:GetIntensity()`
 - Ret: `number intensity`
---
### SetCastShadows
Enable this light to cast shadows. Shadow casting is only supported on Vulkan, and as of writing this, hasn't been implemented yet.

Sig: `Light3D:SetCastShadows(castShadows)`
 - Arg: `boolean castShadows` Whether to cast shadows
---
### ShouldCastShadows
Check if this light is casting shadows.

Sig: `castShadows = Light3D:ShouldCastShadows()`
 - Ret: `boolean castShadows` Whether this light should cast shadows
---
### GetDomain
Get this light's domain (static light (baked), dynamic light, or both).
TODO: Add LightingDomain enum.
Static=0
Dynamic=1
All=2

Sig: `domain = Light3D:GetDomain()`
 - Ret: `integer domain` Lighting domain
---
### GetLightingChannels
Get this light's lighting channels. Lights will only affect primitives that are on the same channels. There are only 8 lighting channels, represented by bits (0xff = all lighting channels).

Sig: `channels = Light3D:GetLightingChannels()`
 - Ret: `integer channels` Lighting channels
---
### SetLightingChannels
Set this light's lighting channels. Lights will only affect primitives that are on the same channels. There are only 8 lighting channels, represented by bits (0xff = all lighting channels).

Sig: `Light3D:SetLightingChannels(channels)`
 - Arg: `integer channels` Lighting channels
---