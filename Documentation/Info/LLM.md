# LLM Codebase Documentation

## What is `.llm/`?

The `.llm/` directory at the repository root contains a suite of markdown files designed to help AI coding assistants (LLMs) understand and navigate the Octave Engine codebase efficiently.

Without these files, an AI assistant must rediscover the project's architecture, conventions, and file layout from scratch each session — reading dozens of files just to orient itself before it can help with actual work. The `.llm/` docs solve this by providing a pre-built map of the codebase.

## Who is this for?

These docs are written **for AI assistants**, not for human developers (though humans are welcome to read them too). They prioritize the kind of information an LLM needs to be productive: file paths, class names, method signatures, naming conventions, and how subsystems connect to each other.

If you're a human contributor looking for documentation, the rest of the `Documentation/` directory and the Lua API reference are better starting points.

## How to use it

When working with an AI coding assistant on this project, point it at `.llm/Spec.md` first. That file is the navigation hub — it contains:

- A directory map of the project
- A table linking to detailed per-subsystem docs
- Key entry points, singletons, naming conventions, and macros

From there, the AI can follow links to whichever subsystem is relevant to the task at hand.

## File overview

| File | Contents |
|------|----------|
| `Spec.md` | Overview and navigation hub (~120 lines) |
| `HowToKeepUpToDate.md` | Instructions for AIs to maintain these docs |
| `Architecture.md` | Core engine: RTTI, factory, properties, lifecycle |
| `NodeSystem.md` | Node hierarchy, 3D/Widget types, World |
| `NodeGraph.md` | Visual scripting: domains, processor, pins, functions |
| `Editor.md` | Editor panels, undo/redo, preferences, UI hooks |
| `Graphics.md` | Vulkan/GX/C3D backends, renderer, materials |
| `Scripting.md` | Lua integration, binding macros, stub generator |
| `AssetSystem.md` | Asset types, versioning, serialization, loading |
| `Platforms.md` | System API, input, audio, networking |
| `Timeline.md` | Animation tracks, clips, keyframes, TimelinePlayer |
| `Addons.md` | Runtime plugins, editor hooks, native addons |

## Keeping it up to date

When the codebase changes significantly — new subsystems, renamed classes, new asset versions, etc. — the relevant `.llm/` files should be updated too. `HowToKeepUpToDate.md` contains a step-by-step checklist for this, including how to keep `mkdocs.yml` in sync when new documentation pages are added.

AI assistants working on this project are encouraged to update these files as part of their work when they notice something has drifted out of date.
