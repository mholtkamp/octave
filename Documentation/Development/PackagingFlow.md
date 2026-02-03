# Packaging Flow

## Overview

`ActionManager::BuildData(Platform platform, bool embedded)` is the main function responsible for packaging game assets and compiling executables for various target platforms.

**Location:** [ActionManager.cpp:152](Engine/Source/Editor/ActionManager.cpp#L152)

---

## Function Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `platform` | `Platform` | Target platform (Windows, Linux, Android, GameCube, Wii, N3DS) |
| `embedded` | `bool` | Whether to embed assets into the executable |

---

## Flow Diagram

```
BuildData()
    |
    v
[1] Gather Engine State & Paths
    |
    v
[2] Create Packaged Directory
    |
    v
[3] Cook & Save Assets (Engine + Project)
    |
    v
[4] Generate Asset Registry
    |
    v
[5] Generate Embedded Files (Assets + Scripts)
    |
    v
[6] Copy Scripts (or embed them)
    |
    v
[7] Copy Project Files (.octp, Config.ini)
    |
    v
[8] Handle Platform-Specific Tasks (shaders, romfs)
    |
    v
[9] Compile Executable (if needed)
    |
    v
[10] Copy Executable to Packaged Directory
```

---

## Detailed Step-by-Step Flow

### Step 1: Initialize Paths and State

```cpp
std::string octaveDirectory = SYS_GetOctavePath();
const EngineState* engineState = GetEngineState();
bool standalone = engineState->mStandalone;
const std::string& projectDir = engineState->mProjectDirectory;
const std::string& projectName = engineState->mProjectName;
```

**Path Resolution (`SYS_GetOctavePath`):**
- First checks current directory for `Octave/imgui.ini`
- Falls back to executable directory if `Standalone/Standalone.rc` not found
- Returns empty string on consoles (Android, 3DS, Dolphin)

### Step 2: Create Packaged Directory Structure

**Directory Structure Created:**
```
{ProjectDir}/Packaged/
    └── {Platform}/           <- e.g., Windows/, Linux/, N3DS/
        ├── Engine/
        └── {ProjectName}/
```

- Existing platform directory is **deleted** and recreated fresh

### Step 3: Cook Assets

The `saveDir` lambda recursively processes asset directories:

1. Creates corresponding directory in packaged folder
2. Loads each asset (if not already loaded)
3. Saves platform-specific `.oct` file
4. Optionally adds to embedded assets list (if `embedded && !useRomfs`)
5. Unloads asset if it wasn't originally loaded

**Packaged Asset Paths:**
- Engine assets: `{PackagedDir}/Engine/{path}/`
- Project assets: `{PackagedDir}/{ProjectName}/{path}/`

### Step 4: Generate Asset Registry

Creates `{PackagedDir}/{ProjectName}/AssetRegistry.txt` containing:
```
{AssetType},{AssetPath}
{AssetType},{AssetPath}
...
```

### Step 5: Generate Embedded Files

Creates in `{ProjectDir}/Generated/`:
- `EmbeddedAssets.h` / `EmbeddedAssets.cpp`
- `EmbeddedScripts.h` / `EmbeddedScripts.cpp`

If `standalone`, copies Generated folder to `Standalone/`

### Step 6: Handle Scripts

| Mode | Behavior |
|------|----------|
| `embedded && !useRomfs` | Gathers scripts for embedding |
| Otherwise | Copies script folders to packaged directory |

**Script Locations:**
- Engine: `{OctaveDir}/Engine/Scripts/` -> `{PackagedDir}/Engine/Scripts/`
- Project: `{ProjectDir}/Scripts/` -> `{PackagedDir}/{ProjectName}/Scripts/`

Note: `LuaPanda.lua` is removed for non-desktop platforms (saves 148KB)

### Step 7: Copy Project Files

Copies to packaged directory:
- `{ProjectName}.octp` (project file)
- `Config.ini` (configuration)

### Step 8: Platform-Specific Tasks

#### Vulkan Platforms (Windows, Linux, Android)
1. Compiles SPIR-V shaders via `compile.bat` (Windows) or `compile.sh` (Linux)
2. Copies shader binaries to `{PackagedDir}/Engine/Shaders/GLSL/bin/`

#### Nintendo 3DS with Romfs
If `useRomfs` (N3DS + embedded):
- Copies all packaged content to `{IntermediateDir}/Romfs/`

**Intermediate Directory:**
- Standalone: `{OctaveDir}/Standalone/Intermediate`
- Project: `{ProjectDir}/Intermediate`

---

## Headless vs Non-Headless Mode

### Key Difference

The critical distinction is at line 407-411:

```cpp
if (standalone && !IsHeadless() &&
    (platform == Platform::Windows || platform == Platform::Linux))
{
    needCompile = !SYS_DoesFileExist(prebuiltExeName.c_str(), false);
}
```

### Non-Headless Mode (Editor)

| Aspect | Behavior |
|--------|----------|
| Compilation | **Skipped** if prebuilt `Octave.exe`/`Octave.elf` exists |
| Use Case | Faster iteration during development |
| Executable | Reuses existing editor binary |
| Assumption | Editor binary is suitable for running packaged game |

**Flow:**
1. Checks for existing `Octave.exe` or `Octave.elf`
2. If found, sets `needCompile = false`
3. Copies existing executable to packaged folder
4. Renames to project name

### Headless Mode (CI/Full Build)

| Aspect | Behavior |
|--------|----------|
| Compilation | **Always performed** |
| Use Case | CI/CD pipelines, release builds |
| Executable | Fresh compile every time |
| Detection | `IsHeadless()` returns `true` when `-headless` flag AND `-project` path are set |

**Flow:**
1. `needCompile` defaults to `true`
2. Full compilation runs regardless of existing binaries
3. Builds optimized release executable

### Headless Detection

```cpp
// Engine/Source/Engine/Engine.cpp:183
bool IsHeadless()
{
    return sEngineConfig.mHeadless && sEngineConfig.mProjectPath != "";
}
```

Both conditions required:
1. `-headless` command line flag
2. Valid project path specified via `-project`

---

## Compilation by Platform

### Windows
```
devenv Octave.sln /Build "Release|x64" /Project {ProjectName}
```
With Steam: Uses `ReleaseSteam|x64` configuration

### Linux
```
make -C {BuildProjDir} -f Makefile_TEMP -j 12
strip --strip-debug {BuildDir}/Linux/{ExeName}.elf
```

### Android
```
cd {BuildProjDir}/Android && ./gradlew assembleRelease
```
Assets copied to `Android/app/src/main/assets/` before build

### GameCube/Wii/3DS
```
make -C {BuildProjDir} -f Makefile_TEMP -j 12
```
Uses DevkitPro toolchain (Makefile_GCN, Makefile_Wii, Makefile_3DS)

---

## Output Paths Summary

### Packaged Directory
```
{ProjectDir}/Packaged/{Platform}/
```

### Build Output by Platform
| Platform | Build Path | Extension |
|----------|-----------|-----------|
| Windows | `Build/Windows/x64/Release/` | `.exe` |
| Linux | `Build/Linux/` | `.elf` |
| Android | `Android/app/build/outputs/apk/release/` | `.apk` |
| GameCube | `Build/GCN/` | `.dol` |
| Wii | `Build/Wii/` | `.dol` |
| N3DS | `Build/3DS/` | `.3dsx` |

### Generated Files
```
{ProjectDir}/Generated/
    ├── EmbeddedAssets.h
    ├── EmbeddedAssets.cpp
    ├── EmbeddedScripts.h
    └── EmbeddedScripts.cpp
```

---

## Example: Full Build Flow Comparison

### Non-Headless (Editor) - Linux Build
```
1. Create Packaged/Linux/
2. Cook assets to .oct files
3. Generate AssetRegistry.txt
4. Generate embedded files (empty if not embedded)
5. Copy scripts to Packaged/Linux/Engine/Scripts/
6. Copy .octp and Config.ini
7. Compile SPIR-V shaders
8. Check for Octave.elf -> EXISTS -> skip compile
9. Copy existing Octave.elf to Packaged/Linux/
10. Rename to {ProjectName}.elf
```

### Headless (CI) - Linux Build
```
1. Create Packaged/Linux/
2. Cook assets to .oct files
3. Generate AssetRegistry.txt
4. Generate embedded files
5. Copy scripts to Packaged/Linux/Engine/Scripts/
6. Copy .octp and Config.ini
7. Compile SPIR-V shaders
8. needCompile = true (always in headless)
9. Run: make -C {dir} -f Makefile_TEMP -j 12
10. Strip debug symbols
11. Copy fresh executable to Packaged/Linux/
```
