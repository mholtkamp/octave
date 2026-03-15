# Octave Templates

Templates are pre-made project starting points that help you quickly begin new projects with a specific setup, assets, and code already in place.

## Using Templates

### Creating a Project from a Template

1. Open the **Project Select** window (appears on startup or via **File > Project Select...**)
2. Click the **Templates** tab
3. Browse available templates and select one
4. Click **Create from Template**
5. Choose a location and name for your new project
6. The template's contents will be copied to your new project

### Installing Templates

Templates can be installed from two sources:

#### From a .zip File

1. In the Templates tab, click **+ Add Template**
2. Click **Browse for .zip...**
3. Select a template .zip file
4. The template will be extracted and added to your library

#### From a GitHub URL

1. In the Templates tab, click **+ Add Template**
2. Enter the GitHub repository URL (e.g., `https://github.com/author/my-template`)
3. Click **Download & Install**
4. The template will be downloaded and added to your library

### Removing Templates

1. Select a template in the Templates tab
2. Click **Remove Template**
3. The template will be removed from your library (does not affect projects already created from it)

---

## Creating Templates

Anyone can create and share templates. A template is simply a project structure packaged with a `package.json` metadata file.

### Template Structure

```
my-template/
    package.json        # Required: Template metadata
    thumbnail.png       # Optional: Preview image (recommended 256x256)
    Assets/             # Your template's assets
        Scenes/
        Textures/
        Materials/
        ...
    Scripts/            # Your template's scripts
        Main.lua
        ...
    MyTemplate.octp     # Project file (will be renamed when used)
```

### package.json Format

Create a `package.json` file in your template's root directory:

```json
{
    "name": "My Awesome Template",
    "author": "Your Name",
    "description": "A brief description of what this template provides and who it's for.",
    "url": "https://github.com/yourname/my-template",
    "version": "1.0.0",
    "updated": "2024-01-15",
    "tags": ["2D", "platformer", "beginner"],
    "type": "lua"
}
```

#### Fields

| Field | Required | Description |
|-------|----------|-------------|
| `name` | Yes | Display name shown in the template browser |
| `author` | Yes | Your name or organization |
| `description` | Yes | What this template includes and its intended use |
| `url` | No | Link to the template's repository or webpage |
| `version` | Yes | Semantic version (e.g., "1.0.0") |
| `updated` | Yes | Last update date (YYYY-MM-DD format) |
| `tags` | No | Array of tags for filtering (e.g., "2D", "3D", "RPG") |
| `type` | Yes | Either "lua" or "cpp" |

### thumbnail.png

Include a `thumbnail.png` image to show a preview of your template:

- **Recommended size:** 256x256 pixels
- **Format:** PNG with transparency supported
- Shows in the template browser grid

### Tips for Good Templates

1. **Include a README** - Add a README.md explaining how to use the template
2. **Keep it focused** - Templates should have a clear purpose
3. **Document assumptions** - Note any required setup or external dependencies
4. **Test thoroughly** - Ensure the template works on a fresh install
5. **Use placeholder names** - Use generic names that users can easily rename

### Project File Handling

When a user creates a project from your template:

1. All files are copied to the new project location
2. The `.octp` file is renamed to match the new project name
3. The project is opened automatically

---

## Distributing Templates

### As a .zip File

1. Zip your template folder (the folder containing package.json)
2. Share the .zip file via download links, email, etc.

### Via GitHub

1. Create a GitHub repository for your template
2. Ensure `package.json` is in the repository root
3. Share the repository URL (e.g., `https://github.com/yourname/my-template`)
4. Users can install directly from the URL

### Template Repository Structure

For GitHub distribution:
```
my-template/           (repository root)
    package.json
    thumbnail.png
    Assets/
    Scripts/
    MyTemplate.octp
    README.md
```

---

## Template Storage Location

Installed templates are stored in:

- **Windows:** `%APPDATA%/OctaveEditor/Templates/`
- **Linux:** `~/.config/OctaveEditor/Templates/`

Each template is stored in its own subdirectory named after its ID (typically the folder name from the zip or repo).

---

## Example: Minimal Template

Here's the simplest possible template:

**package.json:**
```json
{
    "name": "Blank Lua Project",
    "author": "Octave Team",
    "description": "A minimal starting point with just a main scene.",
    "version": "1.0.0",
    "updated": "2024-01-01",
    "type": "lua"
}
```

**Assets/Scenes/SC_Main.oct:** (default scene)

**Scripts/Main.lua:**
```lua
function Start()
    -- Your game starts here
end

function Update(deltaTime)
    -- Called every frame
end
```

**BlankProject.octp:**
```
name=BlankProject
```

---

## Example: Full-Featured Template

**package.json:**
```json
{
    "name": "2D Platformer Starter",
    "author": "Octave Team",
    "description": "A complete 2D platformer foundation with player movement, physics, tilemaps, and sample levels. Perfect for beginners learning game development.",
    "url": "https://github.com/OctaveEngine/template-platformer",
    "version": "2.1.0",
    "updated": "2024-06-15",
    "tags": ["2D", "platformer", "physics", "beginner", "sample"],
    "type": "lua"
}
```

**Directory structure:**
```
2d-platformer-starter/
    package.json
    thumbnail.png
    README.md
    Assets/
        Scenes/
            SC_MainMenu.oct
            SC_Level1.oct
            SC_Level2.oct
        Textures/
            Player/
            Tiles/
            UI/
        Audio/
            Music/
            SFX/
    Scripts/
        Player.lua
        Enemy.lua
        GameManager.lua
        UI/
            MainMenu.lua
            PauseMenu.lua
    PlatformerStarter.octp
```
