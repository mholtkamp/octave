# Octave Addons

Addons are content packs that add assets, scripts, and functionality to your existing projects. Unlike templates (which create new projects), addons merge their contents into a project you're already working on.

## Using Addons

### Opening the Addons Window

Open the Addons window via **File > Addons...** or the toolbar.

### Browsing Addons

1. Open the **Addons** window
2. The **Browse Addons** tab shows all available addons from your configured repositories
3. Use the search bar to filter by name
4. Click tag buttons to filter by category
5. Each addon card shows:
   - Thumbnail preview
   - Name and author
   - Download or "Installed" status
   - **View More** button for details

### Installing an Addon

1. Find the addon you want in the Browse tab
2. Click **Download** on the addon card, or click **View More** and then **Download**
3. The addon will be downloaded and its contents merged into your current project
4. Assets and scripts from the addon are copied to your project's folders

### Managing Installed Addons

1. Go to the **Installed** tab
2. View all addons installed in your current project
3. Click **Update** if a newer version is available
4. Click **Uninstall** to remove an addon (note: files already in your project may remain)

### Adding Repositories

1. Go to the **Repositories** tab
2. Click **+ Add Repo**
3. Enter a GitHub URL that hosts addons
4. Click **Add**
5. The repository's addons will appear in your Browse tab

---

## Creating Addons

Addons have the same structure as templates, but are designed to be merged into existing projects rather than creating new ones.

### Addon Structure

```
my-addon/
    package.json        # Required: Addon metadata
    thumbnail.png       # Optional: Preview image (recommended 256x256)
    Assets/             # Assets to add to the project
        Textures/
        Materials/
        Prefabs/
        ...
    Scripts/            # Scripts to add to the project
        MyFeature.lua
        ...
```

### package.json Format

Create a `package.json` file in your addon's root directory:

```json
{
    "name": "Particle Effects Pack",
    "author": "Your Name",
    "description": "A collection of 50+ particle effects for explosions, magic, weather, and more.",
    "url": "https://github.com/yourname/particle-effects",
    "version": "1.0.0",
    "updated": "2024-01-15",
    "tags": ["effects", "particles", "visual", "2D", "3D"]
}
```

#### Fields

| Field | Required | Description |
|-------|----------|-------------|
| `name` | Yes | Display name shown in the addon browser |
| `author` | Yes | Your name or organization |
| `description` | Yes | What this addon provides |
| `url` | No | Link to the addon's repository or webpage |
| `version` | Yes | Semantic version (e.g., "1.0.0") |
| `updated` | Yes | Last update date (YYYY-MM-DD format) |
| `tags` | No | Array of tags for filtering |

### Key Differences from Templates

| Aspect | Templates | Addons |
|--------|-----------|--------|
| Purpose | Create new projects | Add to existing projects |
| Project files | Includes .octp | No .octp file |
| Installation | Copies all files | Merges, skips existing files |
| Use case | Starting points | Feature packs, asset collections |

### What Gets Installed

When a user installs your addon:

1. **Assets/** folder contents are copied to the project's Assets folder
2. **Scripts/** folder contents are copied to the project's Scripts folder
3. **Files that already exist are NOT overwritten** (prevents accidental data loss)
4. **package.json and thumbnail.png are NOT copied** (metadata only)
5. **No .octp or .ini files** should be included (they would be skipped anyway)

### thumbnail.png

Include a `thumbnail.png` image:

- **Recommended size:** 256x256 pixels
- **Format:** PNG with transparency supported
- Shows in the addon browser grid

---

## Creating Addon Repositories

A repository is a GitHub repo that lists multiple addons. This allows you to share a collection of addons from a single URL.

### Repository Structure

```
my-addon-repo/           (repository root)
    package.json         # Required: Lists all addons
    particle-effects/    # Addon 1
        package.json
        thumbnail.png
        Assets/
        Scripts/
    ui-kit/              # Addon 2
        package.json
        thumbnail.png
        Assets/
        Scripts/
    audio-manager/       # Addon 3
        package.json
        thumbnail.png
        Assets/
        Scripts/
```

### Repository package.json

The root `package.json` lists all available addons:

```json
{
    "name": "My Addon Collection",
    "addons": [
        "particle-effects",
        "ui-kit",
        "audio-manager"
    ]
}
```

Each addon name in the `addons` array must match a subdirectory name in the repository.

### How It Works

When a user adds your repository URL:

1. The editor fetches the root `package.json`
2. It reads the `addons` array to discover available addons
3. For each addon, it fetches that addon's `package.json` for metadata
4. All addons appear in the user's Browse tab

---

## Distributing Addons

### Single Addon via GitHub

1. Create a GitHub repository for your addon
2. Ensure `package.json` is in the repository root
3. Share the URL (users add it as a repository with one addon)

### Addon Collection via GitHub

1. Create a GitHub repository with the structure shown above
2. Include a root `package.json` listing all addon directories
3. Share the repository URL
4. Users get access to all your addons from one repository

### Example Repository URLs

- Single addon: `https://github.com/yourname/particle-effects`
- Addon collection: `https://github.com/yourname/octave-addons`

---

## Addon Storage

### Cache Location

Downloaded addons are cached in:

- **Windows:** `%APPDATA%/OctaveEditor/AddonCache/`
- **Linux:** `~/.config/OctaveEditor/AddonCache/`

### Installed Addon Tracking

Each project tracks its installed addons in:

```
{ProjectDir}/Settings/installed_addons.json
```

This file records:
- Which addons are installed
- What version was installed
- When they were installed
- Source repository URL

---

## Example: Simple Asset Pack

**package.json:**
```json
{
    "name": "Fantasy UI Elements",
    "author": "Octave Team",
    "description": "A collection of medieval/fantasy themed UI elements including buttons, frames, icons, and fonts.",
    "version": "1.0.0",
    "updated": "2024-01-15",
    "tags": ["UI", "fantasy", "medieval", "2D"]
}
```

**Directory structure:**
```
fantasy-ui/
    package.json
    thumbnail.png
    Assets/
        Textures/
            UI/
                btn_normal.png
                btn_hover.png
                btn_pressed.png
                frame_gold.png
                frame_silver.png
                icons/
                    sword.png
                    shield.png
                    potion.png
        Fonts/
            medieval.ttf
```

---

## Example: Script Addon

**package.json:**
```json
{
    "name": "Save System",
    "author": "Octave Team",
    "description": "A flexible save/load system supporting multiple save slots, auto-save, and cloud sync hooks.",
    "url": "https://github.com/OctaveEngine/addon-save-system",
    "version": "2.0.0",
    "updated": "2024-03-20",
    "tags": ["utility", "save", "persistence"]
}
```

**Directory structure:**
```
save-system/
    package.json
    thumbnail.png
    README.md
    Scripts/
        SaveSystem/
            SaveManager.lua
            SaveSlot.lua
            Serializer.lua
            CloudSync.lua
```

---

## Example: Full Addon Repository

**Root package.json:**
```json
{
    "name": "Official Octave Addons",
    "addons": [
        "particle-effects",
        "ui-kit",
        "save-system",
        "audio-manager",
        "dialogue-system"
    ]
}
```

**Repository structure:**
```
octave-official-addons/
    package.json
    README.md
    particle-effects/
        package.json
        thumbnail.png
        Assets/
        Scripts/
    ui-kit/
        package.json
        thumbnail.png
        Assets/
    save-system/
        package.json
        thumbnail.png
        Scripts/
    audio-manager/
        package.json
        thumbnail.png
        Scripts/
    dialogue-system/
        package.json
        thumbnail.png
        Assets/
        Scripts/
```

---

## Best Practices

### For Addon Creators

1. **Avoid conflicts** - Use unique prefixes for your assets and scripts (e.g., `MyAddon_PlayerController.lua`)
2. **Document dependencies** - If your addon requires other addons, mention it in the description
3. **Include a README** - Explain how to use your addon's features
4. **Version properly** - Use semantic versioning and update the version when making changes
5. **Test in fresh projects** - Ensure your addon works when merged into various project types

### For Repository Maintainers

1. **Organize by category** - Group related addons together
2. **Keep descriptions accurate** - Help users find what they need
3. **Update regularly** - Remove broken addons, update versions
4. **Use clear naming** - Addon directory names become their IDs

### For Users

1. **Review before installing** - Check what files an addon will add
2. **Backup your project** - Before installing large addons
3. **Check for updates** - Newer versions may have bug fixes
4. **Report issues** - Help addon creators improve their work
