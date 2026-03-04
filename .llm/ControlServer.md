# Controller Server

## Overview
REST server for editor remote control. Editor-only (`#if EDITOR`). Uses CrowCpp (header-only HTTP framework) with standalone ASIO. Default OFF, controlled via Preferences > Network.

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Source/Editor/ControllerServer/ControllerServerTypes.h` | `ControllerCommand` struct (function + promise) |
| `Engine/Source/Editor/ControllerServer/ControllerServer.h` | Singleton class: Create/Destroy/Get/Start/Stop/Restart/Tick/QueueCommand |
| `Engine/Source/Editor/ControllerServer/ControllerServer.cpp` | Server lifecycle, Crow app management, command queue drain |
| `Engine/Source/Editor/ControllerServer/ControllerServerRoutes.h` | `RegisterRoutes()` declaration |
| `Engine/Source/Editor/ControllerServer/ControllerServerRoutes.cpp` | All endpoint handlers, JSON helpers (NodeToJson, HierarchyToJson, etc.) |
| `Engine/Source/Editor/Preferences/Network/NetworkModule.h` | Preferences module: port, enabled, log requests |
| `Engine/Source/Editor/Preferences/Network/NetworkModule.cpp` | UI rendering, settings persistence, server control buttons |
| `External/asio/` | Standalone ASIO headers (header-only) |
| `External/Crow/` | CrowCpp headers (header-only) |

## Architecture

```
HTTP Client (curl/Python/Agent)
        |
        v
  Crow I/O Thread (run_async)
        |
        v
  QueueCommand(lambda)  -->  std::queue<ControllerCommand>
        |                           |
   future.get()                     | (main thread)
        |                           v
        |                    Tick() drains queue
        |                    executes lambda on main thread
        |                           |
        v                           v
  Response returned          Engine APIs called safely
```

## Preferences (NetworkModule)

| Setting | Type | Default | JSON Key |
|---------|------|---------|----------|
| Controller Server Enabled | bool | false | `controllerServerEnabled` |
| Port | int | 7890 | `port` |
| Log Requests | bool | false | `logRequests` |

Singleton: `NetworkModule::Get()`. Registered as root module in `PreferencesManager::Create()`.

## REST Endpoints

### Scene
| Method | Path | Action |
|--------|------|--------|
| GET | `/api/scene` | Scene name + play/paused state |
| POST | `/api/scene/open` | Open scene by name |
| POST | `/api/scene/save` | Save current scene |
| GET | `/api/scene/hierarchy` | Recursive node tree JSON |
| PUT | `/api/scene/hierarchy` | Reparent node |

### Nodes
| Method | Path | Action |
|--------|------|--------|
| GET | `/api/nodes/<name>` | Node info (type, transform, visibility) |
| POST | `/api/nodes` | Create node by type name |
| POST | `/api/nodes/<name>/delete` | Delete node |
| PUT | `/api/nodes/<name>/transform` | Set pos/rot/scale |
| PUT | `/api/nodes/<name>/move` | Set position |
| PUT | `/api/nodes/<name>/rotate` | Set rotation (degrees) |
| PUT | `/api/nodes/<name>/scale` | Set scale |
| PUT | `/api/nodes/<name>/visibility` | Show/hide |

### Properties
| Method | Path | Action |
|--------|------|--------|
| GET | `/api/nodes/<name>/properties` | All reflected properties |
| PUT | `/api/nodes/<name>/properties` | Set property by name |
| GET | `/api/nodes/<name>/script-properties` | Script fields |
| PUT | `/api/nodes/<name>/script-properties` | Set script field |

### Play Mode
| POST | `/api/play/start` | BeginPlayInEditor |
| POST | `/api/play/stop` | EndPlayInEditor |
| POST | `/api/play/pause` | Pause |
| POST | `/api/play/resume` | Resume |

### Assets
| POST | `/api/assets/import` | Import asset from disk path |

## Thread Safety Model

1. Crow runs on its own I/O thread via `run_async()`
2. Route handlers receive requests on Crow's thread
3. Handlers call `QueueCommand(lambda)` — lambda captures request data, returns JSON string
4. `QueueCommand` packages lambda + `std::promise` into queue, returns `std::future`
5. Handler calls `future.get()` to block until main thread executes
6. `Tick()` on main thread drains queue, executes lambdas, fulfills promises
7. All engine API calls happen only on main thread

## Native Addon Hooks

Added to `EditorUIHooks.h`:
```c
typedef void (*ControllerRouteCallback)(const char* method, const char* path, const char* body, char* responseBuffer, int32_t bufferSize, void* userData);
typedef void (*ControllerServerEventCallback)(int32_t state, void* userData);
```

Function pointers in `EditorUIHooks` struct:
- `RegisterControllerRoute(hookId, method, path, callback, userData)`
- `UnregisterControllerRoute(hookId, path)`
- `RegisterOnControllerServerStateChanged(hookId, callback, userData)`

Storage in `EditorUIHookManager`: `mControllerRoutes`, `mOnControllerServerStateChanged`

Addon routes handled via `CROW_CATCHALL_ROUTE` — unmatched requests checked against registered addon routes.

## EditorImgui Integration

- `EditorImguiInit()`: `ControllerServer::Create()`, auto-start if enabled
- `EditorImguiDraw()`: `ControllerServer::Get()->Tick()` before `ImGui::Render()`
- `EditorImguiPreShutdown()`: `ControllerServer::Destroy()`

## Adding New Endpoints

1. Add route in `ControllerServerRoutes.cpp` using `CROW_ROUTE(app, "/api/...")`
2. Wrap engine calls in `server->QueueCommand(lambda)` for thread safety
3. Use `crow::json::wvalue` to build response JSON
4. Call `future.get()` and return `crow::response(200, "application/json", result)`
5. Add `LogRequest()` call at handler start for request logging support
