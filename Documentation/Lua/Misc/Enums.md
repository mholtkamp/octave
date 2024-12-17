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
 - `Modulate` Multiply against previous color
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

## CullMode
 - `None` Do not cull triangle no matter which way it is facing
 - `Back` Cull triangles that are facing away from the camera
 - `Front` Cull triangles that are facing the camera

## Mouse
 - `Left`
 - `Right`
 - `Middle`
 - `X1`
 - `X2`
 - `1`
 - `2`
 - `3`
 - `4`
 - `5`

## Gamepad
 - `A`
 - `B`
 - `C`
 - `X`
 - `Y`
 - `Z`
 - `L1`
 - `R1`
 - `L2`
 - `R2`
 - `L3`
 - `R3`
 - `Start`
 - `Select`
 - `Left`
 - `Right`
 - `Up`
 - `Down`
 - `Home`
 - `AxisL`
 - `AxisR`
 - `AxisLX`
 - `AxisLY`
 - `AxisRX`
 - `AxisRY`

## Key
 - `Back`
 - `Escape`
 - `N0`
 - `N1`
 - `N2`
 - `N3`
 - `N4`
 - `N5`
 - `N6`
 - `N7`
 - `N8`
 - `N9`
 - `Zero`
 - `One`
 - `Two`
 - `Three`
 - `Four`
 - `Five`
 - `Six`
 - `Seven`
 - `Eight`
 - `Nine`
 - `A`
 - `B`
 - `C`
 - `D`
 - `E`
 - `F`
 - `G`
 - `H`
 - `I`
 - `J`
 - `K`
 - `L`
 - `M`
 - `N`
 - `O`
 - `P`
 - `Q`
 - `R`
 - `S`
 - `T`
 - `U`
 - `V`
 - `W`
 - `X`
 - `Y`
 - `Z`
 - `Space`
 - `Enter`
 - `Backspace`
 - `Tab`
 - `ShiftL`
 - `ControlL`
 - `AltL`
 - `ShiftR`
 - `ControlR`
 - `AltR`
 - `Insert`
 - `Delete`
 - `Home`
 - `End`
 - `PageUp`
 - `PageDown`
 - `Up`
 - `Down`
 - `Left`
 - `Right`
 - `Numpad0`
 - `Numpad1`
 - `Numpad2`
 - `Numpad3`
 - `Numpad4`
 - `Numpad5`
 - `Numpad6`
 - `Numpad7`
 - `Numpad8`
 - `Numpad9`
 - `F1`
 - `F2`
 - `F3`
 - `F4`
 - `F5`
 - `F6`
 - `F7`
 - `F8`
 - `F9`
 - `F10`
 - `F11`
 - `F12`
 - `Period`
 - `Comma`
 - `Plus`
 - `Minus`
 - `Colon`
 - `Question`
 - `Squiggle`
 - `LeftBracket`
 - `BackSlash`
 - `RightBracket`
 - `Quote`
 - `Decimal`
