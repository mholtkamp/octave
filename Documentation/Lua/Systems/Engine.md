# Engine

System to handle generic engine functionality.

---
### Quit
Close the application.

Sig: `Engine.Quit()`

---
### IsShuttingDown
Check if the engine is shutting down / quitting.

Sig: `shuttingDown = Engine.IsShuttingDown()`
 - Ret: `boolean shuttingDown` Is shutting down
---
### GetWorld
Get a world by its index. On most platforms, there will only be one world. On 3DS, world 1 is the top screen and world 2 is the bottom screen.

Sig: `world = Engine.GetWorld(index=1)`
 - Arg: `integer index` World index
 - Ret: `World world` The world
---
### GetTime
Get the time elapsed in seconds since the start of the game.

Sig: `time = Engine.GetTime()`
 - Ret: `number time` Time in seconds since game start
---
### GetGameDeltaTime
Get the amount of time that has elapsed since the previous frame, while respecting time dilation and pausing.

Alias: `GetDeltaTime`

Sig: `deltaTime = Engine.GetGameDeltaTime()`
 - Ret: `number deltaTime` Delta time in seconds
---
### GetRealDeltaTime
Get the amount of time that has elapsed since the previous frame.

Sig: `deltaTime = Engine.GetRealDeltaTime()`
 - Ret: `number deltaTime` Delta time in seconds
---
### GetGameElapsedTime
Get the amount of time that has elapsed in seconds since the start of the game, while respecting time dilation and pausing.

Alias: `GetElapsedTime`

Sig: `time = Engine.GetGameElapsedTime()`
 - Ret: `number time` Elapsed time in seconds
---
### GetRealElapsedTime
Get the amount of time that has elapsed in seconds since the start of the game.

Sig: `time = Engine.GetRealElapsedTime()`
 - Ret: `number time` Elapsed time in seconds
---
### GetPlatform
Get the name of the current platform the game is running on.

Sig: `platform = Engine.GetPlatform()`
 - Ret: `string platform` Platform string (e.g. "Windows", "3DS", "GameCube")
---
### IsEditor
Check if the engine is running in editor.

Sig: `editor = Engine.IsEditor()`
 - Ret: `boolean editor` Is running editor
---
### Break
Break the debugger if attached.

Sig: `Engine.Break()`

---
### Alert
Send an alert message. Useful for debugging.

Sig: `Engine.Alert(msg)`
 - Arg: `string msg` Message string 
---
### SetBreakOnScriptError
Enable breaking the debugger on Lua errors.

Sig: `Engine.SetBreakOnScriptError(break)`
 - Arg: `boolean break` Break on error
---
### IsPlayingInEditor
Check if the engine is playing in editor.

Sig: `pie = Engine.IsPlayingInEditor()`
 - Ret: `boolean pie` Playing in editor
---
### IsPlaying
Check if the game is playing (regular game or PIE).

Sig: `playing = Engine.IsPlaying()`
 - Ret: `boolean playing` Is playing
---
### ReloadAllScripts
Reload all scripts.

Sig: `Engine.ReloadAllScripts(restartScripts=true)`
 - Arg: `boolean restartScripts` Whether to restart currently running scripts
---
### SetPaused
Set whether the game is paused.

Sig: `Engine.SetPaused(pause)`
 - Arg: `boolean pause` Pause game
---
### IsPaused
Check whether the game is paused.

Sig: `paused = Engine.IsPaused()`
 - Ret: `boolean paused` Is game paused
---
### FrameStep
Step one frame forward when the game is paused. Essentially unpause for a single frame.

Sig: `Engine.FrameStep()`

---
### SetTimeDilation
Set the time dilation factor/multiplier.

Sig: `Engine.SetTimeDilation(dilation)`
 - Arg: `number dilation` Time dilation
---
### GetTimeDilation
Get the time dilation factor/multiplier.

Sig: `dilation = Engine.GetTimeDilation()`
 - Ret: `number dilation` Time dilation
---
### GarbageCollect
Garbage collect scripts AND reference sweep assets. This is a slow process and may cause a hitch.

Sig: `Engine.GarbageCollect()`

---
