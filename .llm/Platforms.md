# Platform Support & System Layer

## Overview

The engine abstracts platform differences through a C-style API in `System.h` with platform-specific implementations. Five platforms are supported: Windows, Linux, Android, GameCube/Wii (Dolphin), and Nintendo 3DS.

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Source/System/System.h` | Platform-agnostic system API (SYS_* functions) |
| `Engine/Source/System/SystemTypes.h` | Platform-specific typedefs, SystemState, enums |
| `Engine/Source/System/Windows/` | Windows implementation |
| `Engine/Source/System/Linux/` | Linux implementation |
| `Engine/Source/System/Android/` | Android implementation |
| `Engine/Source/System/Dolphin/` | GameCube/Wii implementation |
| `Engine/Source/System/3DS/` | Nintendo 3DS implementation |
| `Engine/Source/Input/Input.h` | Input API (INP_* functions) |
| `Engine/Source/Audio/Audio.h` | Audio API (AUD_* functions) |
| `Engine/Source/Network/Network.h` | Network API (NET_* functions) |

## System API (System.h)

All functions prefixed with `SYS_`. Categories:

**Lifecycle:** `SYS_Initialize()`, `SYS_Shutdown()`, `SYS_Update()`

**File I/O:**
- `SYS_DoesFileExist(path, isAsset)`, `SYS_AcquireFileData(path, ...)`, `SYS_ReleaseFileData(data)`
- `SYS_CreateDirectory()`, `SYS_RemoveDirectory()`, `SYS_RemoveFile()`, `SYS_Rename()`
- `SYS_CopyFile()`, `SYS_CopyDirectory()`, `SYS_CopyDirectoryRecursive()`
- `SYS_OpenDirectory()`, `SYS_IterateDirectory()`, `SYS_CloseDirectory()` — directory iteration

**Paths:** `SYS_GetExecutablePath()`, `SYS_GetOctavePath()`, `SYS_GetCurrentDirectoryPath()`, `SYS_GetAbsolutePath()`

**Threading:**
- `SYS_CreateThread(func, arg)`, `SYS_JoinThread()`, `SYS_DestroyThread()`
- `SYS_CreateMutex()`, `SYS_LockMutex()`, `SYS_UnlockMutex()`, `SYS_DestroyMutex()`
- `SYS_Sleep(milliseconds)`
- `SCOPED_LOCK(mutex)` — RAII wrapper

**Time:** `SYS_GetTimeMicroseconds()`

**Process:** `SYS_Exec(cmd, output)`, `SYS_ExecFull(cmd, stdout, stderr, exitCode)`

**Memory:** `SYS_AlignedMalloc(size, alignment)`, `SYS_AlignedFree()`, `SYS_GetMemoryStats()`

**Save games:** `SYS_ReadSave()`, `SYS_WriteSave()`, `SYS_DoesSaveExist()`, `SYS_DeleteSave()`, `SYS_UnmountMemoryCard()`

**Clipboard:** `SYS_SetClipboardText()`, `SYS_GetClipboardText()`

**Window:** `SYS_SetWindowTitle()`, `SYS_SetFullscreen()`, `SYS_GetWindowRect()`, `SYS_DoesWindowHaveFocus()`

**Dialogs:** `SYS_OpenFileDialog()`, `SYS_SaveFileDialog()`, `SYS_SelectFolderDialog()`

## Platform-Specific Types (SystemTypes.h)

Thread/mutex types differ per platform:

| Platform | ThreadObject | MutexObject |
|----------|-------------|-------------|
| Windows | `HANDLE` | `HANDLE` |
| Linux/Android | `pthread_t` | `pthread_mutex_t` |
| GameCube | `lwp_t` | `mutex_t` |
| 3DS | `Thread` | `uint32_t` |

**SystemState** struct holds platform-specific window/device state (HWND on Windows, xcb_window on Linux, ANativeWindow on Android, etc.).

**LogSeverity** enum: `Debug`, `Warning`, `Error`, `Count`.

**ScreenOrientation** enum: `Landscape`, `Portrait`, `Auto`.

## Platform Macros

Defined at compile time (one per build):
- `PLATFORM_WINDOWS=1`
- `PLATFORM_LINUX=1`
- `PLATFORM_ANDROID=1`
- `PLATFORM_DOLPHIN=1`
- `PLATFORM_3DS=1`

Graphics backend (one per build):
- `API_VULKAN=1` (Windows, Linux, Android)
- `API_GX=1` (GameCube/Wii)
- `API_C3D=1` (3DS)

## Input System

**File:** `Engine/Source/Input/Input.h` — all functions prefixed with `INP_`.

Platform implementations in `Input/Windows/`, `Input/Linux/`, etc.

**Keyboard:** `INP_IsKeyDown(key)`, `INP_IsKeyJustDown(key)`, `INP_IsKeyJustUp(key)`

**Mouse:** `INP_IsMouseButtonDown(button)`, `INP_GetMousePosition(x, y)`, `INP_GetMouseDelta(dx, dy)`, `INP_GetScrollWheelDelta()`

**Gamepad:** `INP_IsGamepadButtonDown(button, index)`, `INP_GetGamepadAxisValue(axis, index)`, `INP_GetGamepadType(index)`, `INP_IsGamepadConnected(index)`

**Touch (mobile):** `INP_IsTouchDown(touch)`, `INP_SetTouchPosition()`, `INP_GetTouchPositionNormalized()`

**Cursor:** `INP_ShowCursor()`, `INP_LockCursor()`, `INP_TrapCursor()`

Gamepad types: `Standard` (Xbox/generic), `GameCube`, `Wiimote`, `WiiClassic`.

## Audio System

**File:** `Engine/Source/Audio/Audio.h` — all functions prefixed with `AUD_`.

Platform implementations in `Audio/Windows/` (XAudio2), `Audio/Linux/` (ALSA/PulseAudio), `Audio/Android/` (OpenSL ES), `Audio/Dolphin/` (DSP), `Audio/3DS/` (hardware).

**Playback:** `AUD_Play(voiceIndex, soundWave, volume, pitch, loop, startTime, spatial)`, `AUD_Stop()`, `AUD_IsPlaying()`

**Codec:** `AUD_EncodeVorbis()`, `AUD_DecodeVorbis()`

## Network System

**File:** `Engine/Source/Network/Network.h` — all functions prefixed with `NET_`.

**Low-level sockets:**
- `NET_SocketCreate()`, `NET_SocketBind()`, `NET_SocketRecvFrom()`, `NET_SocketSendTo()`, `NET_SocketClose()`
- `NET_GetIpAddress()`, `NET_IpStringToUint32()`

**Abstract platform layer** (`NetPlatform.h`):
- `NetPlatform` base class with `Create()`, `Login()`, `OpenSession()`, `JoinSession()`, `SendMessage()`, `RecvMessage()`
- Implementations: LAN, Epic Online Services (`NetPlatformEpic.h`), Steam (planned)

## Platform Tier

`SYS_GetPlatformTier()` returns a capability level used by the engine to adjust quality settings.

## Console-Specific Notes

**GameCube/Wii** (Dolphin SDK): FIFO-based GX graphics, memory card save system, 65535 max vertices per mesh, 10 max GPU bones.

**3DS** (citro3d): Dual-screen (top stereo + bottom touch), 65535 max vertices, 16 max GPU bones, DVLE shaders, fixed-function + programmable pipeline.
