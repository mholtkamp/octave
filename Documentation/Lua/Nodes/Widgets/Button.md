# Button

A simple button widget. Internally, the button is composed of a Text widget and a Quad widget. Assign a function to a Button node's `:OnActivated` in order to handle when the button is pressed. Alternatively, you can connect to the button's `Activated` signal. Connect to the `StateChanged` signal to handle when the button state changes (See [ButtonState](../../Misc/Enums.md#buttonstate)).

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetSelected
Set a given button as the selected button. Out of all Button nodes, there may only be a single selected node. If mouse handling is enabled, then if a mouse hovers over a button, it will forcefully change the selected node.

Sig: `Button.SetSelected(button)`
 - Arg: `Button button` Button to select
---
### GetSelected
Get the currently selected button. Out of all Button nodes, there may only be a single selected node. If mouse handling is enabled, then if a mouse hovers over a button, it will forcefully change the selected node.

Sig: `selButton = Button.GetSelected()`
 - Ret: `Button selButton` Button that is selected
---
### EnableMouseHandling
Enable mouse handling globally for all buttons.

Sig: `Button.EnableMouseHandling(enable)`
 - Arg: `boolean enable` Enable mouse handling for all buttons
 ---
 ### IsSelected
Check if this button is currently the selected button.

Sig: `Button:Activate()`
 - Ret: `boolean selected` True if the selected button
---
### Activate
Activate the button (as if it were clicked).

Sig: `Button:Activate()`

---
### GetState
Get the button's state.

See [ButtonState](../../Misc/Enums.md#buttonstate)

Sig: `state = Button:GetState()`
 - Ret: `ButtonState(integer) state` Current button state
---
### SetLocked
Set whether the button should be locked. When locked, the button cannot be activated when clicked.

Sig: `Button:SetLocked(locked)`
 - Arg: `boolean locked` Lock
---
### SetTextString
Set the displayed text string.

Sig: `Button:SetTextString(str)`
 - Arg: `string str` New text string
---
### GetTextString
Get the displayed text string.

Sig: `str = Button:GetTextString()`
 - Ret: `string str` Currently displayed text string
---
### SetStateTextures
Change the textures that are displayed based on the button state. Passing a nil will result in the normal state's texture being used for that state. If the normal state texture is nil, pure white will be used. 

Sig: `Button:SetStateTextures(normal, hovered, pressed, locked)`
 - Arg: `Texture normal` Normal texture
 - Arg: `Texture hovered` Hovered texture
 - Arg: `Texture pressed` Pressed texture
 - Arg: `Texture locked` Locked texture
---
### SetStateColors
Change the quad colors (or possibly text colors) that are displayed based on the button state.

Sig: `Button:SetStateTextures(normal, hovered, pressed, locked)`
 - Arg: `Vector normal` Normal color
 - Arg: `Vector hovered` Hovered color
 - Arg: `Vector pressed` Pressed color
 - Arg: `Vector locked` Locked color
---
### GetStateTextures
Get the textures that are displayed based on the button state.

Sig: `normal, hovered, pressed, locked = Button:GetStateTextures()`
 - Ret: `Texture normal` Normal texture
 - Ret: `Texture hovered` Hovered texture
 - Ret: `Texture pressed` Pressed texture
 - Ret: `Texture locked` Locked texture
---
### GetStateColors
Get the quad colors (or possibly text colors) that are displayed based on the button state.

Sig: `normal, hovered, pressed, locked = Button:SetStateTextures()`
 - Ret: `Vector normal` Normal color
 - Ret: `Vector hovered` Hovered color
 - Ret: `Vector pressed` Pressed color
 - Ret: `Vector locked` Locked color
---
### GetText
Get the internal Text widget used by this Button.

Sig: `text = Button:GetText()`
 - Arg: `Text text` Text used by button
---
### GetQuad
Get the internal Quad widget used by this Button.

Sig: `quad = Button:GetQuad()`
 - Arg: `Quad quad` Quad used by button
---
