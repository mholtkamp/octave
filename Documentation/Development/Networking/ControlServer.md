# Controller Server

REST API server for remote control of the Octave editor.

## Overview

The Controller Server provides an HTTP REST API that allows external tools to programmatically control the Octave editor. Use cases include:

- **AI Agents** — Automated scene manipulation and testing
- **Automation Scripts** — Batch operations (import assets, build scenes)
- **External Editors** — Blender bridges, custom level editors
- **CI/CD Pipelines** — Automated testing and packaging

The server is **editor-only** (`#if EDITOR`), **default OFF**, and uses [CrowCpp](https://crowcpp.org/) for HTTP handling. Both Crow and its ASIO dependency are header-only.

## Setup

### Enable via Preferences

1. Open **Preferences** (Edit > Preferences)
2. Select **Network** in the sidebar
3. Check **Enabled (Auto-start)** to start the server on editor launch
4. Set **Port** (default: 7890, range: 1024-65535)
5. Optionally check **Log Requests** to log each request to the Debug Log

### Manual Control

The Network preferences panel provides:
- **Start** / **Stop** / **Restart** buttons for manual control
- **Status indicator** — green "Running" or red "Stopped"
- Port changes take effect on next start/restart

## API Reference

All endpoints accept and return JSON. Set `Content-Type: application/json` for request bodies.

Error responses use the format: `{"error": "description"}`

### Scene Endpoints

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `GET` | `/api/scene` | — | Current scene name, play state, paused state |
| `POST` | `/api/scene/open` | `{"name":"SC_Level1"}` | Open a scene by asset name |
| `POST` | `/api/scene/save` | — | Save the current scene |
| `GET` | `/api/scene/hierarchy` | — | Recursive node tree as JSON |
| `PUT` | `/api/scene/hierarchy` | `{"node":"Cube","parent":"Root","index":0}` | Reparent a node |

### Node Endpoints

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `GET` | `/api/nodes/<name>` | — | Node info (type, transform, visibility) |
| `POST` | `/api/nodes` | `{"type":"Box3D","name":"MyCube","parent":"Root"}` | Create a node |
| `POST` | `/api/nodes/<name>/delete` | — | Delete a node |
| `PUT` | `/api/nodes/<name>/transform` | `{"position":[x,y,z],"rotation":[x,y,z],"scale":[x,y,z]}` | Set full transform |
| `PUT` | `/api/nodes/<name>/move` | `{"position":[x,y,z]}` | Set position |
| `PUT` | `/api/nodes/<name>/rotate` | `{"rotation":[x,y,z]}` | Set rotation (degrees) |
| `PUT` | `/api/nodes/<name>/scale` | `{"scale":[x,y,z]}` | Set scale |
| `PUT` | `/api/nodes/<name>/visibility` | `{"visible":true}` | Show/hide node |

### Property Endpoints

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `GET` | `/api/nodes/<name>/properties` | — | All reflected properties |
| `PUT` | `/api/nodes/<name>/properties` | `{"name":"Speed","value":10.0}` | Set property by name |
| `GET` | `/api/nodes/<name>/script-properties` | — | Script (Lua) fields |
| `PUT` | `/api/nodes/<name>/script-properties` | `{"name":"health","value":100}` | Set script field |

### Play Mode Endpoints

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/api/play/start` | Begin Play-In-Editor |
| `POST` | `/api/play/stop` | End Play-In-Editor |
| `POST` | `/api/play/pause` | Pause play mode |
| `POST` | `/api/play/resume` | Resume play mode |

### Asset Endpoints

| Method | Path | Body | Description |
|--------|------|------|-------------|
| `POST` | `/api/assets/import` | `{"path":"C:/art/model.obj"}` | Import an asset from disk |

### Other Endpoints

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/preferences` | Dump server status |

## Request/Response Format

### Successful Responses

Most endpoints return a JSON object with the result:

```json
// GET /api/scene
{
  "scene": "SC_Level1",
  "playing": false,
  "paused": false
}
```

```json
// GET /api/nodes/MyCube
{
  "name": "MyCube",
  "type": "Box3D",
  "active": true,
  "visible": true,
  "position": [0.0, 1.0, 0.0],
  "rotation": [0.0, 0.0, 0.0],
  "scale": [1.0, 1.0, 1.0]
}
```

### Error Responses

```json
{
  "error": "Node not found: MyCube"
}
```

### Node Hierarchy Response

```json
{
  "name": "Root",
  "type": "Node3D",
  "active": true,
  "visible": true,
  "position": [0, 0, 0],
  "rotation": [0, 0, 0],
  "scale": [1, 1, 1],
  "children": [
    {
      "name": "Camera",
      "type": "Camera3D",
      ...
    },
    {
      "name": "DirectionalLight",
      "type": "DirectionalLight3D",
      ...
    }
  ]
}
```

## Native Addon Hooks

Native addons can register custom REST endpoints via the `EditorUIHooks` system.

### Registering a Custom Route

```c
typedef void (*ControllerRouteCallback)(
    const char* method,
    const char* path,
    const char* body,
    char* responseBuffer,
    int32_t bufferSize,
    void* userData
);

// In your addon's OnLoad:
api->editorUI->RegisterControllerRoute(
    hookId, "GET", "/api/addons/mycustom",
    MyRouteHandler, nullptr
);
```

### Server State Events

```c
typedef void (*ControllerServerEventCallback)(int32_t state, void* userData);
// state: 0=Started, 1=Stopped

api->editorUI->RegisterOnControllerServerStateChanged(
    hookId, MyStateCallback, nullptr
);
```

### Complete Addon Example

```c
void MyRouteHandler(const char* method, const char* path, const char* body,
                    char* responseBuffer, int32_t bufferSize, void* userData) {
    snprintf(responseBuffer, bufferSize,
        "{\"addon\":\"mycompany.myaddon\",\"status\":\"ok\"}");
}

void MyStateCallback(int32_t state, void* userData) {
    if (state == 0) printf("Controller server started!\n");
    if (state == 1) printf("Controller server stopped!\n");
}

int OnLoad(OctaveEngineAPI* api) {
    HookId hookId = GenerateHookId("com.mycompany.myaddon");
    api->editorUI->RegisterControllerRoute(
        hookId, "GET", "/api/addons/mydata", MyRouteHandler, nullptr);
    api->editorUI->RegisterOnControllerServerStateChanged(
        hookId, MyStateCallback, nullptr);
    return 0;
}
```

## Examples

### curl Examples

```bash
# Get current scene info
curl http://localhost:7890/api/scene

# Open a scene
curl -X POST http://localhost:7890/api/scene/open \
  -H "Content-Type: application/json" \
  -d '{"name":"SC_Level1"}'

# Get scene hierarchy
curl http://localhost:7890/api/scene/hierarchy

# Create a node
curl -X POST http://localhost:7890/api/nodes \
  -H "Content-Type: application/json" \
  -d '{"type":"Box3D","name":"MyCube","parent":"Root"}'

# Move a node
curl -X PUT http://localhost:7890/api/nodes/MyCube/move \
  -H "Content-Type: application/json" \
  -d '{"position":[5.0, 2.0, 0.0]}'

# Start play mode
curl -X POST http://localhost:7890/api/play/start

# Stop play mode
curl -X POST http://localhost:7890/api/play/stop

# Get node properties
curl http://localhost:7890/api/nodes/MyCube/properties

# Set a property
curl -X PUT http://localhost:7890/api/nodes/MyCube/properties \
  -H "Content-Type: application/json" \
  -d '{"name":"Visible","value":false}'

# Import an asset
curl -X POST http://localhost:7890/api/assets/import \
  -H "Content-Type: application/json" \
  -d '{"path":"C:/art/model.obj"}'
```

### Python Script Example

```python
import requests
import json

BASE = "http://localhost:7890"

# Get scene info
scene = requests.get(f"{BASE}/api/scene").json()
print(f"Scene: {scene['scene']}, Playing: {scene['playing']}")

# Create some nodes
for i in range(5):
    requests.post(f"{BASE}/api/nodes", json={
        "type": "Box3D",
        "name": f"Box_{i}",
        "parent": "Root"
    })

# Position them in a row
for i in range(5):
    requests.put(f"{BASE}/api/nodes/Box_{i}/move", json={
        "position": [i * 2.0, 0.0, 0.0]
    })

# Get the hierarchy
hierarchy = requests.get(f"{BASE}/api/scene/hierarchy").json()
print(json.dumps(hierarchy, indent=2))
```
