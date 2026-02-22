# How to Keep LLM Documentation Up to Date

This guide tells AI assistants when and how to update the `.llm/` docs, `Documentation/` markdown, and `mkdocs.yml`.

## When to Update

Update after any of these changes:

- [ ] **New files/classes added** to a subsystem
- [ ] **Subsystem architecture changed** (new patterns, renamed classes, restructured directories)
- [ ] **New subsystem created** (add a new `.llm/*.md` file and link it from `Spec.md`)
- [ ] **New asset version** bumped (update `AssetSystem.md` version table)
- [ ] **New node graph domain** added (update `NodeGraph.md`)
- [ ] **New editor panel/tool** added (update `Editor.md`)
- [ ] **New platform support** added (update `Platforms.md`)
- [ ] **New documentation pages** added to `Documentation/` (update `mkdocs.yml` nav)
- [ ] **Build configuration changed** (update `Spec.md` build section)

## What to Update

### 1. `.llm/` Files

Each file covers one subsystem. When modifying a subsystem:

1. Open the relevant `.llm/*.md` file
2. Update class names, file paths, method signatures, or descriptions
3. If a new subsystem is created, create a new `.llm/NewSubsystem.md` and add it to the table in `Spec.md`
4. Keep `Spec.md` under ~300 lines — it's the navigation hub, not the detail

### 2. `Documentation/` Markdown Files

The `Documentation/` directory contains user-facing docs served via MkDocs. When adding new documentation pages:

1. Create the `.md` file in the appropriate `Documentation/` subdirectory
2. Follow existing structure: `Development/` for dev guides, `Lua/` for Lua API, `Info/` for general info
3. Update `mkdocs.yml` nav (see below)

### 3. `mkdocs.yml` Nav Updates

The `mkdocs.yml` at the repository root defines the documentation site navigation. Its current top-level structure is:

```yaml
nav:
  - Home: index.md
  - Info:
      - Editor: Info/Editor.md
      - Scripting: Info/Scripting.md
      - Templates: Info/Templates.md
      - Addons: Info/Addons.md
      - Build Profiles: Info/BuildProfiles.md
      - Docker: Info/Docker.md
  - Development:
      - Mesh Instancing: Development/MeshInstancing.md
      - Packaging Flow: Development/PackagingFlow.md
      - Themes / CSS: Development/ThemesCss.md
      - 3DS: ...
      - UI: ...
      - Input: ...
      - Logging: ...
      - Node Graph: ...
      - Timeline: ...
      - Native Addons: ...
  - Lua API:
      - Overview: Lua/README.md
      - Systems: ...
      - Assets: ...
      - Misc: ...
      - Nodes: ...
  - Widgets: ...
  - API Reference: ...
```

**Rules for updating `mkdocs.yml`:**

1. **Match the existing hierarchy.** Place new pages under the correct section (Info, Development, Lua API, etc.)
2. **Use consistent formatting.** Each entry is `- Display Name: path/relative/to/Documentation/dir.md`
3. **Alphabetize within sections** where the existing entries are alphabetized
4. **Subsection nesting.** If a new subsection has multiple pages, create a nested structure like the existing `Node Graph:` or `Timeline:` sections
5. **Verify the file exists** in `Documentation/` before adding to nav
6. **Test locally** with `mkdocs serve` if possible

## Checklist for Common Changes

### Added a new C++ class to an existing subsystem
- [ ] Update the relevant `.llm/*.md` with class name, file path, key methods
- [ ] No `mkdocs.yml` change needed unless new user-facing docs were written

### Created a new engine subsystem
- [ ] Create `.llm/NewSubsystem.md` with purpose, key files, classes, APIs, patterns
- [ ] Add row to the subsystem table in `.llm/Spec.md`
- [ ] Link the new file from `Spec.md`
- [ ] If user-facing docs exist, add to `mkdocs.yml`

### Bumped asset version
- [ ] Update `ASSET_VERSION_CURRENT` value in `.llm/AssetSystem.md`
- [ ] Add new version to the version history table with description of changes

### Added a new editor panel
- [ ] Add panel to the panels list in `.llm/Editor.md`
- [ ] Note its file path, draw function, and purpose

### Added a new Lua binding
- [ ] Update binding count and list in `.llm/Scripting.md`
- [ ] If new Lua API docs written in `Documentation/Lua/`, add to `mkdocs.yml` nav under `Lua API`

### Added new documentation pages
- [ ] Add entry to `mkdocs.yml` nav in the correct section
- [ ] Verify relative path from `Documentation/` directory
