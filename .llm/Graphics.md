# Graphics System

## Overview

The rendering system uses a platform-agnostic C API (`Graphics.h`) with backend implementations for Vulkan (primary), GX (GameCube/Wii), and C3D (3DS). The `Renderer` class orchestrates rendering at a high level.

## Directory Structure

```
Engine/Source/Graphics/
├── Graphics.h                  # Platform-agnostic API (GFX_* functions)
├── GraphicsTypes.h             # Resource structs (platform-specific #if branches)
├── GraphicsConstants.h         # MAX_FRAMES, MAX_GPU_BONES, etc.
├── GraphicsUtils.h/.cpp        # Utility functions
├── Vulkan/                     # Primary backend
│   ├── VulkanContext.h/.cpp    # Device, swapchain, command buffers
│   ├── Pipeline.h/.cpp         # Graphics/compute pipeline
│   ├── Shader.h/.cpp           # SPIRV shader modules
│   ├── Buffer.h/.cpp           # GPU buffers (vertex/index/uniform/storage)
│   ├── Image.h/.cpp            # Textures and render targets
│   ├── MultiBuffer.h/.cpp      # Per-frame ring buffers, UniformBuffer
│   ├── DescriptorSet.h/.cpp    # Fluent descriptor set builder
│   ├── DescriptorPool.h/.cpp   # Per-frame descriptor pools
│   ├── DescriptorLayoutCache.h # Cached descriptor layouts
│   ├── PipelineCache.h/.cpp    # Persistent pipeline compilation cache
│   ├── RenderPassCache.h/.cpp  # Cached render passes + framebuffers
│   ├── VramAllocator.h/.cpp    # GPU memory management
│   ├── DestroyQueue.h/.cpp     # Deferred resource destruction
│   ├── PipelineConfigs.h/.cpp  # Preset pipeline configurations
│   ├── VulkanTypes.h/.cpp      # PipelineState, GlobalUniformData, LightUniformData
│   ├── VulkanUtils.h/.cpp      # Device selection, validation layers
│   ├── VulkanConstants.h       # Extensions, descriptor limits
│   ├── PostProcessChain.h/.cpp # Post-process orchestration
│   ├── RayTracer.h/.cpp        # Path tracing and light baking
│   ├── Graphics_Vulkan.cpp     # GFX_* function implementations
│   └── PostProcess/
│       ├── PostProcessPass.h/.cpp  # Base post-process pass
│       ├── BlurPass.h/.cpp         # Gaussian blur
│       └── TonemapPass.h/.cpp      # Tonemap (path tracing)
├── GX/                         # GameCube/Wii
│   ├── Graphics_GX.cpp         # GFX_* implementations
│   ├── GxTypes.h               # GxContext struct
│   └── GxUtils.h/.cpp          # GX utilities
└── C3D/                        # Nintendo 3DS
    ├── Graphics_C3D.cpp         # GFX_* implementations
    ├── C3dTypes.h               # C3dContext struct
    ├── C3dUtils.h/.cpp          # C3D utilities
    └── DoubleBuffer.h/.cpp      # Double-buffered vertex data
```

## Graphics API (Platform-Agnostic)

**File:** `Engine/Source/Graphics/Graphics.h`

All functions prefixed with `GFX_`. Key categories:

**Lifecycle:** `GFX_Initialize()`, `GFX_Shutdown()`, `GFX_BeginFrame()`, `GFX_EndFrame()`

**Render passes:** `GFX_BeginRenderPass(RenderPassId)`, `GFX_EndRenderPass()`

**State:** `GFX_SetPipelineState(PipelineConfig)`, `GFX_SetViewport()`, `GFX_SetScissor()`

**Drawing:** `GFX_DrawStaticMesh()`, `GFX_DrawSkeletalMeshComp()`, `GFX_DrawInstancedMeshComp()`, `GFX_DrawLines()`, `GFX_DrawFullscreen()`, `GFX_DrawQuad()`, `GFX_DrawText()`, `GFX_DrawPoly()`

**Resource creation:** `GFX_CreateTextureResource()`, `GFX_CreateMaterialResource()`, `GFX_CreateStaticMeshResource()`, etc. (and matching `GFX_Destroy*` functions)

**Advanced:** `GFX_PathTrace()`, `GFX_BeginLightBake()`, `GFX_UpdateLightBake()`, `GFX_EndLightBake()`

## Renderer Class

**File:** `Engine/Source/Engine/Renderer.h/.cpp`

Singleton (`Renderer::Get()`) that orchestrates the rendering pipeline:

1. `BeginFrame()` — Acquire swapchain image
2. `GatherDrawData(world)` — Collect visible objects into draw lists
3. `FrustumCull(camera)` — Cull objects outside view
4. `RenderShadowCasters()` — Shadow pass
5. `RenderDraws()` — Main forward pass (opaque, translucent, additive)
6. `RenderSelectedGeometry()` — Editor selection highlight
7. Widget rendering — 2D UI pass
8. `PostProcessChain` — Blur, tonemap
9. `EndFrame()` — Submit and present

**Draw lists:** `mShadowDraws`, `mOpaqueDraws`, `mTranslucentDraws`, `mWireframeDraws`, `mWidgetDraws`

**Default assets:** White/black textures, default material, primitive meshes (cone, cube, cylinder, plane, sphere, torus), default fonts.

## Render Passes

```cpp
enum class RenderPassId { Shadows, Forward, Selected, Ui, Clear, HitCheck };
```

## Pipeline Configurations

```cpp
enum class PipelineConfig {
    Shadow, Forward, Opaque, Translucent, Additive,
    ShadowMeshBack, ShadowMeshFront, PostProcess, NullPostProcess,
    Quad, Text, Poly, Selected, HitCheck, Wireframe, Collision, Line
};
```

## Vulkan Backend Key Classes

**VulkanContext**: Device, swapchain, command buffers, synchronization, render pass/pipeline caches, descriptor pools, shadow map + scene color images, ray tracer, post-process chain.

**Pipeline**: Wraps `VkPipeline` + `VkPipelineLayout`. Created from `PipelineState` struct.

**Shader**: Wraps `VkShaderModule`. Loads SPIRV, reflects descriptor bindings via SPIRV-Cross.

**Buffer/Image**: GPU resource wrappers. Image supports mipmaps, layout transitions, samplers.

**DescriptorSet**: Fluent builder — `Begin().WriteImage().WriteUniformBuffer().Build().Bind()`.

## Material System

**Files:** `Engine/Source/Engine/Assets/Material.h/.cpp`, `MaterialBase.h`, `MaterialLite.h`, `MaterialInstance.h`

Materials hold shader parameters (scalar, vector, texture):
- `SetScalarParameter(name, float)`, `SetVectorParameter(name, vec4)`, `SetTextureParameter(name, Texture*)`
- `GetBlendMode()`, `GetCullMode()`, `IsDepthTestDisabled()`
- `MaterialInstance` inherits from a base material, overriding specific parameters

## Platform Constants

| Constant | Vulkan | GX | C3D |
|----------|--------|-----|-----|
| `MAX_FRAMES` | 2 | 1 | 2 |
| `MAX_MESH_VERTEX_COUNT` | 4B | 65535 | 65535 |
| `MAX_GPU_BONES` | 64 | 10 | 16 |
| `SUPPORTS_SECOND_SCREEN` | 0 | 0 | 1 |

## Vertex Types

```cpp
enum class VertexType {
    Vertex, VertexInstanceColor, VertexColor, VertexColorInstanceColor,
    VertexUI, VertexLine, VertexSkinned, VertexParticle, Max
};
```

Each has a corresponding struct with position, texcoords, normals, colors, bone data as appropriate.

## Resource Structs (GraphicsTypes.h)

Resource structs use `#if API_VULKAN / API_GX / API_C3D` branches. For example:
```cpp
struct TextureResource {
#if API_VULKAN
    Image* mImage;
#elif API_GX
    GXTexObj mGxTexObj; TPLFile mTplFile; void* mTplData;
#elif API_C3D
    C3D_Tex mTex; void* mT3dsData;
#endif
};
```

Similar pattern for `MaterialResource`, `StaticMeshResource`, `SkeletalMeshResource`, etc.
