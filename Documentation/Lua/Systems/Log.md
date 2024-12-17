# Log

System for printing information to the log and screen.

---
### Debug
Write a string to the log with Debug severity level.

Alias: `Info`

Sig: `Log.Debug(msg)`
 - Arg: `string msg` Message to log
---
### Warning
Write a string to the log with Warning severity level.

Sig: `Log.Warning(msg)`
 - Arg: `string msg` Message to log
---
### Error
Write a string to the log with Error severity level.

Sig: `Log.Error(msg)`
 - Arg: `string msg` Message to log
---
### Console
Write a string to the screen console.

Sig: `Log.Console(msg, color)`
 - Arg: `string msg` Message to log
 - Arg: `Vector color` Text color
---
### Enable
Enable or disable logging and screen console printing.

Sig: `Log.Enable(enable)`
 - Arg: `boolean enable` Whether to enable logging
---
### IsEnabled
Check if logging and screen console printing is enabled.

Sig: `enabled = Log.IsEnabled()`
 - Ret: `boolean enabled` Whether logging is enabled
---
