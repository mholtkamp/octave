# OctaveGameEngine Docker Build System

One Docker container with all toolchains for building Octave Game Engine on any supported platform.

## Quick Start

```bash
# 1. Build the Docker image (from project root)
./Docker/build.sh

# 2. Build the Editor
docker run --rm -v ./output:/game octavegameengine build-editor

# 3. Build a game (pass your project directory)
docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux
```

## Available Build Commands

| Command | Platform | Project |
|---------|----------|---------|
| `build-editor` | Linux Editor | Optional (recommended) |
| `build-linux` | Linux Game | Required |
| `build-3ds` | Nintendo 3DS | Required |
| `build-gcn` | Nintendo GameCube | Required |
| `build-wii` | Nintendo Wii | Required |

## Volumes

| Volume | Purpose |
|--------|---------|
| `/game` | Output directory - where build artifacts are copied |
| `/project` | Your game project directory (required for game builds) |

## Project Structure

Your game project should be structured like:

```
MyGame/
├── MyGame.oct      # Project file
├── Assets/         # Game assets (textures, models, sounds, etc.)
└── Scripts/        # Lua scripts (optional)
```

## Usage Examples

### Build the Docker Image

```bash
# Using the helper script
./Docker/build.sh

# Or manually
docker build -f Docker/Dockerfile -t octavegameengine .
```

### Build the Editor

The editor can optionally include your project files. Pass your project directory to have it copied into the output.

```bash
# With project (recommended)
docker run --rm -v ./dist/Octave:/game -v ./MyGame:/project octavegameengine build-editor

# Without project
docker run --rm -v ./dist/Octave:/game octavegameengine build-editor
```

Output:
```
dist/Octave/
├── Engine/
├── Standalone/
├── External/
├── ...
└── OctaveEditor.elf    ← Ready to run
```

### Build a Game

Game builds require your project directory mounted at `/project`.

```bash
# Linux
docker run --rm -v ./dist:/game -v ./MyGame:/project octavegameengine build-linux

# Nintendo 3DS
docker run --rm -v ./dist:/game -v ./MyGame:/project octavegameengine build-3ds

# Nintendo GameCube
docker run --rm -v ./dist:/game -v ./MyGame:/project octavegameengine build-gcn

# Nintendo Wii
docker run --rm -v ./dist:/game -v ./MyGame:/project octavegameengine build-wii
```

Output contains only the executable/ROM files:
```
# Linux
dist/Linux/
└── Octave.elf

# GameCube / Wii
dist/GCN/
├── Octave.dol
└── Octave.elf

# 3DS
dist/3DS/
├── Octave.3dsx
└── Octave.smdh
```

### Interactive Shell

```bash
docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine bash
```

### Show Help

```bash
docker run --rm octavegameengine help
```

## Output Directory

You should create the `dist` directory or whatever you want to export to beforehand or else the directory will be created by Docker and you will have to `sudo chmod -R 777 ./dist` to change permissions so you can access it.
 or do a `sudo rm -rf ./dist` to delete the directory.
## VS Code Integration

Use the task "Docker Build Editor - Linux" from Command Palette (`Ctrl+Shift+P` → "Tasks: Run Task").

## Included Toolchains

- **Vulkan SDK 1.3.275.0** - For Linux builds
- **devkitPPC** - For GameCube and Wii
- **devkitARM** - For Nintendo 3DS

## Requirements

- Docker
- ~5GB disk space for the image
- Internet connection (first build only)

## Troubleshooting

### "No project mounted at /project"

Game builds require your project directory. Mount it with `-v`:

```bash
docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux
```

### Permission denied when deleting output

Use Docker to clean:

```bash
docker run --rm -v ./output:/game alpine rm -rf /game/*
```

### devkitPro installation fails

Try rebuilding without cache:

```bash
docker build --no-cache -f Docker/Dockerfile -t octavegameengine .
```
