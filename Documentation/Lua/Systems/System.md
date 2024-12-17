# System

Generic platform functions.

---
### WriteSave
Write save data from a Stream.

Sig: `System.WriteSave(name, stream)`
 - Arg: `string name` Name of save file
 - Arg: `Stream stream` Byte stream containing save data (Create one with Stream.Create())
---
### ReadSave
Read save data to a Stream.

Sig: `System.ReadSave(name, stream)`
 - Arg: `string name` Save file name
 - Arg: `Stream stream` Stream to fill with save data.
---
### DoesSaveExist
Check if a save file exists.

Sig: `exists = System.DoesSaveExist(name)`
 - Arg: `string name` Save file name
 - Ret: `boolean exists` Does file exist
---
### DeleteSave
Delete a save file.

Sig: `System.DeleteSave(name)`
 - Arg: `string name` Save file name
---
### UnmountMemoryCard
Unmount the memory card. Octave only uses the first memory card right now.

Sig: `System.UnmountMemoryCard()`

---
### SetScreenOrientation
Set the screen orientation. Only used by Android.

See [ScreenOrientation](../Misc/Enums.md#screenorientation)

Sig: `System.SetScreenOrientation(orientation)`
 - Arg: `ScreenOrientation(integer) orientation` Screen orientation
---
### GetScreenOrientation
Get the screen orientation.

See [ScreenOrientation](../Misc/Enums.md#screenorientation)

Sig: `orientation = System.GetScreenOrientation()`
 - Ret: `ScreenOrientation(integer) orientation` Screen orientation
---
### SetFullscreen
Enable/disable fullscreen.

Sig: `System.SetFullscreen(fullscreen)`
 - Arg: `boolean fullscreen` Whether to enable fullscreen
---
### IsFullscreen
Check if fullscreen is enabled.

Sig: `fullscreen = System.IsFullscreen()`
 - Ret: `boolean fullscreen` Whether fullscreen is enabled
---
### SetWindowTitle
Set the window's title.

Sig: `System.SetWindowTitle(title)`
 - Arg: `string title` Title of the window
---
