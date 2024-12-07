# Enums

Tables that act as enumerations.

## BlendMode

 - `Opaque` No blending, render depth
 - `Masked` No blending, render depth, discard pixels based on alpha value
 - `Translucent` Mix color with existing scene color
 - `Additive` Add color with existing scene color

 ## ShadingModel

 - `Unlit` No lighting
 - `Lit` Standard lighting
 - `Toon` Toon shading (Only supported on Vulkan / 3DS right now)

 ## TevMode

 - `Replace` Replace previous color
 - `Modulate`  Multiply against previous color
 - `Decal` Blend with previous color based on alpha
 - `Add` Add with previous color
 - `SignedAdd` Add (color - 0.5) to previous color
 - `Subtract` Subtract previous color from this color
 - `Interpolate` Not yet implemented. (Blend between this sample and previous sample based on uniform value)

## AnchorMode

 - `TopLeft`
 - `TopMid`
 - `TopRight`
 - `MidLeft`
 - `Mid`
 - `MidRight`
 - `BottomLeft`
 - `BottomMid`
 - `BottomRight`
 - `TopStretch`
 - `MidHorizontalStretch`
 - `BottomStretch`
 - `LeftStretch`
 - `MidVerticalStretch`
 - `RightStretch`
 - `FullStretch`

## DatumType
 - `Integer`
 - `Float`
 - `Bool`
 - `String`
 - `Vector2D`
 - `Vector`
 - `Color`
 - `Asset`
 - `Byte`
 - `Table`
 - `Pointer`
 - `Short`

## NetFuncType
 - `Server` Function only executes on the server.
 - `Client` Function executes on the owning client
 - `Multicast` Function executes on all hosts (must be called on the server)

## Justification
 - `Left`
 - `Right`
 - `Center`
 - `Top`
 - `Bottom`

## ScreenOrientation
 - `Landscape`
 - `Portrait`
 - `Auto`

## ParticleOrientation
 - `X`
 - `Y`
 - `Z`
 - `NX` -X
 - `NY` -Y
 - `NZ` -Z
 - `Billboard`

## NetHost
 - `Invalid` Invalid host id (0)
 - `Server` Server host id (1)

## AttenuationFunc

 - `Constant` No attenuation based on distance from sound source
 - `Linear` Linear falloff as sound receiver moves away from sound source
