# Mesh Instancing in Octave Engine

## Overview

Octave Engine supports mesh instancing through two distinct mechanisms:

1. **Asset-level sharing** - Multiple scene nodes reference a single `StaticMesh` asset in memory (always active, all platforms).
2. **GPU hardware instancing** - A single draw call renders many copies of the same mesh with per-instance transforms (Vulkan only, via `InstancedMesh3D`).

On platforms that lack hardware instancing support (GameCube, Wii, 3DS), the engine uses an **unrolling** fallback that merges instance geometry into batched static meshes at load time.

---

## Asset Sharing (All Platforms)

Every `StaticMesh` is a shared asset managed by the `AssetManager`. When you place the same mesh 100 times using individual `StaticMesh3D` nodes, the mesh data is loaded into memory **once**. Each node holds an `AssetRef` (a lightweight smart-pointer wrapper defined in `Engine/Source/Engine/AssetRef.h`) that points back to the single shared `StaticMesh` asset.

**What this means in practice:**
- Importing one building mesh and placing it 50 times does **not** duplicate the vertex/index data 50 times in RAM or VRAM.
- The GPU-side mesh resource (`StaticMeshResource`) is created once and bound for each draw call that uses it.
- This is a clear win over importing 50 copies of the geometry from Blender/Maya as a single merged blob, because the single-asset approach uses far less memory.

**However**, each individual `StaticMesh3D` node still generates its **own draw call**. There is no automatic cross-node draw call batching - 50 separate `StaticMesh3D` nodes using the same mesh = 50 separate `vkCmdDrawIndexed` calls (each with `instanceCount = 1`). On modern desktop GPUs this is usually fine, but on constrained hardware the draw call overhead adds up.

---

## InstancedMesh3D Node (Hardware Instancing)

`InstancedMesh3D` (defined in `Engine/Source/Engine/Nodes/3D/InstancedMesh3d.h`) is a specialized node designed for rendering many copies of the same mesh efficiently. It extends `StaticMesh3D` and adds:

- A vector of `MeshInstanceData`, each containing per-instance `position`, `rotation`, and `scale`.
- An `InstancedMeshCompResource` that holds GPU-side instance data buffers.
- Support for per-instance vertex colors (baked lighting).

### How It Works

Instead of creating 100 separate nodes, you create **one** `InstancedMesh3D` node, assign it a `StaticMesh`, and populate its instance data array:

```
MeshInstanceData data;
data.mPosition = {10.0f, 0.0f, 5.0f};
data.mRotation = {0.0f, 45.0f, 0.0f};
data.mScale = {1.0f, 1.0f, 1.0f};
instancedMesh->AddInstanceData(data);
```

On Vulkan, this results in a **single** `vkCmdDrawIndexed` call with `instanceCount` set to the number of instances. The GPU vertex shader reads per-instance transform data from a storage buffer to position each copy. This is true hardware instancing - the GPU does all the heavy lifting.

### Per-Instance Colors

`InstancedMesh3D` supports baked per-instance vertex colors for static lighting. The vertex type is selected automatically based on what data is available:

| Mesh Has Vertex Color | Instance Colors Present | Vertex Type Used |
|---|---|---|
| No | No | `Vertex` |
| Yes | No | `VertexColor` |
| No | Yes | `VertexInstanceColor` |
| Yes | Yes | `VertexColorInstanceColor` |

---

## Platform-Specific Behavior

### Vulkan (Windows / Linux / Android)

- **Full hardware instancing support.**
- `InstancedMesh3D` renders all instances in a single `vkCmdDrawIndexed` call with `instanceCount = N`.
- Per-instance transforms are uploaded to a storage buffer (`mInstanceDataBuffer`) and read by the vertex shader.
- The instance data buffer is rebuilt when marked dirty (instances added/removed/modified).
- `ShouldUnroll()` returns `false` on Windows/Linux/Android - instances stay as true GPU instances.

**Relevant code:** `Engine/Source/Graphics/Vulkan/VulkanUtils.cpp` - `DrawInstancedMeshComp()` (line ~1905).

### GameCube / Wii (GX)

- **No hardware instancing support.**
- `GFX_DrawInstancedMeshComp()` is an **empty stub** in `Engine/Source/Graphics/GX/Graphics_GX.cpp` (line ~842).
- `ShouldUnroll()` returns `true` for these platforms (any platform that isn't Windows/Linux/Android).
- On load, `InstancedMesh3D::Unroll()` is called automatically, which **merges instance geometry into batched static meshes**.

**How unrolling works:**

1. The engine calculates a spatial grid over all instance positions using `mUnrolledCellSize` (default 25 units).
2. Instances are bucketed into grid cells based on their XZ position.
3. For each non-empty cell, vertex data from the source mesh is duplicated and pre-transformed by each instance's transform (position, rotation, scale baked into vertex positions and normals).
4. A new transient `StaticMesh` asset is created per cell containing the merged geometry.
5. A child `StaticMesh3D` node is spawned for each cell, with optional `mUnrolledCullDistance` for distance-based culling.
6. The parent `InstancedMesh3D` stops rendering itself (`mUnrolled = true` skips the `Render()` call).

**Implication:** On GCN/Wii, instancing saves you from manually merging geometry in your DCC tool, but at runtime the geometry **is** duplicated per instance in VRAM (just like importing a merged scene from Blender). The benefit is spatial cell-based culling and simplified authoring - the engine handles the merge at load time, and cells outside the cull distance can be skipped entirely.

**Limitation:** Unrolling does not support meshes with vertex colors.

**Relevant code:** `Engine/Source/Engine/Nodes/3D/InstancedMesh3d.cpp` - `Unroll()` (line ~455).

### 3DS (Citro3D)

- **No hardware instancing support.**
- `GFX_DrawInstancedMeshComp()` is an **empty stub** in `Engine/Source/Graphics/C3D/Graphics_C3D.cpp` (line ~1061).
- Same unrolling behavior as GCN/Wii - `ShouldUnroll()` returns `true`.
- Geometry is pre-merged into spatial cells at load time.

### Summary Table

| Feature | Vulkan (Win/Linux) | GX (GameCube/Wii) | C3D (3DS) |
|---|---|---|---|
| Asset sharing (1 mesh in memory) | Yes | Yes | Yes |
| Hardware instanced draw calls | Yes | No | No |
| `InstancedMesh3D` support | Full (GPU instancing) | Unroll fallback | Unroll fallback |
| Per-instance transforms | Storage buffer (GPU) | Baked into vertices | Baked into vertices |
| Draw calls for 100 instances | 1 | ~N cells (spatial grid) | ~N cells (spatial grid) |
| Per-instance vertex colors | Yes | No (unroll limitation) | No (unroll limitation) |

---

## Practical Guidance: Instancing vs. Merged Geometry

### Should you import one mesh and instance it, or merge everything in Blender/Maya?

**Use `InstancedMesh3D` (one mesh, many instances) when:**
- You have many copies of the same mesh (trees, buildings, rocks, props).
- On Vulkan: massive draw call savings (100 instances = 1 draw call).
- On GCN/Wii/3DS: the engine handles the merge for you with spatial culling built in, so you get the same end result as a manual merge but with less manual work and better culling.
- You want the flexibility to adjust instance placement without re-exporting from your DCC tool.

**Use merged geometry (single large mesh from Blender/Maya) when:**
- The objects are all unique (no repeated meshes).
- You need vertex colors on GCN/Wii/3DS (unrolling doesn't support them).
- You want full control over the final mesh topology and optimization.

**Use individual `StaticMesh3D` nodes when:**
- You have a small number of unique objects that each need independent behavior (animation, physics, scripting).
- Draw call count isn't a concern (few objects on screen at once).

### The Bottom Line

On **Vulkan** targets, using `InstancedMesh3D` is a significant performance win - it's real GPU instancing with a single draw call. Importing one building mesh and instancing it 50 times is dramatically cheaper than 50 separate `StaticMesh3D` nodes (1 draw call vs 50), and uses the same amount of VRAM either way since asset data is always shared.

On **GCN/Wii/3DS** targets, `InstancedMesh3D` is a convenience and culling feature, not a draw call reduction tool. The geometry gets pre-merged at load time (like importing a merged scene from Blender), but it's split into spatial cells with distance culling. This is roughly equivalent to manually merging geometry in your DCC tool and splitting it into chunks for LOD culling - but the engine does it automatically. Memory usage is the same as a manual merge (vertices are duplicated per instance), but you gain spatial culling that you wouldn't get from a single monolithic merged mesh.

**TL;DR:** Always prefer `InstancedMesh3D` over placing many individual `StaticMesh3D` nodes for repeated geometry. On Vulkan you get true GPU instancing. On console exports you get automatic spatial cell merging with distance culling. Either way it's better than both "50 separate nodes" and "one giant blob from Blender" approaches.

---

## Configurable Properties

`InstancedMesh3D` exposes these properties in the editor:

| Property | Type | Default | Description |
|---|---|---|---|
| Unrolled Cull Distance | Float | 0.0 | Distance at which unrolled cells are culled (0 = no culling) |
| Unrolled Cell Size | Float | 25.0 | Size of spatial grid cells (in world units) used when unrolling |
| Always Unroll | Bool | false | Force unrolling even on platforms that support hardware instancing |

These only affect behavior when the `InstancedMesh3D` is unrolled (console platforms, or when `Always Unroll` is enabled).
