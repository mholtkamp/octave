# 3DS Platform Overview

## Hardware Summary

| Spec | Original 3DS | New 3DS |
|------|-------------|---------|
| **CPU** | ARM11 MPCore (dual-core, 268 MHz) | ARM11 MPCore (quad-core, 804 MHz) |
| **GPU** | PICA200 | PICA200 |
| **Top Screen** | 400 x 240 (stereoscopic 3D) | 400 x 240 (super-stable 3D) |
| **Bottom Screen** | 320 x 240 (resistive touch) | 320 x 240 (resistive touch) |
| **Right Stick** | Circle Pad Pro (accessory) | C-Stick (built-in) |

Octave detects the New 3DS at startup via `APT_CheckNew3DS()` and enables CPU speedup automatically. See `SYS_GetPlatformTier()` which returns **1** on New 3DS and **0** on the original model.

## Toolchain & Build System

3DS builds use the **devkitARM** toolchain from devkitPro. The `DEVKITARM` environment variable must be set.

**Build command:**

```bash
make -f Makefile_3DS -j12
```

**Key build flags:**

| Flag | Value |
|------|-------|
| Architecture | `-march=armv6k -mtune=mpcore -mfloat-abi=hard` |
| Platform define | `PLATFORM_3DS=1` |
| Graphics API define | `API_C3D=1` |
| C++ standard | `gnu++11` |
| RTTI / Exceptions | Disabled (`-fno-rtti -fno-exceptions`) |

**Libraries:** `-lcitro3d -lctru -lm`

**Output:** Static library `Build/3DS/lib<target>.a`, used when linking the final `.3dsx` homebrew executable.

**Docker:** 3DS builds are also supported via Docker (`docker run ... octavegameengine build-3ds`).

## Graphics (Citro3D)

The 3DS graphics backend uses **Citro3D** (C3D), a high-level wrapper around the PICA200 GPU.

| Constant | Value |
|----------|-------|
| `MAX_FRAMES` | 2 |
| `MAX_MESH_VERTEX_COUNT` | 65,535 |
| `MAX_GPU_BONES` | 16 |
| `SUPPORTS_SECOND_SCREEN` | 1 |

**Shaders** are written for the PICA200 vertex processor and compiled with the `picasso` assembler. Source files live in `Engine/Shaders/PICA200/` (`.v.pica` extension) and are compiled into header files in `Engine/Intermediate/3DS/`:

| Shader | Description |
|--------|-------------|
| `StaticMesh` | Static mesh rendering |
| `SkeletalMesh` | Skeletal mesh with bone transforms |
| `Particle` | Particle systems |
| `Quad` | 2D quads / widgets |
| `Text` | Text rendering |

For details on dual-screen rendering and stereoscopic 3D, see [Screens](Screens.md).

## Input

The 3DS input backend supports gamepad and touch input. Keyboard and mouse are not available.

| Input Type | Supported | Notes |
|------------|-----------|-------|
| Gamepad buttons | Yes | A, B, X, Y, L, R, ZL, ZR, D-pad, Start, Select |
| Circle Pad (left stick) | Yes | Normalized to -1.0 to 1.0 |
| C-Stick (right stick) | Yes | New 3DS only, via `irrstCstickRead()` |
| Triggers | Yes | L/R as digital (0.0 or 1.0) |
| Touch | Yes | Bottom screen, via `hidTouchRead()` |
| Accelerometer | Yes | 3-axis, normalized (scale 1/512) |
| Gyroscope | Yes | 3-axis, normalized (scale 1/1024) |
| Soft keyboard | Yes | System keyboard via `swkbdInputText()` |
| Keyboard | No | |
| Mouse | No | |

Gamepad 0 is always connected on 3DS. The accelerometer and gyroscope are enabled during `INP_Initialize()` and disabled on shutdown.

## Audio (NDSP)

Audio uses the **DSP service** via `ndspInit()`.

| Constant | Value |
|----------|-------|
| `AUDIO_MAX_VOICES` | 8 |

**Supported formats:**

- Mono PCM8 / PCM16
- Stereo PCM8 / PCM16

**Features:**

- Linear interpolation (`NDSP_INTERP_LINEAR`)
- Pitch control via sample rate scaling
- Per-channel left/right volume mixing
- Looping support
- 8-bit PCM automatic unsigned-to-signed conversion

Wave buffers are allocated in linear memory (`linearAlloc` / `linearFree`), which is required for DMA transfers to the DSP.

## Networking (SOC)

Networking uses the **SOC service** (`socInit()` / `socExit()`), which provides a BSD-style socket API.

- UDP sockets (`SOCK_DGRAM`)
- Broadcast support
- Non-blocking mode via `ioctl(FIONBIO)`
- IP address queries via `SOCU_GetIPInfo()`
- Buffer size: 1 MB (`SOC_BUFFERSIZE = 0x100000`)

## Asset Pipeline

3DS builds use **embedded mode** -- assets are compiled into the executable or placed in RomFS.

The packaging flow:

1. Assets are cooked to platform-specific `.oct` format
2. `AssetRegistry.txt` is generated
3. Embedded asset headers (`EmbeddedAssets.h/.cpp`) are generated
4. Assets are copied to `{IntermediateDir}/Romfs/` for RomFS packaging
5. The final `.3dsx` + `.smdh` are produced via `make`

See [Packaging Flow](../PackagingFlow.md) and [Build Profiles](../../Info/BuildProfiles.md) for more details.

## Platform-Specific Source Files

| File | Description |
|------|-------------|
| `Engine/Source/System/3DS/System_3DS.cpp` | Platform init/shutdown, file I/O, threading, time |
| `Engine/Source/Graphics/C3D/Graphics_C3D.cpp` | Citro3D rendering backend |
| `Engine/Source/Graphics/C3D/C3dTypes.h` | Graphics context struct |
| `Engine/Source/Input/3DS/Input_3DS.cpp` | Gamepad, touch, motion input |
| `Engine/Source/Audio/3DS/Audio_3DS.cpp` | NDSP audio backend |
| `Engine/Source/Network/3DS/Network_3DS.cpp` | SOC networking backend |
| `Engine/Source/Input/InputConstants.h` | Per-platform input capability flags |
| `Engine/Source/Audio/AudioConstants.h` | `AUDIO_MAX_VOICES` |
| `Engine/Source/Graphics/GraphicsConstants.h` | GPU limits, `SUPPORTS_SECOND_SCREEN` |
| `Engine/Makefile_3DS` | Build configuration |
| `Engine/Shaders/PICA200/` | PICA200 vertex shaders |

## Further Reading

- [Screens](Screens.md) -- dual-screen architecture, stereoscopic 3D, Lua API
