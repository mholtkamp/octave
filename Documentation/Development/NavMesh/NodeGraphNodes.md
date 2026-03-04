# NavMesh NodeGraph Nodes

All NavMesh nodes are in the **"NavMesh"** category with a teal-green color `(0.2, 0.7, 0.5)`.

## Pure Data Nodes

These nodes query the navigation mesh and return results. They do not require execution flow pins.

### Find Nav Path

Finds a path between two points on the navigation mesh and outputs it as a PointCloud.

| Pin | Direction | Type | Default | Description |
|-----|-----------|------|---------|-------------|
| Start | Input | Vector | (0,0,0) | Starting position |
| Goal | Input | Vector | (0,0,0) | Goal position |
| Path | Output | PointCloud | — | Waypoints as a point cloud (positions = waypoints) |
| Found | Output | Bool | — | Whether a valid path was found |
| Waypoint Count | Output | Integer | — | Number of waypoints in the path |

The Path output is a `PointCloud` where each point's position is a waypoint. This integrates with Point nodes like For Each Point, Get Point Position, and Get Point Count.

### Find Random Nav Point

Returns a random walkable position on the navigation mesh.

| Pin | Direction | Type | Description |
|-----|-----------|------|-------------|
| Point | Output | Vector | Random point on the nav mesh |
| Found | Output | Bool | Whether a valid point was found |

### Find Closest Nav Point

Snaps an arbitrary position to the nearest walkable point on the navigation mesh.

| Pin | Direction | Type | Default | Description |
|-----|-----------|------|---------|-------------|
| Position | Input | Vector | (0,0,0) | Query position |
| Point | Output | Vector | — | Nearest point on nav mesh |
| Found | Output | Bool | — | Whether a nearest point was found |

### Is Auto Nav Rebuild

Returns whether automatic nav mesh rebuilding is currently enabled.

| Pin | Direction | Type | Description |
|-----|-----------|------|-------------|
| Enabled | Output | Bool | Auto-rebuild state |

## Flow Nodes

These nodes perform actions and require execution flow connections.

### Build Nav Data

Triggers a full navigation mesh rebuild. Connect to Start or after scene setup.

| Pin | Direction | Type | Description |
|-----|-----------|------|-------------|
| Exec | Input | Execution | Trigger |
| Exec | Output | Execution | Continues after build |

### Enable Auto Nav Rebuild

Enables or disables automatic nav mesh rebuilding.

| Pin | Direction | Type | Default | Description |
|-----|-----------|------|---------|-------------|
| Exec | Input | Execution | — | Trigger |
| Enable | Input | Bool | true | Whether to enable auto-rebuild |
| Exec | Output | Execution | — | Continues after setting |

### Invalidate Nav Mesh

Marks the navigation mesh as dirty. If auto-rebuild is enabled, triggers a rebuild.

| Pin | Direction | Type | Description |
|-----|-----------|------|-------------|
| Exec | Input | Execution | Trigger |
| Exec | Output | Execution | Continues after invalidation |

## Example Workflows

### Basic Pathfinding Setup

```
[Start] → [Build Nav Data] → [Enable Auto Nav Rebuild (true)]
```

Then in a Tick or event:

```
[Find Nav Path (Start=EnemyPos, Goal=PlayerPos)] → Path → [For Each Point] → [Get Point Position] → move logic
```

### Dynamic Obstacle Handling

```
[On Obstacle Moved] → [Invalidate Nav Mesh]
```

With auto-rebuild enabled, this automatically triggers a nav mesh rebuild.

### Random Patrol

```
[Timer Event] → [Find Random Nav Point] → Found? → [Branch] → [Navigate To Point]
```
