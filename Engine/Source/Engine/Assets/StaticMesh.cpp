#include "Assets/StaticMesh.h"
#include "Renderer.h"
#include "Vertex.h"
#include "AssetManager.h"
#include "Utilities.h"
#include "Log.h"

#include "Graphics/Graphics.h"

#if EDITOR
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"

using namespace std;

FORCE_LINK_DEF(StaticMesh);
DEFINE_ASSET(StaticMesh);

bool StaticMesh::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    StaticMesh* mesh = static_cast<StaticMesh*>(prop->mOwner);

    bool handled = false;

    if (prop->mName == "Generate Triangle Collision Mesh")
    {
        mesh->SetGenerateTriangleCollisionMesh(*((bool*)newValue));
        handled = true;
    }

    return handled;
}

StaticMesh::StaticMesh() :
    mMaterial(nullptr),
    mNumVertices(0),
    mNumIndices(0),
    mNumUvMaps(2),
    mVertices(nullptr),
    mIndices(nullptr),
    mCollisionShape(nullptr),
    mTriangleCollisionShape(nullptr),
    mTriangleIndexVertexArray(nullptr),
    mTriangleInfoMap(nullptr),
    mGenerateTriangleCollisionMesh(false),
    mHasVertexColor(false)
{
    mType = StaticMesh::GetStaticType();
}

StaticMesh::~StaticMesh()
{

}

void StaticMesh::CreateRaw(
    uint32_t numVertices,
    Vertex* vertices,
    uint32_t numIndices,
    IndexType* indices)
{
    mNumVertices = numVertices;
    mNumIndices = numIndices;
    mHasVertexColor = false;
    ResizeVertexArray(numVertices);
    ResizeIndexArray(numIndices);

    memcpy(mVertices, vertices, numVertices * GetVertexSize());
    memcpy(mIndices, indices, numIndices * sizeof(IndexType));

    Create();
}

StaticMeshResource* StaticMesh::GetResource()
{
    return &mResource;
}

Material* StaticMesh::GetMaterial()
{
    return mMaterial.Get<Material>();
}

void StaticMesh::SetMaterial(class Material* newMaterial)
{
    mMaterial = newMaterial;
}

void StaticMesh::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mNumVertices = stream.ReadUint32();
    mNumIndices = stream.ReadUint32();
    mNumUvMaps = stream.ReadUint32();

    stream.ReadAsset(mMaterial);

    if (mMaterial.Get() == nullptr)
    {
        mMaterial = Renderer::Get()->GetDefaultMaterial();
    }

    mGenerateTriangleCollisionMesh = stream.ReadBool();
    mHasVertexColor = stream.ReadBool();

    ResizeVertexArray(mNumVertices);

    if (mHasVertexColor)
    {
        VertexColor* vertices = GetColorVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            vertices[i].mPosition = stream.ReadVec3();
            vertices[i].mTexcoord0 = stream.ReadVec2();
            vertices[i].mTexcoord1 = stream.ReadVec2();
            vertices[i].mNormal = stream.ReadVec3();
            vertices[i].mColor = stream.ReadUint32();
        }
    }
    else
    {
        Vertex* vertices = GetVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            vertices[i].mPosition = stream.ReadVec3();
            vertices[i].mTexcoord0 = stream.ReadVec2();
            vertices[i].mTexcoord1 = stream.ReadVec2();
            vertices[i].mNormal = stream.ReadVec3();
        }
    }

    ResizeIndexArray(mNumIndices);
    for (uint32_t i = 0; i < mNumIndices; ++i)
    {
        mIndices[i] = (IndexType) stream.ReadUint32();
    }

    // Collision shapes
    bool compound = stream.ReadBool();
    uint32_t numCollisionShapes = stream.ReadUint32();
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<btTransform> collisionTransforms;

    collisionShapes.resize(numCollisionShapes);
    collisionTransforms.resize(numCollisionShapes);

    for (uint32_t i = 0; i < numCollisionShapes; ++i)
    {
        CollisionShape shapeType = CollisionShape(stream.ReadUint32());

        switch (shapeType)
        {
        case CollisionShape::Box:
        {
            glm::vec3 halfExtents(0, 0, 0);
            halfExtents.x = stream.ReadFloat();
            halfExtents.y = stream.ReadFloat();
            halfExtents.z = stream.ReadFloat();
            collisionShapes[i] = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
            break;
        }
        case CollisionShape::Sphere:
        {
            float radius = stream.ReadFloat();
            collisionShapes[i] = new btSphereShape(radius);
            break;
        }
        case CollisionShape::ConvexHull:
        {
            uint32_t numPoints = stream.ReadUint32();
            std::vector<glm::vec3> points;
            points.reserve(numPoints);

            for (uint32_t i = 0; i < numPoints; ++i)
            {
                points.push_back(stream.ReadVec3());
            }

            collisionShapes[i] = new btConvexHullShape(
                reinterpret_cast<float*>(points.data()),
                numPoints,
                sizeof(glm::vec3));
            break;
        }
        case CollisionShape::Empty:
        {
            collisionShapes[i] = new btEmptyShape();
            break;
        }
        default:
        {
            LogWarning("Unrecognized shape type in StaticMesh::LoadStream()");
            break;
        }
        }

        if (compound)
        {
            // Read rotation and position to reconstruct transform.
            btVector3 origin = GlmToBullet(stream.ReadVec3());
            btQuaternion rotation = GlmToBullet(stream.ReadQuat());
            collisionTransforms[i].setIdentity();
            collisionTransforms[i].setRotation(rotation);
            collisionTransforms[i].setOrigin(origin);
        }
    }

    SetCollisionShapes(numCollisionShapes, collisionShapes.data(), collisionTransforms.data(), compound);

    mBounds.mCenter = stream.ReadVec3();
    mBounds.mRadius = stream.ReadFloat();
}

void StaticMesh::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);
    
#if EDITOR
    stream.WriteUint32(mNumVertices);
    stream.WriteUint32(mNumIndices);
    stream.WriteUint32(mNumUvMaps);

    stream.WriteAsset(mMaterial);
    stream.WriteBool(mGenerateTriangleCollisionMesh);
    stream.WriteBool(mHasVertexColor);

    if (mHasVertexColor)
    {
        VertexColor* vertices = GetColorVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            stream.WriteVec3(vertices[i].mPosition);
            stream.WriteVec2(vertices[i].mTexcoord0);
            stream.WriteVec2(vertices[i].mTexcoord1);
            stream.WriteVec3(vertices[i].mNormal);
            stream.WriteUint32(vertices[i].mColor);
        }
    }
    else
    {
        Vertex* vertices = GetVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            stream.WriteVec3(vertices[i].mPosition);
            stream.WriteVec2(vertices[i].mTexcoord0);
            stream.WriteVec2(vertices[i].mTexcoord1);
            stream.WriteVec3(vertices[i].mNormal);
        }
    }

    for (uint32_t i = 0; i < mNumIndices; ++i)
    {
        stream.WriteUint32(mIndices[i]);
    }

    // Collision shapes
    uint32_t numCollisionShapes = 0;
    std::vector<btCollisionShape*> collisionShapes;
    btCompoundShape* compoundShape = nullptr;

    if (mCollisionShape != nullptr)
    {
        if (mCollisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
        {
            compoundShape = static_cast<btCompoundShape*>(mCollisionShape);
            numCollisionShapes = (uint32_t) compoundShape->getNumChildShapes();
            OCT_ASSERT(numCollisionShapes >= 1);

            for (uint32_t i = 0; i < numCollisionShapes; ++i)
            {
                collisionShapes.push_back(compoundShape->getChildShape(i));
            }
        }
        else
        {
            collisionShapes.push_back(mCollisionShape);
            numCollisionShapes = 1;
        }
    }

    OCT_ASSERT(collisionShapes.size() == numCollisionShapes);

    stream.WriteBool(compoundShape != nullptr);
    stream.WriteUint32(numCollisionShapes);

    for (uint32_t i = 0; i < numCollisionShapes; ++i)
    {
        int32_t shapeType = collisionShapes[i]->getShapeType();

        switch (shapeType)
        {
        case BOX_SHAPE_PROXYTYPE:
        {
            btBoxShape* boxShape = static_cast<btBoxShape*>(collisionShapes[i]);
            btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
            stream.WriteUint32(uint32_t(CollisionShape::Box));
            stream.WriteFloat(halfExtents.x());
            stream.WriteFloat(halfExtents.y());
            stream.WriteFloat(halfExtents.z());
            break;
        }
        case SPHERE_SHAPE_PROXYTYPE:
        {
            btSphereShape* sphereShape = static_cast<btSphereShape*>(collisionShapes[i]);
            stream.WriteUint32(uint32_t(CollisionShape::Sphere));
            stream.WriteFloat(sphereShape->getRadius());
            break;
        }
        case CONVEX_HULL_SHAPE_PROXYTYPE:
        {
            btConvexHullShape* convexShape = static_cast<btConvexHullShape*>(collisionShapes[i]);
            stream.WriteUint32(uint32_t(CollisionShape::ConvexHull));

            uint32_t numPoints = convexShape->getNumPoints();
            stream.WriteUint32(numPoints);

            const btVector3* points = convexShape->getPoints();
            for (uint32_t p = 0; p < numPoints; ++p)
            {
                glm::vec3 glmPoint = { points[p].x(), points[p].y(), points[p].z() };
                stream.WriteVec3(glmPoint);
            }
            break;
        }
        case EMPTY_SHAPE_PROXYTYPE:
        {
            stream.WriteUint32(uint32_t(CollisionShape::Empty));
            break;
        }
        default:
        {
            stream.WriteUint32(uint32_t(CollisionShape::Num));
            LogWarning("Unrecognized shape type in StaticMesh::SaveStream()");
        }
        }

        if (compoundShape != nullptr)
        {
            const btTransform& transform = compoundShape->getChildTransform(i);
            glm::vec3 origin = BulletToGlm(transform.getOrigin());
            glm::quat rotation = BulletToGlm(transform.getRotation());
            stream.WriteVec3(origin);
            stream.WriteQuat(rotation);

        }
    }

    stream.WriteVec3(mBounds.mCenter);
    stream.WriteFloat(mBounds.mRadius);
#endif
}

void StaticMesh::Create()
{
    Asset::Create();

    OCT_ASSERT(mNumVertices <= MAX_MESH_VERTEX_COUNT); // Vertex index must fit into IndexType width.
    GFX_CreateStaticMeshResource(
        this,
        mHasVertexColor,
        mNumVertices,
        mHasVertexColor ? (void*)GetColorVertices() : (void*)GetVertices(),
        mNumIndices,
        mIndices);

    if (ShouldGenerateTriangleCollision())
    {
        CreateTriangleCollisionShape();
    }

    ComputeBounds();
}

void StaticMesh::Destroy()
{
    Asset::Destroy();

#if CREATE_CONVEX_COLLISION_MESH
    for (StaticMesh* mesh : mCollisionMeshes)
    {
        delete mesh;
    }
    mCollisionMeshes.clear();
#endif

    GFX_DestroyStaticMeshResource(this);

    if (mCollisionShape != nullptr)
    {
        DestroyCollisionShape(mCollisionShape);
        mCollisionShape = nullptr;
    }

    DestroyTriangleCollisionShape();

    ResizeVertexArray(0);
    ResizeIndexArray(0);

    mMaterial = nullptr;
}

void StaticMesh::Import(const std::string& path, ImportOptions* options)
{
#if EDITOR
    Asset::Import(path, options);

    // Loads a .DAE file and loads the first mesh in the mesh library.
    if (mResource.mVertexBuffer == VK_NULL_HANDLE)
    {
        Assimp::Importer importer;

        // TODO: If supporting lightmap textures and automatic lightmap UV generation, then do not
        // join identical vertices. Auto-generated lightmap UVs will have all-unique vertices.
        const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if (scene == nullptr)
        {
            LogError("Failed to load dae file");
            return;
        }

        if (scene->mNumMeshes < 1)
        {
            LogError("Failed to find any meshes in dae file");
            return;
        }

        const aiMesh* mainMesh = nullptr;
        std::vector<const aiMesh*> collisionMeshes;

        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[i];
            const char* meshName = mesh->mName.C_Str();

            if (strncmp(meshName, "UCX", 3) == 0 ||
                strncmp(meshName, "UBX", 3) == 0 ||
                strncmp(meshName, "USP", 3) == 0)
            {
                collisionMeshes.push_back(mesh);
            }
            else
            {
                if (mainMesh == nullptr)
                {
                    mainMesh = mesh;
                }
                else
                {
                    LogError("More than one non-collision mesh found");
                }
            }
        }

        Create(scene, *mainMesh, (uint32_t)collisionMeshes.size(), collisionMeshes.data());
    }
#endif
}

void StaticMesh::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Material", this, &mMaterial, 1, nullptr, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Generate Triangle Collision Mesh", this, &mGenerateTriangleCollisionMesh, 1, HandlePropChange));
}

glm::vec4 StaticMesh::GetTypeColor()
{
    return glm::vec4(0.3f, 1.0f, 0.8f, 1.0f);
}

const char* StaticMesh::GetTypeName()
{
    return "StaticMesh";
}

const char* StaticMesh::GetTypeImportExt()
{
    return ".dae";
}

uint32_t StaticMesh::GetNumIndices() const
{
    return mNumIndices;
}

uint32_t StaticMesh::GetNumFaces() const
{
    return mNumIndices / 3;
}

uint32_t StaticMesh::GetNumVertices() const
{
    return mNumVertices;
}

bool StaticMesh::HasVertexColor() const
{
    return mHasVertexColor;
}

Vertex* StaticMesh::GetVertices()
{
    OCT_ASSERT(!mHasVertexColor);
    return reinterpret_cast<Vertex*>(mVertices);
}

VertexColor* StaticMesh::GetColorVertices()
{
    OCT_ASSERT(mHasVertexColor);
    return reinterpret_cast<VertexColor*>(mVertices);
}

IndexType* StaticMesh::GetIndices()
{
    return mIndices;
}

Bounds StaticMesh::GetBounds() const
{
    return mBounds;
}

btBvhTriangleMeshShape* StaticMesh::GetTriangleCollisionShape()
{
    return ShouldGenerateTriangleCollision() ? mTriangleCollisionShape : nullptr;
}

btCollisionShape* StaticMesh::GetCollisionShape()
{
    return mCollisionShape;
}

void StaticMesh::SetCollisionShape(btCollisionShape* shape)
{
    if (mCollisionShape != nullptr)
    {
        delete mCollisionShape;
        mCollisionShape = nullptr;
    }

    mCollisionShape = shape;
}

void StaticMesh::SetCollisionShapes(uint32_t numCollisionShapes, btCollisionShape** collisionShapes, btTransform* transforms, bool compound)
{
    if (numCollisionShapes > 0)
    {
        if (numCollisionShapes == 1 && !compound)
        {
            SetCollisionShape(collisionShapes[0]);
        }
        else
        {
            btCompoundShape* compoundShape = new btCompoundShape();

            for (uint32_t i = 0; i < numCollisionShapes; ++i)
            {
                compoundShape->addChildShape(transforms[i], collisionShapes[i]);
            }

            // Do we want to initialize the principle axis transform? Not really sure what it is.
            // I think it's related to physics simulation.
#if 0
            btScalar masses[3] = { 1, 1, 1 };
            btTransform principal;
            btVector3 inertia;
            compoundShape->calculatePrincipalAxisTransform(masses, principal, inertia);
#endif

            SetCollisionShape(compoundShape);
        }

#if CREATE_CONVEX_COLLISION_MESH
        OCT_ASSERT(mCollisionMeshes.size() == 0);
        for (uint32_t i = 0; i < numCollisionShapes; ++i)
        {
            CreateCollisionMesh(collisionShapes[i]);
        }
#endif
    }
}

void StaticMesh::SetGenerateTriangleCollisionMesh(bool generate)
{
    if (mGenerateTriangleCollisionMesh != generate)
    {
        mGenerateTriangleCollisionMesh = generate;

        if (ShouldGenerateTriangleCollision())
        {
            CreateTriangleCollisionShape();
        }
        else
        {
            DestroyTriangleCollisionShape();
        }
    }
}

bool StaticMesh::IsTriangleCollisionMeshEnabled() const
{
    return mGenerateTriangleCollisionMesh;
}

uint32_t StaticMesh::GetVertexSize() const
{
    return mHasVertexColor ? sizeof(VertexColor) : sizeof(Vertex);
}

bool StaticMesh::ShouldGenerateTriangleCollision() const
{
#if EDITOR
    // Always generate it in Editor. For vertex color and instance painting, we want to use the 
    // triangle collision data for placing the paint sphere reticle.
    return true;
#else
    return mGenerateTriangleCollisionMesh;
#endif
}

void StaticMesh::CreateTriangleCollisionShape()
{
    OCT_ASSERT(mNumIndices % 3 == 0);

    // Don't do anything if we already have triangle collision data generated
    // Note: In EDITOR, we always generate triangle collision data even if it's disabled.
    if (mTriangleIndexVertexArray == nullptr &&
        mTriangleCollisionShape == nullptr)
    {
        mTriangleIndexVertexArray = new btTriangleIndexVertexArray();

        btIndexedMesh mesh;
        mesh.m_numTriangles = mNumIndices / 3;
        mesh.m_triangleIndexBase = (const unsigned char*)mIndices;
        mesh.m_triangleIndexStride = sizeof(IndexType) * 3;
        mesh.m_numVertices = mNumVertices;
        mesh.m_vertexBase = (const unsigned char*)mVertices;
        mesh.m_vertexStride = GetVertexSize();

        mTriangleIndexVertexArray->addIndexedMesh(mesh, sizeof(IndexType) == 2 ? PHY_SHORT : PHY_INTEGER);

        bool useQuantizedAabbCompression = true;

        mTriangleCollisionShape = new btBvhTriangleMeshShape(mTriangleIndexVertexArray, useQuantizedAabbCompression);
        mTriangleInfoMap = new btTriangleInfoMap();
        btGenerateInternalEdgeInfo(mTriangleCollisionShape, mTriangleInfoMap);
    }
}

void StaticMesh::DestroyTriangleCollisionShape()
{
    if (mTriangleInfoMap != nullptr)
    {
        delete mTriangleInfoMap;
        mTriangleInfoMap = nullptr;
    }

    if (mTriangleCollisionShape != nullptr)
    {
        delete mTriangleCollisionShape;
        mTriangleCollisionShape = nullptr;
    }

    if (mTriangleIndexVertexArray != nullptr)
    {
        delete mTriangleIndexVertexArray;
        mTriangleIndexVertexArray = nullptr;
    }
}

void StaticMesh::ResizeVertexArray(uint32_t newSize)
{
    if (mVertices != nullptr)
    {
        free(mVertices);
        mVertices = nullptr;
    }

    if (newSize > 0)
    {
        if (mHasVertexColor)
        {
            mVertices = malloc(sizeof(VertexColor) * newSize);
        }
        else
        {
            mVertices = malloc(sizeof(Vertex) * newSize);
        }
    }
}

void StaticMesh::ResizeIndexArray(uint32_t newSize)
{
    if (mIndices != nullptr)
    {
        free(mIndices);
        mIndices = nullptr;
    }

    if (newSize > 0)
    {
        mIndices = (IndexType*)malloc(sizeof(IndexType) * newSize);
    }
}

void StaticMesh::ComputeBounds()
{
    if (mNumVertices == 0)
    {
        mBounds.mCenter = { 0.0f, 0.0f, 0.0f };
        mBounds.mRadius = 0.0f;
        return;
    }

    bool hasColor = HasVertexColor();

    glm::vec3 boxMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    glm::vec3 boxMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        glm::vec3 pos = 
            hasColor ? 
            GetColorVertices()[i].mPosition : 
            GetVertices()[i].mPosition;

        boxMin = glm::min(boxMin, pos);
        boxMax = glm::max(boxMax, pos);
    }

    mBounds.mCenter = (boxMin + boxMax) / 2.0f;

    float maxDist = 0.0f;

    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        glm::vec3 pos =
            hasColor ?
            GetColorVertices()[i].mPosition :
            GetVertices()[i].mPosition;

        float dist = glm::distance(pos, mBounds.mCenter);
        maxDist = glm::max(maxDist, dist);
    }

    mBounds.mRadius = maxDist;
}

#if EDITOR

const aiNode* FindMeshNode(const aiNode* node, const aiMesh* mesh)
{
    const aiNode* retNode = nullptr;
    if (mesh->mName == node->mName)
    {
        retNode = node;
    }
    else
    {
        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            retNode = FindMeshNode(node->mChildren[i], mesh);

            if (retNode != nullptr)
            {
                break;
            }
        }
    }

    return retNode;
};

void StaticMesh::Create(
    const aiScene* scene,
    const aiMesh& meshData,
    uint32_t numCollisionMeshes,
    const aiMesh** collisionMeshes)
{
    // First, handle the main mesh that we want to render.
    if (meshData.mNumVertices == 0 ||
        meshData.mNumFaces == 0)
    {
        return;
    }

    mNumVertices = meshData.mNumVertices;
    mNumIndices = meshData.mNumFaces * 3;
    
    // Always set the num uv maps to 2 since we are storing two sets of UVs no matter what.
    mNumUvMaps = 2;
    //mNumUvMaps = glm::clamp(meshData.GetNumUVChannels(), 0u, MAX_UV_MAPS - 1u);

    mHasVertexColor = meshData.GetNumColorChannels() > 0;

    // Get pointers to vertex attributes
    glm::vec3* positions = reinterpret_cast<glm::vec3*>(meshData.mVertices);
    glm::vec3* texcoords0 = meshData.HasTextureCoords(0) ? reinterpret_cast<glm::vec3*>(meshData.mTextureCoords[0]) : nullptr;
    glm::vec3* texcoords1 = meshData.HasTextureCoords(1) ? reinterpret_cast<glm::vec3*>(meshData.mTextureCoords[1]) : texcoords0;
    glm::vec3* normals = reinterpret_cast<glm::vec3*>(meshData.mNormals);
    glm::vec4* colors = mHasVertexColor ? reinterpret_cast<glm::vec4*>(meshData.mColors[0]) : nullptr;

    aiFace* faces = meshData.mFaces;

    ResizeVertexArray(mNumVertices);

    // Create an interleaved VBO

    if (mHasVertexColor)
    {
        VertexColor* vertices = GetColorVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            vertices[i].mPosition = glm::vec3(positions[i].x, positions[i].y, positions[i].z);
            vertices[i].mTexcoord0 = texcoords0 ? glm::vec2(texcoords0[i].x, texcoords0[i].y) : glm::vec2(0.0f, 0.0f);
            vertices[i].mTexcoord1 = texcoords1 ? glm::vec2(texcoords1[i].x, texcoords1[i].y) : glm::vec2(0.0f, 0.0f);
            vertices[i].mNormal = glm::vec3(normals[i].x, normals[i].y, normals[i].z);

            glm::vec4 color4f = glm::vec4(colors[i].r, colors[i].g, colors[i].b, colors[i].a);
            vertices[i].mColor = ColorFloat4ToUint32(color4f);
        }
    }
    else
    {
        Vertex* vertices = GetVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            vertices[i].mPosition = glm::vec3(positions[i].x, positions[i].y, positions[i].z);
            vertices[i].mTexcoord0 = texcoords0 ? glm::vec2(texcoords0[i].x, texcoords0[i].y) : glm::vec2(0.0f, 0.0f);
            vertices[i].mTexcoord1 = texcoords1 ? glm::vec2(texcoords1[i].x, texcoords1[i].y) : glm::vec2(0.0f, 0.0f);
            vertices[i].mNormal = glm::vec3(normals[i].x, normals[i].y, normals[i].z);
        }
    }


    ResizeIndexArray(mNumIndices);

    for (uint32_t i = 0; i < meshData.mNumFaces; ++i)
    {
        // Enforce triangulated faces
        OCT_ASSERT(faces[i].mNumIndices == 3);
        mIndices[i * 3 + 0] = (IndexType) faces[i].mIndices[0];
        mIndices[i * 3 + 1] = (IndexType) faces[i].mIndices[1];
        mIndices[i * 3 + 2] = (IndexType) faces[i].mIndices[2];
    }

    // Next, create collision objects for the collision meshes.
    uint32_t numCollisionShapes = 0;
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<btTransform> collisionTransforms;

    for (uint32_t i = 0; i < numCollisionMeshes; ++i)
    {
        const aiMesh* colMesh = collisionMeshes[i];

        const aiNode* node = FindMeshNode(scene->mRootNode, colMesh);

        btVector3 bScale = { 1.0f, 1.0f, 1.0f };
        btQuaternion bRotation = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
        btVector3 bPosition = { 0.0f, 0.0f, 0.0f };
        btTransform bTransform = btTransform::getIdentity();

        if (node != nullptr)
        {
            aiVector3D scale;
            aiQuaternion rotation;
            aiVector3D position;
            node->mTransformation.Decompose(scale, rotation, position);

            bScale = { scale.x, scale.y, scale.z };
            bRotation = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
            bPosition = { position.x, position.y, position.z };

            bTransform = btTransform(bRotation, bPosition);
        }
        else
        {
            LogWarning("Could not find collision mesh node. Please ensure mesh and node have exact same name.");
        }

        if (strncmp(colMesh->mName.C_Str(), "UBX", 3) == 0)
        {
            // Box collision shape
            collisionShapes.push_back(new btBoxShape(bScale));
            collisionTransforms.push_back(bTransform);
            ++numCollisionShapes;
        }
        else if (strncmp(colMesh->mName.C_Str(), "USP", 3) == 0)
        {
            // Sphere collision shape
            collisionShapes.push_back(new btSphereShape(bScale.x()));
            collisionTransforms.push_back(bTransform);
            ++numCollisionShapes;

        }
        else if (strncmp(colMesh->mName.C_Str(), "UCX", 3) == 0)
        {
            // Convex collision shape
            collisionShapes.push_back(new btConvexHullShape(
                reinterpret_cast<float*>(colMesh->mVertices),
                colMesh->mNumVertices,
                sizeof(aiVector3D)));
            collisionTransforms.push_back(btTransform(bRotation, bPosition));
            ++numCollisionShapes;
        }
        else
        {
            LogWarning("Unknown collision mesh type found");
        }
    }

    OCT_ASSERT(collisionShapes.size() == numCollisionShapes);
    OCT_ASSERT(collisionTransforms.size() == numCollisionShapes);

    // Any imported mesh will have a compound collision shape.
    // Engine assets like SM_Cube and SM_Sphere will still use non-compound shapes, as those are 
    // explicitly set up during editor initialization.
    // Note: Even when a mesh has only one collision shape, we still need to use a compound shape parent
    // so that it can receive there correct transform. Otherwise it will always be positioned at the origin.
    SetCollisionShapes(numCollisionShapes, collisionShapes.data(), collisionTransforms.data(), true);

    mMaterial = Renderer::Get()->GetDefaultMaterial();

    Create();
}

#endif // EDITOR


#if CREATE_CONVEX_COLLISION_MESH

void StaticMesh::CreateCollisionMesh(btCollisionShape* collisionShape)
{
    if (collisionShape->getShapeType() == CONVEX_HULL_SHAPE_PROXYTYPE)
    {
        btConvexHullShape* convexShape = static_cast<btConvexHullShape*>(collisionShape);

        uint32_t numPoints = convexShape->getNumPoints();
        const btVector3* points = convexShape->getPoints();
        std::vector<Vertex> vertices;
        std::vector<IndexType> indices;

        for (uint32_t f = 0; f < numPoints / 3; ++f)
        {
            for (uint32_t v = 0; v < 3; ++v)
            {
                Vertex vert = {};
                btVector3 point = points[f * 3 + v];
                vert.mPosition = { point.x(), point.y(), point.z() };

                vertices.push_back(vert);
                indices.push_back(f * 3 + v);
            }
        }

        StaticMesh* newStaticMesh = new StaticMesh();
        newStaticMesh->CreateRaw(uint32_t(vertices.size()), vertices.data(), uint32_t(indices.size()), indices.data());
        newStaticMesh->SetName("Debug Collision Mesh");
        mCollisionMeshes.push_back(newStaticMesh);
    }
}
#endif
