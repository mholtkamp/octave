# Stream

A table which can process a stream of bytes. A stream can be used to read and write to files. They can read/write both binary and ASCII files. 

---
### Create
Create a Stream.

Sig: `stream = Stream.Create()`
 - Ret: `Stream stream` Newly created stream
---
### GetSize
Get the size of the Stream in bytes.

Sig: `size = Stream:GetSize()`
 - Ret: `integer size` Size in bytes
---
### GetPos
Get the Stream cursor position (byte offset where we are reading from or writing to).

Sig: `pos = Stream:GetPos()`
 - Ret: `integer pos` Cursor position
---
### SetPos
Set the Stream cursor position (byte offset where we are reading from or writing to).

Sig: `Stream:SetPos(pos)`
 - Arg: `integer pos` Cursor position
---
### ReadFile
Read the bytes from a file and store them in this Stream.

Sig: `Stream:ReadFile(path, maxSize=0)`
 - Arg: `string path` File path
 - Arg: `integer maxSize` Max number of bytes to store in Stream (0 = no limit).
---
### WriteFile
Write the contents of this Stream to a file.

Sig: `Stream:WriteFile(path)`
 - Arg: `string path` File path 
---
### ReadAsset
Read an asset descriptor from the Stream.

Sig: `asset = Stream:ReadAsset()`
 - Ret: `Asset asset` Asset
---
### WriteAsset
Write an asset descriptor to the Stream.

Sig: `Stream:WriteAsset(asset)`
 - Arg: `Asset asset` Asset
---
### ReadString
Read a string from the Stream.

Sig: `str = Stream:ReadString()`
 - Ret: `string str` String
---
### WriteString
Write a string to the Stream.

Sig: `Stream:WriteString(str)`
 - Arg: `string str` String
---
### ReadInt32
Read a 32-bit integer from the Stream.

Sig: `int = Stream:ReadInt32()`
 - Ret: `integer int` Integer
---
### ReadInt16
Read a 16-bit integer from the Stream.

Sig: `int = Stream:ReadInt16()`
 - Ret: `integer int` Integer
---
### ReadInt8
Read an 8-bit integer from the Stream.

Sig: `int = Stream:ReadInt8()`
 - Ret: `integer int` Integer
---
### ReadFloat
Read a floating-point number from the Stream.

Sig: `float = Stream:ReadFloat()`
 - Ret: `number float` Float
---
### ReadBool
Read a boolean from the Stream.

Sig: `bool = Stream:ReadBool()`
 - Ret: `boolean bool` Boolean
---
### ReadVec2
Read a 2-component Vector from the Stream.

Sig: `vec = Stream:ReadVec2()`
 - Ret: `Vector vec` Vector
---
### ReadVec3
Read a 3-component Vector from the Stream.

Sig: `vec = Stream:ReadVec3()`
 - Ret: `Vector vec` Vector
---
### ReadVec4
Read a 4-component Vector from the Stream.

Sig: `vec = Stream:ReadVec4()`
 - Ret: `Vector vec` Vector
---
### WriteInt32
Write a 32-bit integer to the Stream.

Sig: `Stream:WriteInt32(int)`
 - Arg: `integer int` Integer
---
### WriteInt16
Write a 16-bit integer to the Stream.

Sig: `Stream:WriteInt16(int)`
 - Arg: `integer int` Integer
---
### WriteInt8
Write an 8-bit integer to the Stream.

Sig: `Stream:WriteInt8(int)`
 - Arg: `integer int` Integer
---
### WriteFloat
Write a floating-point number to the Stream.

Sig: `Stream:WriteFloat(float)`
 - Arg: `number float` Float
---
### WriteBool
Write a boolean to the Stream.

Sig: `Stream:WriteBool(bool)`
 - Arg: `boolean bool` Boolean
---
### WriteVec2
Write a 2-component Vector to the Stream.

Sig: `Stream:WriteVec2(vec)`
 - Arg: `Vector vec` Vector
---
### WriteVec3
Write a 3-component Vector to the Stream.

Sig: `Stream:WriteVec3(vec)`
 - Arg: `Vector vec` Vector
---
### WriteVec4
Write a 4-component Vector to the Stream.

Sig: `Stream:WriteVec4(vec)`
 - Arg: `Vector vec` Vector
---
