#include "Graphics/Vulkan/RayTracer.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Log.h"
#include "Engine.h"
#include "Utilities.h"
#include "World.h"
#include "Renderer.h"

#include "Graphics/GraphicsUtils.h"

#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/StaticMesh3d.h"

void RayTracer::CreateStaticRayTraceResources()
{
    if (GetVulkanContext()->IsRayTracingSupported())
    {
        // Uniform Buffer
        mRayTraceUniformBuffer = new UniformBuffer(sizeof(RayTraceUniforms), "Path Trace Uniforms");

        // Buffers
        mRayTraceTriangleBuffer = new Buffer(BufferType::Storage, sizeof(RayTraceTriangle), "Path Trace Triangle Buffer", nullptr, false);
        mRayTraceMeshBuffer = new Buffer(BufferType::Storage, sizeof(RayTraceMesh), "Path Trace Mesh Buffer", nullptr, false);
        mRayTraceLightBuffer = new Buffer(BufferType::Storage, sizeof(RayTraceLight), "Path Trace Light Buffer", nullptr, false);
        mLightBakeVertexBuffer = new Buffer(BufferType::Storage, sizeof(LightBakeVertex), "Light Bake Vertex Buffer", nullptr, true);

        mBakeDiffuseTriangleBuffer = new Buffer(BufferType::Storage, sizeof(DiffuseTriangle), "Bake Triangle Buffer", nullptr, false);
        mBakeAverageBuffer = new Buffer(BufferType::Storage, sizeof(VertexLightData), "Bake Average Buffer", nullptr, true);
    }
}

void RayTracer::DestroyStaticRayTraceResources()
{
    if (GetVulkanContext()->IsRayTracingSupported())
    {
        GetDestroyQueue()->Destroy(mRayTraceUniformBuffer);
        mRayTraceUniformBuffer = nullptr;

        GetDestroyQueue()->Destroy(mRayTraceTriangleBuffer);
        mRayTraceTriangleBuffer = nullptr;

        GetDestroyQueue()->Destroy(mRayTraceMeshBuffer);
        mRayTraceMeshBuffer = nullptr;

        GetDestroyQueue()->Destroy(mRayTraceLightBuffer);
        mRayTraceLightBuffer = nullptr;

        GetDestroyQueue()->Destroy(mLightBakeVertexBuffer);
        mLightBakeVertexBuffer = nullptr;

        GetDestroyQueue()->Destroy(mBakeAverageBuffer);
        mBakeAverageBuffer = nullptr;

        GetDestroyQueue()->Destroy(mBakeDiffuseTriangleBuffer);
        mBakeDiffuseTriangleBuffer = nullptr;
    }
}

void RayTracer::CreateDynamicRayTraceResources()
{
    if (GetVulkanContext()->IsRayTracingSupported())
    {
        // Images
        ImageDesc imageDesc;

        VkFormat sceneColorImageFormat = GetVulkanContext()->GetSceneColorFormat();

        imageDesc.mWidth = GetVulkanContext()->GetSceneWidth();
        imageDesc.mHeight = GetVulkanContext()->GetSceneHeight();
        imageDesc.mFormat = sceneColorImageFormat;
        imageDesc.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

        SamplerDesc samplerDesc;
        samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        mPathTraceImage = new Image(imageDesc, samplerDesc, "Path Trace Image");

        mPathTraceImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

void RayTracer::DestroyDynamicRayTraceResources()
{
    if (GetVulkanContext()->IsRayTracingSupported())
    {
        GetDestroyQueue()->Destroy(mPathTraceImage);
        mPathTraceImage = nullptr;
    }
}

void RayTracer::UpdateRayTracingScene(
    std::vector<RayTraceTriangle>& triangleData,
    std::vector<RayTraceMesh>& meshData,
    std::vector<RayTraceLight>& lightData,
    int32_t* outBakeMeshIndex)
{
    World* world = GetWorld(0);

    if (world == nullptr)
        return;

    triangleData.clear();
    meshData.clear();
    lightData.clear();

    // Update triangle + mesh + light buffers
    mTextureImages.clear();

    mTextureImages.reserve(PATH_TRACE_MAX_TEXTURES);
    Texture* whiteTex = LoadAsset<Texture>("T_White");
    OCT_ASSERT(whiteTex);
    Image* whiteImg = whiteTex->GetResource()->mImage;
    OCT_ASSERT(whiteImg);
    mTextureImages.push_back(whiteImg);

    uint32_t totalTriangles = 0;

    // TODO-NODE: GatherNodes is slow, and allocates memory, consider different approach.
    const std::vector<Node*>& nodes = world->GatherNodes();

    for (uint32_t a = 0; a < nodes.size(); ++a)
    {
        if (!nodes[a]->IsVisible())
            continue;

        StaticMesh3D* meshComp = nodes[a]->As<StaticMesh3D>();
        Light3D* lightComp = nodes[a]->As<Light3D>();
        if (meshComp != nullptr)
        {
            StaticMesh* meshAsset = meshComp->GetStaticMesh();
            if (meshAsset == nullptr)
            {
                continue;
            }

            if (outBakeMeshIndex != nullptr &&
                meshComp == mLightBakeNodes[mBakingCompIndex].Get())
            {
                *outBakeMeshIndex = (int32_t)meshData.size();
            }

            Material* matTop = meshComp->GetMaterial();
            MaterialLite* material = (matTop && matTop->IsLite()) ? (MaterialLite*)matTop : nullptr;

            if (material == nullptr)
            {
                material = Renderer::Get()->GetDefaultMaterial();
            }

            glm::mat4 transform = meshComp->GetRenderTransform();
            glm::mat4 normalTransform = glm::transpose(glm::inverse(transform));

            meshData.push_back(RayTraceMesh());
            RayTraceMesh& mesh = meshData.back();
            Bounds bounds = meshComp->GetBounds();
            mesh.mBounds = glm::vec4(bounds.mCenter.x, bounds.mCenter.y, bounds.mCenter.z, bounds.mRadius);
            mesh.mStartTriangleIndex = totalTriangles;
            mesh.mNumTriangles = meshAsset->GetNumFaces();
            mesh.mCastShadows = meshComp->ShouldCastShadows();
            mesh.mHasBakedLighting = meshComp->HasBakedLighting();
            WriteMaterialLiteUniformData(mesh.mMaterial, material);

            // Add textures and record indices.
            for (uint32_t t = 0; t < MATERIAL_LITE_MAX_TEXTURES; ++t)
            {
                Texture* tex = material->GetTexture((TextureSlot)t);
                Image* img = tex ? tex->GetResource()->mImage : nullptr;

                if (img != nullptr)
                {
                    int32_t index = -1;

                    // Look through already added textures for match
                    for (uint32_t i = 0; i < mTextureImages.size(); ++i)
                    {
                        if (mTextureImages[i] == img)
                        {
                            index = (int32_t)i;
                            break;
                        }
                    }

                    // If texture wasn't found, then add it to the list.
                    if (index == -1)
                    {
                        mTextureImages.push_back(img);
                        index = int32_t(mTextureImages.size() - 1);
                    }

                    OCT_ASSERT(mTextureImages.size() < PATH_TRACE_MAX_TEXTURES);
                    OCT_ASSERT(index >= 0 && index < PATH_TRACE_MAX_TEXTURES);

                    mesh.mTextures[t] = (uint32_t)index;
                }
                else
                {
                    mesh.mTextures[t] = 0;
                }
            }

            // Add triangle data.
            bool hasColor = meshAsset->HasVertexColor();
            IndexType* indices = meshAsset->GetIndices();
            Vertex* verts = hasColor ? nullptr : meshAsset->GetVertices();
            VertexColor* colorVerts = hasColor ? meshAsset->GetColorVertices() : nullptr;
            const std::vector<uint32_t>& instanceColors = meshComp->GetInstanceColors();
            uint32_t numVerts = meshAsset->GetNumVertices();

            std::vector<glm::vec4>* directLightColors = nullptr;
            if (mLightBakePhase == LightBakePhase::Indirect)
            {
                // See if we have baked direct lighting for this mesh. (This is higher precision than instance colors)
                for (uint32_t i = 0; i < mLightBakeNodes.size(); ++i)
                {
                    if (mLightBakeNodes[i] == meshComp)
                    {
                        directLightColors = &(mLightBakeResults[i].mDirectColors);
                        break;
                    }
                }
            }

            for (uint32_t t = 0; t < mesh.mNumTriangles; ++t)
            {
                triangleData.push_back(RayTraceTriangle());
                RayTraceTriangle& triangle = triangleData.back();

                for (uint32_t v = 0; v < 3; ++v)
                {
                    uint32_t vertIndex = indices[t * 3 + v];
                    if (hasColor)
                    {
                        triangle.mVertices[v].mPosition = glm::vec3(transform * glm::vec4(colorVerts[vertIndex].mPosition, 1));
                        triangle.mVertices[v].mTexcoord0 = colorVerts[vertIndex].mTexcoord0;
                        triangle.mVertices[v].mTexcoord1 = colorVerts[vertIndex].mTexcoord1;
                        triangle.mVertices[v].mNormal = glm::normalize(glm::vec3(normalTransform * glm::vec4(colorVerts[vertIndex].mNormal, 0)));
                        triangle.mVertices[v].mColor = ColorUint32ToFloat4(colorVerts[vertIndex].mColor);
                    }
                    else
                    {
                        triangle.mVertices[v].mPosition = glm::vec3(transform * glm::vec4(verts[vertIndex].mPosition, 1));
                        triangle.mVertices[v].mTexcoord0 = verts[vertIndex].mTexcoord0;
                        triangle.mVertices[v].mTexcoord1 = verts[vertIndex].mTexcoord1;
                        triangle.mVertices[v].mNormal = glm::normalize(glm::vec3(normalTransform * glm::vec4(verts[vertIndex].mNormal, 0)));
                        triangle.mVertices[v].mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    }

                    if (directLightColors != nullptr && directLightColors->size() == numVerts)
                    {
                        triangle.mVertices[v].mColor = directLightColors->at(vertIndex);
                    }
                    else if (instanceColors.size() == numVerts)
                    {
                        triangle.mVertices[v].mColor = ColorUint32ToFloat4(instanceColors[vertIndex]);
                    }
                }
            }

            totalTriangles += mesh.mNumTriangles;
        }
        else if (lightComp && lightComp->GetLightingDomain() != LightingDomain::Dynamic)
        {
            if (lightComp->Is(PointLight3D::ClassRuntimeId()))
            {
                PointLight3D* pointLightComp = lightComp->As<PointLight3D>();

                lightData.push_back(RayTraceLight());
                RayTraceLight& light = lightData.back();
                light.mPosition = pointLightComp->GetWorldPosition();
                light.mRadius = pointLightComp->GetRadius();
                light.mColor = pointLightComp->GetColor();
                light.mDirection = { 0.0f, 0.0f, -1.0f };
                light.mLightType = uint32_t(RayTraceLightType::Point);
                light.mCastShadows = (uint32_t)pointLightComp->ShouldCastShadows();
            }
            else if (lightComp->Is(DirectionalLight3D::ClassRuntimeId()))
            {
                DirectionalLight3D* dirLightComp = lightComp->As<DirectionalLight3D>();

                lightData.push_back(RayTraceLight());
                RayTraceLight& light = lightData.back();
                light.mPosition = dirLightComp->GetWorldPosition();
                light.mRadius = 10000.0f;
                light.mColor = dirLightComp->GetColor();
                light.mDirection = dirLightComp->GetDirection();
                light.mLightType = uint32_t(RayTraceLightType::Directional);
                light.mCastShadows = (uint32_t)dirLightComp->ShouldCastShadows();
            }
        }
    }

    // Reallocate storage buffers if needed.
    size_t triangleSize = triangleData.size() * sizeof(RayTraceTriangle);
    size_t meshSize = meshData.size() * sizeof(RayTraceMesh);
    size_t lightSize = lightData.size() * sizeof(RayTraceLight);

    if (triangleSize > mRayTraceTriangleBuffer->GetSize())
    {
        GetDestroyQueue()->Destroy(mRayTraceTriangleBuffer);
        mRayTraceTriangleBuffer = nullptr;
        mRayTraceTriangleBuffer = new Buffer(BufferType::Storage, triangleSize, "Path Trace Triangle Buffer", nullptr, false);
    }

    if (meshSize > mRayTraceMeshBuffer->GetSize())
    {
        GetDestroyQueue()->Destroy(mRayTraceMeshBuffer);
        mRayTraceMeshBuffer = nullptr;
        mRayTraceMeshBuffer = new Buffer(BufferType::Storage, meshSize, "Path Trace Mesh Buffer", nullptr, false);
    }

    if (lightSize > mRayTraceLightBuffer->GetSize())
    {
        GetDestroyQueue()->Destroy(mRayTraceLightBuffer);
        mRayTraceLightBuffer = nullptr;
        mRayTraceLightBuffer = new Buffer(BufferType::Storage, lightSize, "Path Trace Light Buffer", nullptr, false);
    }

    if (triangleSize > 0)
        mRayTraceTriangleBuffer->Update(triangleData.data(), triangleSize);

    if (meshSize > 0)
        mRayTraceMeshBuffer->Update(meshData.data(), meshSize);

    if (lightSize > 0)
        mRayTraceLightBuffer->Update(lightData.data(), lightSize);

    // Update texture array descriptor.
    // Fill in any unused texture slots with the T_White texture
    while (mTextureImages.size() < PATH_TRACE_MAX_TEXTURES)
    {
        mTextureImages.push_back(whiteImg);
    }
}

void RayTracer::UpdateBakeVertexData()
{
    StaticMesh3D* meshComp = mLightBakeNodes[mBakingCompIndex].Get<StaticMesh3D>();
    StaticMesh* meshAsset = meshComp->GetStaticMesh();
    uint32_t numVerts = meshAsset->GetNumVertices();

    // Update light bake vertex buffer
    std::vector<LightBakeVertex> lightBakeVertexData;

    glm::mat4 transform = meshComp->GetRenderTransform();
    glm::mat4 normalTransform = glm::transpose(glm::inverse(transform));

    lightBakeVertexData.resize(numVerts);

    // Add triangle data.
    bool hasColor = meshAsset->HasVertexColor();
    IndexType* indices = meshAsset->GetIndices();
    Vertex* verts = hasColor ? nullptr : meshAsset->GetVertices();
    VertexColor* colorVerts = hasColor ? meshAsset->GetColorVertices() : nullptr;
    const LightBakeResult& bakeResult = mLightBakeResults[mBakingCompIndex];

    for (uint32_t v = 0; v < numVerts; ++v)
    {
        if (hasColor)
        {
            lightBakeVertexData[v].mPosition = glm::vec3(transform * glm::vec4(colorVerts[v].mPosition, 1));
            lightBakeVertexData[v].mNormal = glm::normalize(glm::vec3(normalTransform * glm::vec4(colorVerts[v].mNormal, 0)));
        }
        else
        {
            lightBakeVertexData[v].mPosition = glm::vec3(transform * glm::vec4(verts[v].mPosition, 1));
            lightBakeVertexData[v].mNormal = glm::normalize(glm::vec3(normalTransform * glm::vec4(verts[v].mNormal, 0)));
        }

        // During the diffusion phase, we want to upload the resulting light data
        if (mLightBakePhase == LightBakePhase::Diffuse)
        {
            lightBakeVertexData[v].mDirectLight = bakeResult.mDirectColors[v];
            lightBakeVertexData[v].mIndirectLight = bakeResult.mIndirectColors[v];
        }
    }

    // If the vertex data is larger than our currently allocated storage buffer, create a new larger buffer
    size_t vertBufferSize = sizeof(LightBakeVertex) * lightBakeVertexData.size();
    if (mLightBakeVertexBuffer->GetSize() < vertBufferSize)
    {
        GetDestroyQueue()->Destroy(mLightBakeVertexBuffer);
        mLightBakeVertexBuffer = nullptr;
        mLightBakeVertexBuffer = new Buffer(BufferType::Storage, vertBufferSize, "Light Bake Vertex Buffer", nullptr, true);
    }

    // Upload vertex data
    if (vertBufferSize > 0)
    {
        mLightBakeVertexBuffer->Update(lightBakeVertexData.data(), vertBufferSize);
    }
}

void RayTracer::PathTraceWorld()
{
    OCT_ASSERT(!IsLightBakeInProgress());
    World* world = GetWorld(0);

    if (world != nullptr)
    {
        // Check to see if camera moved. If so we need to reset our accumulated image data.

        Camera3D* camera = world->GetActiveCamera();
        if (camera != nullptr)
        {
            glm::vec3 camPos = camera->GetWorldPosition();
            glm::vec3 camRot = camera->GetWorldRotationEuler();

            bool posChanged = glm::any(glm::epsilonNotEqual(mPathTracePrevCameraPos, camPos, 0.00001f));
            bool rotChanged = glm::any(glm::epsilonNotEqual(mPathTracePrevCameraRot, camRot, 0.00001f));

            if (posChanged || rotChanged)
            {
                mAccumulatedFrames = 0;
            }

            mPathTracePrevCameraPos = camPos;
            mPathTracePrevCameraRot = camRot;
        }
        else
        {
            mAccumulatedFrames = 0;
        }

        if (!Renderer::Get()->IsPathTraceAccumulationEnabled())
        {
            mAccumulatedFrames = 0;
        }

        std::vector<RayTraceTriangle> triangleData;
        std::vector<RayTraceMesh> meshData;
        std::vector<RayTraceLight> lightData;
        UpdateRayTracingScene(triangleData, meshData, lightData);

        // Write uniform data.
        RayTraceUniforms uniforms;
        uniforms.mNumTriangles = (uint32_t)triangleData.size();
        uniforms.mNumMeshes = (uint32_t)meshData.size();
        uniforms.mNumLights = (uint32_t)lightData.size();
        uniforms.mMaxBounces = Renderer::Get()->GetMaxBounces();
        uniforms.mRaysPerThread = Renderer::Get()->GetRaysPerPixel();
        uniforms.mAccumulatedFrames = mAccumulatedFrames;
        uniforms.mShadowBias = 0.01f;
        uniforms.mBakeMeshIndex = -1;
        uniforms.mReceiveShadows = true;
        uniforms.mSkyZenithColor = Renderer::Get()->GetSkyZenithColor();
        uniforms.mSkyHorizonColor = Renderer::Get()->GetSkyHorizonColor();
        uniforms.mGroundColor = Renderer::Get()->GetGroundColor();
        mRayTraceUniformBuffer->Update(&uniforms, sizeof(RayTraceUniforms));

        VkCommandBuffer cb = GetCommandBuffer();

        mPathTraceImage->Transition(VK_IMAGE_LAYOUT_GENERAL, cb);

        GetVulkanContext()->SetComputeShader("PathTrace.comp");

        GetVulkanContext()->CommitPipeline();
        BindPathTraceDescriptorSet();

        // TODO-NODE: Should we use viewport instead?
        uint32_t width = GetEngineState()->mWindowWidth;
        uint32_t height = GetEngineState()->mWindowHeight;

        vkCmdDispatch(cb, (width + 7) / 8, (height + 7) / 8, 1);

        mPathTraceImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cb);

        mAccumulatedFrames++;
    }
}

void RayTracer::BeginLightBake()
{
    World* world = GetWorld(0);

    if (world != nullptr &&
        mLightBakePhase == LightBakePhase::Count)
    {
        mLightBakeNodes.clear();

        // TODO-NODE: Again, GatherNodes() is slow, so consider an alternative.
        const std::vector<Node*>& nodes = world->GatherNodes();
        for (uint32_t a = 0; a < nodes.size(); ++a)
        {
            Node* node = nodes[a];
            StaticMesh3D* meshComp = node->As<StaticMesh3D>();

            if (meshComp != nullptr &&
                meshComp->IsVisible() &&
                meshComp->GetBakeLighting())
            {
                meshComp->ClearInstanceColors();
                mLightBakeNodes.push_back(ResolvePtr(meshComp));
            }
        }

        if (mLightBakeNodes.size() > 0)
        {
            mLightBakePhase = LightBakePhase::Direct;
            mBakingCompIndex = -1;
            mNextBakingCompIndex = 0;
            mBakedFrame = -1;

            uint32_t directDiffusals = Renderer::Get()->GetBakeDiffuseDirectPasses();
            uint32_t indirectDiffusals = Renderer::Get()->GetBakeDiffuseIndirectPasses();
            mTotalDiffusePasses = glm::max<uint32_t>(directDiffusals, indirectDiffusals);

            // Add an extra diffusal pass for the final deduplication averaging.
            ++mTotalDiffusePasses;

            mLightBakeResults.resize(mLightBakeNodes.size());
        }
    }
}

void RayTracer::UpdateLightBake()
{
    // Check if we can dispatch a new bake job.
    if (mLightBakePhase == LightBakePhase::Direct)
    {
        // Are we currently baking a mesh or can we move to the next one?
        if (mBakingCompIndex == -1)
        {
            // Do we have more meshes to bake?
            if (mNextBakingCompIndex < mLightBakeNodes.size())
            {
                StaticMesh3D* meshComp = mLightBakeNodes[mNextBakingCompIndex].Get<StaticMesh3D>();
                if (meshComp != nullptr &&
                    meshComp->GetWorld() == GetWorld(0))
                {
                    DispatchNextLightBake();
                }
                else
                {
                    // Component was deleted? Or exiled from world?
                    ++mNextBakingCompIndex;
                }
            }
            else
            {
                // Finished baking direct light, move on to Indirect phase
                mLightBakePhase = LightBakePhase::Indirect;
                mBakingCompIndex = -1;
                mNextBakingCompIndex = 0;
                mBakedFrame = -1;
            }
        }
    }
    else if (mLightBakePhase == LightBakePhase::Indirect)
    {
        if (mBakingCompIndex == -1)
        {
            mBakingCompIndex = mNextBakingCompIndex;
            mAccumulatedFrames = 0;

            ++mNextBakingCompIndex;
        }

        if (mBakingCompIndex < mLightBakeNodes.size() &&
            Renderer::Get()->GetBakeIndirectIterations() > 0)
        {
            DispatchNextLightBake();
        }
        else
        {
            // Light has baked, now diffuse the light to blur result slightly.
            mLightBakePhase = LightBakePhase::Diffuse;
            mBakingCompIndex = -1;
            mNextBakingCompIndex = 0;
            mBakedFrame = -1;
        }
    }
    else if (mLightBakePhase == LightBakePhase::Diffuse)
    {
        if (mBakingCompIndex == -1)
        {
            mBakingCompIndex = mNextBakingCompIndex;
            mAccumulatedFrames = 0;

            ++mNextBakingCompIndex;
        }

        if (mBakingCompIndex < mLightBakeNodes.size())
        {
            DispatchNextBakeDiffuse();
        }
        else
        {
            EndLightBake();
        }
    }
}

void RayTracer::EndLightBake()
{
    FinalizeLightBake();

    mLightBakePhase = LightBakePhase::Count;
    mLightBakeNodes.clear();
    mLightBakeResults.clear();

    mBakingCompIndex = -1;
    mNextBakingCompIndex = 0;
    mBakedFrame = -1;
}

bool RayTracer::IsLightBakeInProgress()
{
#if EDITOR
    return (mLightBakePhase != LightBakePhase::Count);
#else
    return false;
#endif
}

float RayTracer::GetLightBakeProgress()
{
    float ret = 1.0f;

    if (mLightBakePhase != LightBakePhase::Count)
    {
        uint32_t numComps = (uint32_t)mLightBakeNodes.size();
        uint32_t numIndirectIterations = Renderer::Get()->GetBakeIndirectIterations();
        uint32_t numDirectDiffuses = Renderer::Get()->GetBakeDiffuseDirectPasses();
        uint32_t numIndirectDiffuses = Renderer::Get()->GetBakeDiffuseIndirectPasses();
        uint32_t numDiffuseIterations = glm::max<uint32_t>(numDirectDiffuses, numIndirectDiffuses) + 1;

        uint32_t totalDirectDispatches = numComps;
        uint32_t totalIndirectDispatches = numComps * numIndirectIterations;
        uint32_t totalDiffuseDispatches = numComps * numDiffuseIterations;
        uint32_t totalDispatches = totalDirectDispatches + totalIndirectDispatches + totalDiffuseDispatches;

        uint32_t curDispatch = 0;
        uint32_t compIndex = (uint32_t)glm::clamp<int32_t>(mNextBakingCompIndex - 1, 0, (int32_t)numComps);

        if (mLightBakePhase == LightBakePhase::Direct)
        {
            curDispatch = uint32_t(compIndex);
        }
        else if (mLightBakePhase == LightBakePhase::Indirect)
        {
            curDispatch = uint32_t(compIndex) * numIndirectIterations + mAccumulatedFrames + totalDirectDispatches;
        }
        else if (mLightBakePhase == LightBakePhase::Diffuse)
        {
            curDispatch = uint32_t(compIndex) * numDiffuseIterations + mAccumulatedFrames + totalDirectDispatches + totalIndirectDispatches;
        }

        ret = float(curDispatch) / float(totalDispatches);
    }

    return ret;
}

Image* RayTracer::GetPathTraceImage()
{
    return mPathTraceImage;
}

LightBakePhase RayTracer::GetLightBakePhase() const
{
    return mLightBakePhase;
}

void RayTracer::DispatchNextLightBake()
{
    if (mBakingCompIndex == -1)
    {
        mBakingCompIndex = mNextBakingCompIndex;
        mAccumulatedFrames = 0;

        ++mNextBakingCompIndex;
    }

    if (mBakingCompIndex >= mLightBakeNodes.size() ||
        (mLightBakePhase == LightBakePhase::Indirect && mAccumulatedFrames >= Renderer::Get()->GetBakeIndirectIterations()))
        return;

    StaticMesh3D* meshComp = mLightBakeNodes[mBakingCompIndex].Get<StaticMesh3D>();

    if (meshComp != nullptr &&
        meshComp->GetStaticMesh() != nullptr &&
        meshComp->GetWorld() == GetWorld(0))
    {
        // Update path tracing scene
        int32_t bakeMeshIndex = -1;
        std::vector<RayTraceTriangle> triangleData;
        std::vector<RayTraceMesh> meshData;
        std::vector<RayTraceLight> lightData;
        UpdateRayTracingScene(triangleData, meshData, lightData, &bakeMeshIndex);

        // Update light bake vertex buffer
        std::vector<LightBakeVertex> lightBakeVertexData;

        StaticMesh* meshAsset = meshComp->GetStaticMesh();
        uint32_t numVerts = meshAsset->GetNumVertices();

        // On the first iteration of the bake, we need to upload bake vertex data.
        // During the indirect bake, previous dispatch results are accumulated.
        if (mAccumulatedFrames == 0)
        {
            UpdateBakeVertexData();
        }

        // Update uniform buffer
        RayTraceUniforms uniforms;
        uniforms.mNumTriangles = (uint32_t)triangleData.size();
        uniforms.mNumMeshes = (uint32_t)meshData.size();
        uniforms.mNumLights = (uint32_t)lightData.size();
        uniforms.mMaxBounces = Renderer::Get()->GetBakeMaxBounces();
        uniforms.mRaysPerThread = Renderer::Get()->GetBakeRaysPerVertex();
        uniforms.mAccumulatedFrames = mAccumulatedFrames;
        uniforms.mNumBakeVertices = numVerts;
        uniforms.mNumBakeTriangles = meshAsset->GetNumFaces();
        uniforms.mShadowBias = Renderer::Get()->GetBakeShadowBias();
        uniforms.mBakeMeshIndex = bakeMeshIndex;
        uniforms.mReceiveShadows = (uint32_t)meshComp->ShouldReceiveShadows();
        uniforms.mSkyZenithColor = Renderer::Get()->GetSkyZenithColor();
        uniforms.mSkyHorizonColor = Renderer::Get()->GetSkyHorizonColor();
        uniforms.mGroundColor = Renderer::Get()->GetGroundColor();
        mRayTraceUniformBuffer->Update(&uniforms, sizeof(RayTraceUniforms));

        VkCommandBuffer cb = GetCommandBuffer();

        // Apparently still need to transition this image or validation gets mad.
        mPathTraceImage->Transition(VK_IMAGE_LAYOUT_GENERAL, cb);

        Pipeline* bakePipeline = nullptr;

        if (mLightBakePhase == LightBakePhase::Direct)
        {
            GetVulkanContext()->SetComputeShader("LightBakeDirect.comp");
        }
        else if (mLightBakePhase == LightBakePhase::Indirect)
        {
            GetVulkanContext()->SetComputeShader("LightBakeIndirect.comp");
        }

        GetVulkanContext()->CommitPipeline();
        BindPathTraceDescriptorSet();

        vkCmdDispatch(cb, (numVerts + 31) / 32, 1, 1);

        mPathTraceImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cb);

        mBakedFrame = (int64_t)Renderer::Get()->GetFrameNumber();
        mAccumulatedFrames++;
    }
}

void RayTracer::DispatchNextBakeDiffuse()
{
    if (mBakingCompIndex >= mLightBakeNodes.size() ||
        mAccumulatedFrames >= mTotalDiffusePasses)
        return;

    StaticMesh3D* meshComp = mLightBakeNodes[mBakingCompIndex].Get<StaticMesh3D>();

    if (meshComp != nullptr &&
        meshComp->GetStaticMesh() != nullptr &&
        meshComp->GetWorld() == GetWorld(0))
    {
        StaticMesh* meshAsset = meshComp->GetStaticMesh();
        uint32_t numVerts = meshAsset->GetNumVertices();
        uint32_t numTriangles = meshAsset->GetNumFaces();

        // On the first iteration of the bake, we need to upload bake vertex data.
        // During the indirect bake, previous dispatch results are accumulated.
        if (mAccumulatedFrames == 0)
        {
            UpdateBakeVertexData();

            // Update triangle vertex indices
            std::vector<DiffuseTriangle> triangles;

            IndexType* indices = meshAsset->GetIndices();

            for (uint32_t f = 0; f < numTriangles; ++f)
            {
                uint32_t i = f * 3;

                DiffuseTriangle tri;
                tri.mVertexIndices = glm::uvec3(indices[i], indices[i + 1], indices[i + 2]);
                triangles.push_back(tri);
            }

            size_t triBufferSize = sizeof(DiffuseTriangle) * triangles.size();
            if (mBakeDiffuseTriangleBuffer->GetSize() < triBufferSize)
            {
                GetDestroyQueue()->Destroy(mBakeDiffuseTriangleBuffer);
                mBakeDiffuseTriangleBuffer = nullptr;
                mBakeDiffuseTriangleBuffer = new Buffer(BufferType::Storage, triBufferSize, "Bake Diffuse Triangle Buffer", nullptr, false);
            }

            mBakeDiffuseTriangleBuffer->Update(triangles.data(), triBufferSize);

            size_t avgBufferSize = sizeof(VertexLightData) * numVerts;
            if (mBakeAverageBuffer->GetSize() < avgBufferSize)
            {
                GetDestroyQueue()->Destroy(mBakeAverageBuffer);
                mBakeAverageBuffer = nullptr;
                mBakeAverageBuffer = new Buffer(BufferType::Storage, avgBufferSize, "Light Bake Vertex Buffer", nullptr, true);
            }
        }

        // Update uniform buffer
        RayTraceUniforms uniforms;
        uniforms.mNumTriangles = 0;
        uniforms.mNumMeshes = 0;
        uniforms.mNumLights = 0;
        uniforms.mMaxBounces = Renderer::Get()->GetBakeMaxBounces();
        uniforms.mRaysPerThread = Renderer::Get()->GetBakeRaysPerVertex();
        uniforms.mAccumulatedFrames = mAccumulatedFrames;
        uniforms.mNumBakeVertices = numVerts;
        uniforms.mNumBakeTriangles = numTriangles;
        uniforms.mShadowBias = Renderer::Get()->GetBakeShadowBias();
        uniforms.mDiffuseDirect = (mAccumulatedFrames < Renderer::Get()->GetBakeDiffuseDirectPasses());
        uniforms.mDiffuseIndirect = (mAccumulatedFrames < Renderer::Get()->GetBakeDiffuseIndirectPasses());
        uniforms.mSkyZenithColor = Renderer::Get()->GetSkyZenithColor();
        uniforms.mSkyHorizonColor = Renderer::Get()->GetSkyHorizonColor();
        uniforms.mGroundColor = Renderer::Get()->GetGroundColor();
        mRayTraceUniformBuffer->Update(&uniforms, sizeof(RayTraceUniforms));

        VkCommandBuffer cb = GetCommandBuffer();


        // Average First
        {
            GetVulkanContext()->SetComputeShader("LightBakeAverage.comp");

            GetVulkanContext()->CommitPipeline();
            BindBakeDiffuseDescriptorSet();

            vkCmdDispatch(cb, (numVerts + 31) / 32, 1, 1);
        }

        // Barrier so we can safely read the averages during the diffuse dispatch
        {
            VkBufferMemoryBarrier bufferBarrier = {};
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.buffer = mBakeAverageBuffer->Get();
            bufferBarrier.offset = 0;
            bufferBarrier.size = VK_WHOLE_SIZE;
            vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                0, nullptr,
                1, &bufferBarrier,
                0, nullptr);
        }

        // Diffuse Second
        {

            GetVulkanContext()->SetComputeShader("LightBakeDiffuse.comp");

            GetVulkanContext()->CommitPipeline();
            BindBakeDiffuseDescriptorSet();

            vkCmdDispatch(cb, (numVerts + 31) / 32, 1, 1);
        }

        mBakedFrame = (int64_t)Renderer::Get()->GetFrameNumber();
        mAccumulatedFrames++;
    }
}

static void AssignInstanceColors(StaticMesh3D* meshComp, std::vector<glm::vec4>& colors)
{
    std::vector<uint32_t> instanceColors;
    uint32_t numVerts = (uint32_t)colors.size();

    bool texBlend = false;
    StaticMesh* mesh = meshComp->GetStaticMesh();
    VertexColor* colorVerts = (mesh && mesh->HasVertexColor()) ? mesh->GetColorVertices() : nullptr;
    Material* material = meshComp->GetMaterial();
    if (material && mesh && colorVerts && mesh->GetNumVertices() == numVerts)
    {
        MaterialLite* matLite = material->As<MaterialLite>();
        texBlend = (matLite && matLite->GetVertexColorMode() == VertexColorMode::TextureBlend);
    }

    for (uint32_t v = 0; v < numVerts; ++v)
    {
        glm::vec4 directClamped = glm::clamp(
            colors[v] / LIGHT_BAKE_SCALE,
            glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Alpha should be the average of R/G/B
        // R G B components should remain as they are on the mesh
        if (texBlend)
        {
            float luminance = (directClamped.r + directClamped.g + directClamped.b) / 3.0f;
            directClamped.a = luminance;
        }

        uint32_t color32 = ColorFloat4ToUint32(directClamped);

        if (texBlend)
        {
            color32 = (color32 & (0xff000000)) | (colorVerts[v].mColor & (0x00ffffff));
        }

        instanceColors.push_back(color32);
    }

    meshComp->SetInstanceColors(instanceColors, true);
}

void RayTracer::ReadbackLightBakeResults()
{
    uint32_t curFrame = Renderer::Get()->GetFrameNumber();
    bool resultsReady = (mBakedFrame != -1) && (curFrame - MAX_FRAMES >= mBakedFrame);

    if (resultsReady)
    {
        StaticMesh3D* meshComp = nullptr;

        if (mBakingCompIndex >= 0 && mBakingCompIndex < mLightBakeNodes.size())
        {
            meshComp = mLightBakeNodes[mBakingCompIndex].Get<StaticMesh3D>();
        }

        // Assuming the component still exists, readback the baked light data.
        if (meshComp != nullptr &&
            meshComp->GetStaticMesh() != nullptr)
        {
            uint32_t numVerts = meshComp->GetStaticMesh()->GetNumVertices();
            OCT_ASSERT(mLightBakeVertexBuffer->GetSize() >= numVerts * sizeof(LightBakeVertex));

            LightBakeVertex* verts = (LightBakeVertex*)mLightBakeVertexBuffer->Map();
            std::vector<glm::vec4>& directColors = mLightBakeResults[mBakingCompIndex].mDirectColors;
            std::vector<glm::vec4>& indirectColors = mLightBakeResults[mBakingCompIndex].mIndirectColors;

            bool writeDirect = (mLightBakePhase == LightBakePhase::Direct || mLightBakePhase == LightBakePhase::Diffuse);
            bool writeIndirect = (mLightBakePhase == LightBakePhase::Indirect || mLightBakePhase == LightBakePhase::Diffuse);

            if (writeDirect)
            {
                directColors.clear();
            }

            if (writeIndirect)
            {
                indirectColors.clear();
            }

            for (uint32_t i = 0; i < numVerts; ++i)
            {
                LightBakeVertex& vert = verts[i];

                if (writeDirect)
                {
                    directColors.push_back(vert.mDirectLight);
                }

                if (writeIndirect)
                {
                    indirectColors.push_back(vert.mIndirectLight);
                }
            }

            mLightBakeVertexBuffer->Unmap();

            // Go ahead and apply the instance colors to the static mesh if we finished a direct bake.
            // It is important that the direct lighting is applied to the mesh before entering the indirect light pass.
            if (mLightBakePhase == LightBakePhase::Direct)
            {
                // In case we don't bake indirect light (0 iterations), then just allocate
                // default constructed colors so we don't get an access violation.
                indirectColors.resize(directColors.size());

                AssignInstanceColors(meshComp, directColors);
            }
        }

        mBakedFrame = -1;
        mBakingCompIndex = -1;
    }
}

void RayTracer::FinalizeLightBake()
{
    // Add the direct and indirect light colors and convert the result to uint32_t format.

    for (uint32_t c = 0; c < mLightBakeNodes.size(); ++c)
    {
        StaticMesh3D* meshComp = mLightBakeNodes[c].Get<StaticMesh3D>();

        if (meshComp != nullptr &&
            meshComp->GetStaticMesh() != nullptr)
        {
            const LightBakeResult& result = mLightBakeResults[c];
            uint32_t numVerts = meshComp->GetStaticMesh()->GetNumVertices();

            // Ensure the vertex counts match
            if (numVerts == result.mDirectColors.size() &&
                numVerts == result.mIndirectColors.size())
            {

                std::vector<glm::vec4> combinedColors;
                combinedColors.resize(numVerts);
                for (uint32_t v = 0; v < numVerts; ++v)
                {
                    combinedColors[v] = result.mDirectColors[v] + result.mIndirectColors[v];
                }

                AssignInstanceColors(meshComp, combinedColors);
            }
        }
    }
}

void RayTracer::BindPathTraceDescriptorSet()
{
    VkCommandBuffer cb = GetCommandBuffer();

    DescriptorSet::Begin("PathTrace DS")
        .WriteUniformBuffer(0, mRayTraceUniformBuffer)
        .WriteStorageBuffer(1, mRayTraceTriangleBuffer)
        .WriteStorageBuffer(2, mRayTraceMeshBuffer)
        .WriteStorageBuffer(3, mRayTraceLightBuffer)
        .WriteImageArray(4, mTextureImages)
        .WriteStorageImage(5, mPathTraceImage)
        .WriteStorageBuffer(6, mLightBakeVertexBuffer)
        .Build()
        .Bind(cb, 1);
}

void RayTracer::BindBakeDiffuseDescriptorSet()
{
    VkCommandBuffer cb = GetCommandBuffer();

    DescriptorSet::Begin("PathTrace DS")
        .WriteUniformBuffer(0, mRayTraceUniformBuffer)
        .WriteStorageBuffer(1, mLightBakeVertexBuffer)
        .WriteStorageBuffer(2, mBakeDiffuseTriangleBuffer)
        .WriteStorageBuffer(3, mBakeAverageBuffer)
        .Build()
        .Bind(cb, 1);
}

