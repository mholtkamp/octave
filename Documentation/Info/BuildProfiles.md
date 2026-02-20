# Build Profiles

Build Profiles let you save packaging configurations for different platforms and quickly build your game with one click.

## Opening the Packaging Window

Go to **File > Packaging...** to open the Packaging window.

## Creating a Build Profile

1. Click the **+** button in the Build Profiles list
2. Give your profile a name (e.g., "GameCube Release")
3. Configure the settings:
   - **Platform** - Select your target platform (Windows, Linux, GameCube, Wii, 3DS, etc.)
   - **Embedded Mode** - Embeds assets into the executable (required for console platforms)
   - **Output Directory** - Optional custom location for built files (leave empty for default)
   - **Use Docker** - Forces Docker-based builds (see Docker section below)

## Building Your Game

1. Select a profile from the list
2. Click **Build** to package your game
3. Click **Build & Run** to package and automatically launch in an emulator

## Build & Run Setup

To use Build & Run, you need to configure your emulator paths:

1. Click the gear icon next to "Build & Run"
2. This opens **Preferences > External > Launchers**
3. Set the paths for your emulators:

| Platform | Emulator |
|----------|----------|
| GameCube | Dolphin |
| Wii | Dolphin |
| 3DS | Azahar or Citra |

### Custom Launch Arguments

You can customize how games launch using placeholders:

- `{emulator}` - The emulator executable path
- `{output}` - The built game file path
- `{outputdir}` - The output directory

**Examples:**
- Dolphin fullscreen: `{emulator} -e {output} -f`
- Dolphin with debugger: `{emulator} -e {output} -d`

### 3DS Hardware (3dslink)

To send games directly to your 3DS:

1. Set the **3dslink Path** in Preferences
2. Select **Hardware** under "3DS Launch Method"
3. When you Build & Run, ensure your 3DS has Homebrew Launcher open and ready

## Docker Builds
### Requirements

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) installed and running
- The `octavegameengine` Docker image built (see Docker.md)

### When Docker is Used

| Host OS | Platform | Docker Required |
|---------|----------|-----------------|
| Windows | GameCube | Yes (automatic) |
| Windows | Wii | Yes (automatic) |
| Windows | 3DS | Yes (automatic) |
| Windows | Windows | No |
| Linux | Any | Optional (checkbox) |

### Troubleshooting

**"Docker Desktop is not running"**
- Open Docker Desktop and wait for it to start
- Try the build again

**"Docker image not found"**
- Build the Docker image first (see Docker.md)
- Try `wsl docker ...`

## Profile Storage

Build profiles are saved per-project in `{ProjectDir}/Settings/BuildProfiles.json` and travel with your project.

Emulator settings are saved in your user preferences and persist across all projects.
