# Skybox in Octave Engine

## Overview

Octave Engine supports skybox rendering through a standard `StaticMesh3D` node placed in the scene. The skybox is a large inverted sphere that surrounds the camera, displaying either a textured panorama or a solid color background. The editor provides menu shortcuts to create pre-configured skybox nodes.

---

## Creating a Skybox

### From the Scene Panel

Right-click in the scene hierarchy and use either menu:

- **Add Basic 3D > Skybox > Skybox Textured** - For panoramic sky textures
- **Add Basic 3D > Skybox > Skybox Vertex Color** - For solid/vertex-colored skies
- **Add Node > 3D > Skybox > Skybox Textured**
- **Add Node > 3D > Skybox > Skybox Vertex Color**

Both options create a `StaticMesh3D` node named "Skybox" with these defaults:

| Property | Value |
|---|---|
| Mesh | SM_Sphere |
| Scale | 500, 500, 500 |
| Collision | Disabled |
| Overlaps | Disabled |
| Physics | Disabled |
| Cast Shadows | Disabled |
| Receive Shadows | Disabled |
| Receive Simple Shadows | Disabled |

If a Material asset is selected in the asset browser when the skybox is created, it will be applied automatically as the material override.

---

## Setting Up a Textured Skybox

### Step 1: Import a Sky Texture

Import an equirectangular panorama image. The texture must have **power-of-2 dimensions** (required by the engine). Recommended sizes:

| Resolution | Aspect Ratio | Notes |
|---|---|---|
| 2048 x 1024 | 2:1 | Good balance of quality and memory |
| 4096 x 2048 | 2:1 | Higher quality, more memory |
| 1024 x 512 | 2:1 | Lower quality, suitable for 3DS/GCN |

### Step 2: Create a Material

Create a new `MaterialLite` asset and configure it:

| Property | Value | Reason |
|---|---|---|
| Shading Model | **Unlit** | Sky should not be affected by scene lighting |
| Cull Mode | **Front** | Renders the inside faces of the sphere (camera is inside) |
| Depth Test | **Disabled** | Sky renders behind everything regardless of depth |
| Apply Fog | **Disabled** | Sky should not be affected by fog |
| Texture 0 | Your sky texture | The panorama image |

### Step 3: Assign the Material

Select the Skybox node in the scene hierarchy and set its **Material Override** to the material you created.

### Step 4: Adjust Scale

The default scale of 500 works for most scenes. If your camera's far plane is smaller, reduce the scale so the skybox stays within the view frustum. A good rule of thumb:

```
Skybox scale < Camera far distance
```

---

## Setting Up a Vertex-Colored Skybox

For a simple gradient sky without textures:

1. Create a skybox node using **Skybox Vertex Color** from the menu
2. Create a `MaterialLite` with **Unlit** shading, **Cull Mode: Front**, **Depth Test: Disabled**
3. Set **Vertex Color Mode** to **Modulate** on the material
4. Import or create a mesh with baked vertex colors representing your sky gradient
5. Assign the mesh and material to the Skybox node

---

## Platform Notes

| Platform | Skybox Support |
|---|---|
| Vulkan (Windows/Linux) | Full support |
| GameCube / Wii (GX) | Supported via `GFX_DrawStaticMesh` |
| 3DS (Citro3D) | Supported via `GFX_DrawStaticMesh` |

The skybox is a regular scene node, so it renders on all platforms that support `StaticMesh3D`.

---

## Ray Traced Sky (Vulkan Only)

Separately from the skybox node, Octave supports ray traced sky colors used by the path tracer and light baker. These are configured in **Global Properties > Light Baking**:

| Property | Description |
|---|---|
| Sky Zenith Color | Color at the top of the sky dome |
| Sky Horizon Color | Color at the horizon |
| Ground Color | Color below the horizon |

These colors are only used by the Vulkan ray tracing shaders and do not affect the rasterized skybox node.

---

## Tips

- **Skybox should be a child of the root node** so it's always present in the scene.
- **Rotation**: Rotate the Skybox node's Y-axis to adjust the sky orientation.
- **Multiple skyboxes**: You can swap between skyboxes by enabling/disabling different Skybox nodes via script.
- **Performance**: A skybox is a single draw call with a simple unlit material - negligible performance cost.
- **Sorting**: If the skybox renders in front of other objects, ensure Depth Test is disabled on the material and the skybox scale is within the camera's far plane.
