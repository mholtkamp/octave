# Script

System to manage scripts.

---
### Load
Load a script file. The path should be given relative to the Scripts directory. Adding the .lua extension is optional.

Alias: `Require`

Sig: `Script.Load(path)`
 - Arg: `string path` Script file path
---
### Run
Run a script file. Similar to Load, but it will run the script even if the lua file has already been loaded. May be useful for running scripts in editor.

Sig: `Script.Run(path)`
 - Arg: `string path` Script file path
---
### Inherit
Extend one class table from another class table. Useful for creating inheritance hierarchies.

Example in a Goblin.lua file: 
```lua
Script.Load(Monster) -- Load parent class script (if not already loaded)
Goblin = {} -- Create your new class's global table
Script.Inherit(Goblin, Monster) -- Extend your new class table from its parent class
```

Alias: `Extend`

Sig: `Script.Inherit(childTable, parentTable)`
 - Arg: `table childTable` Child class table
 - Arg: `table parentTable` Parent class table
---
### New
Create a new table that inherits from a given parent table. This can be used to essentially create a new instance of a table.

Sig: `inst = Script.New(parent)`
 - Arg: `table parent` Parent table
 - Ret: `table inst` New table that inherits from parent
---
### GarbageCollect
Run Lua garbage collection.

Sig: `Script.GarbageCollect()`

---
### LoadDirectory
Load an entire directory of scripts (if they aren't loaded already).

Sig: `Script.LoadDirectory(dirPath, recurse=true)`
 - Arg: `string dirPath` Directory path
 - Arg: `boolean recurse` Whether to recursively load child directories
---
