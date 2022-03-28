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

StaticMesh::StaticMesh() :
    mMaterial(nullptr),
    mNumVertices(0),
    mNumIndices(0),
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
            vertices[i].mTexcoord = stream.ReadVec2();
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
            vertices[i].mTexcoord = stream.ReadVec2();
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
    assert(numCollisionShapes <= MAX_COLLISION_SHAPES);
    btCollisionShape* collisionShapes[MAX_COLLISION_SHAPES] = {};
    btTransform collisionTransforms[MAX_COLLISION_SHAPES] = {};

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

    SetCollisionShapes(numCollisionShapes, collisionShapes, collisionTransforms, compound);

    mBounds.mCenter = stream.ReadVec3();
    mBounds.mRadius = stream.ReadFloat();
}

void StaticMesh::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);
    
#if EDITOR
    stream.WriteUint32(mNumVertices);
    stream.WriteUint32(mNumIndices);

    stream.WriteAsset(mMaterial);
    stream.WriteBool(mGenerateTriangleCollisionMesh);
    stream.WriteBool(mHasVertexColor);

    if (mHasVertexColor)
    {
        VertexColor* vertices = GetColorVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            stream.WriteVec3(vertices[i].mPosition);
            stream.WriteVec2(vertices[i].mTexcoord);
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
            stream.WriteVec2(vertices[i].mTexcoord);
            stream.WriteVec3(vertices[i].mNormal);
        }
    }

    for (uint32_t i = 0; i < mNumIndices; ++i)
    {
        stream.WriteUint32(mIndices[i]);
    }

    // Collision shapes
    uint32_t numCollisionShapes = 0;
    btCollisionShape* collisionShapes[MAX_COLLISION_SHAPES] = {};
    btCompoundShape* compoundShape = nullptr;

    if (mCollisionShape != nullptr)
    {
        if (mCollisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
        {
            compoundShape = static_cast<btCompoundShape*>(mCollisionShape);
            numCollisionShapes = (uint32_t) compoundShape->getNumChildShapes();
            assert(numCollisionShapes >= 1 && numCollisionShapes < MAX_COLLISION_SHAPES);

            for (uint32_t i = 0; i < numCollisionShapes; ++i)
            {
                collisionShapes[i] = compoundShape->getChildShape(i);
            }
        }
        else
        {
            collisionShapes[0] = mCollisionShape;
            numCollisionShapes = 1;
        }
    }

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

    assert(mNumVertices <= MAX_MESH_VERTEX_COUNT); // Vertex index must fit into IndexType width.
    GFX_CreateStaticMeshResource(
        this,
        mHasVertexColor,
        mNumVertices,
        mHasVertexColor ? (void*)GetColorVertices() : (void*)GetVertices(),
        mNumIndices,
        mIndices);

    if (mGenerateTriangleCollisionMesh)
    {
        CreateTriangleCollisionShape(mNumVertices, mVertices, mNumIndices, mIndices);
    }
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

    ResizeVertexArray(0);
    ResizeIndexArray(0);

    mMaterial = nullptr;
}

void StaticMesh::Import(const std::string& path)
{
#if EDITOR
    Asset::Import(path);

    // Loads a .DAE file and loads the first mesh in the mesh library.
    if (mResource.mVertexBuffer == VK_NULL_HANDLE)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs);

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
        uint32_t numCollisionMeshes = 0;
        const aiMesh* collisionMeshes[MAX_COLLISION_SHAPES] = {};

        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[i];
            const char* meshName = mesh->mName.C_Str();

            if (strncmp(meshName, "UCX", 3) == 0 ||
                strncmp(meshName, "UBX", 3) == 0 ||
                strncmp(meshName, "USP", 3) == 0)
            {
                if (numCollisionMeshes < MAX_COLLISION_SHAPES)
                {
                    collisionMeshes[numCollisionMeshes] = mesh;
                    numCollisionMeshes++;
                }
                else
                {
                    LogError("More than %d collision meshes found", MAX_COLLISION_SHAPES);
                }
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

        Create(scene, *mainMesh, numCollisionMeshes, collisionMeshes);
    }
#endif
}

void StaticMesh::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Material", this, &mMaterial, 1, nullptr, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Generate Triangle Collision Mesh", this, &mGenerateTriangleCollisionMesh));
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
    assert(!mHasVertexColor);
    return reinterpret_cast<Vertex*>(mVertices);
}

VertexColor* StaticMesh::GetColorVertices()
{
    assert(mHasVertexColor);
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
    return mGenerateTriangleCollisionMesh ? mTriangleCollisionShape : nullptr;
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
        if (numCollisionShapes > MAX_COLLISION_SHAPES)
        {
            LogError("Max collision shapes exceeded! Num: %d, Max: %d", numCollisionShapes, MAX_COLLISION_SHAPES);
            numCollisionShapes = MAX_COLLISION_SHAPES;
        }

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
        assert(mCollisionMeshes.size() == 0);
        for (uint32_t i = 0; i < numCollisionShapes; ++i)
        {
            CreateCollisionMesh(collisionShapes[i]);
        }
#endif
    }
}

void StaticMesh::SetGenerateTriangleCollisionMesh(bool generate)
{
    mGenerateTriangleCollisionMesh = generate;
}

uint32_t StaticMesh::GetVertexSize() const
{
    return mHasVertexColor ? sizeof(VertexColor) : sizeof(Vertex);
}

void StaticMesh::CreateTriangleCollisionShape(uint32_t numVertices,
                                              void* vertices,
                                              uint32_t numIndices,
                                              IndexType* indices)
{
    assert(mTriangleIndexVertexArray == nullptr);
    assert(mTriangleCollisionShape == nullptr);
    assert(numIndices % 3 == 0);

    mTriangleIndexVertexArray = new btTriangleIndexVertexArray();

    btIndexedMesh mesh;
    mesh.m_numTriangles = numIndices / 3;
    mesh.m_triangleIndexBase = (const unsigned char *)indices;
    mesh.m_triangleIndexStride = sizeof(IndexType) * 3;
    mesh.m_numVertices = numVertices;
    mesh.m_vertexBase = (const unsigned char *)vertices;
    mesh.m_vertexStride = GetVertexSize();

    mTriangleIndexVertexArray->addIndexedMesh(mesh, sizeof(IndexType) == 2 ? PHY_SHORT : PHY_INTEGER);

    bool useQuantizedAabbCompression = true;
    btVector3 aabbMin(-1000, -1000, -1000);
    btVector3 aabbMax( 1000,  1000,  1000);

    mTriangleCollisionShape = new btBvhTriangleMeshShape(mTriangleIndexVertexArray, useQuantizedAabbCompression, aabbMin, aabbMax);
    mTriangleInfoMap = new btTriangleInfoMap();
    btGenerateInternalEdgeInfo(mTriangleCollisionShape, mTriangleInfoMap);
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

    mHasVertexColor = meshData.GetNumColorChannels() > 0;

    // Get pointers to vertex attributes
    glm::vec3* positions = reinterpret_cast<glm::vec3*>(meshData.mVertices);
    glm::vec3* texcoords3D = reinterpret_cast<glm::vec3*>(meshData.mTextureCoords[0]);
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
            vertices[i].mPosition = glm::vec3(positions[i].x,
                positions[i].y,
                positions[i].z);
            vertices[i].mTexcoord = glm::vec2(texcoords3D[i].x,
                texcoords3D[i].y);
            vertices[i].mNormal = glm::vec3(normals[i].x,
                normals[i].y,
                normals[i].z);

            glm::vec4 color4f = glm::vec4(colors[i].r, colors[i].g, colors[i].b, colors[i].a);
            vertices[i].mColor = ColorFloat4ToUint32(color4f);
        }
    }
    else
    {
        Vertex* vertices = GetVertices();
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            vertices[i].mPosition = glm::vec3(positions[i].x,
                positions[i].y,
                positions[i].z);
            vertices[i].mTexcoord = glm::vec2(texcoords3D[i].x,
                texcoords3D[i].y);
            vertices[i].mNormal = glm::vec3(normals[i].x,
                normals[i].y,
                normals[i].z);
        }
    }


    ResizeIndexArray(mNumIndices);

    for (uint32_t i = 0; i < meshData.mNumFaces; ++i)
    {
        // Enforce triangulated faces
        assert(faces[i].mNumIndices == 3);
        mIndices[i * 3 + 0] = (IndexType) faces[i].mIndices[0];
        mIndices[i * 3 + 1] = (IndexType) faces[i].mIndices[1];
        mIndices[i * 3 + 2] = (IndexType) faces[i].mIndices[2];
    }

    // Next, create collision objects for the collision meshes.
    uint32_t numCollisionShapes = 0;
    btCollisionShape* collisionShapes[MAX_COLLISION_SHAPES] = {};
    btTransform collisionTransforms[MAX_COLLISION_SHAPES] = {};

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
            // Why do I need to reorder the quaternion values so that w comes first?
            bRotation = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
            bPosition = { position.x, position.y, position.z };

            bTransform = btTransform(bRotation, bPosition);
        }

        if (strncmp(colMesh->mName.C_Str(), "UBX", 3) == 0)
        {
            // Box collision shape
            collisionShapes[numCollisionShapes] = new btBoxShape(bScale);
            collisionTransforms[numCollisionShapes] = bTransform;
            ++numCollisionShapes;
        }
        else if (strncmp(colMesh->mName.C_Str(), "USP", 3) == 0)
        {
            // Sphere collision shape
            collisionShapes[numCollisionShapes] = new btSphereShape(bScale.x());
            collisionTransforms[numCollisionShapes] = bTransform;
            ++numCollisionShapes;

        }
        else if (strncmp(colMesh->mName.C_Str(), "UCX", 3) == 0)
        {
            // Convex collision shape
            collisionShapes[numCollisionShapes] = new btConvexHullShape(
                reinterpret_cast<float*>(colMesh->mVertices),
                colMesh->mNumVertices,
                sizeof(aiVector3D));
            collisionTransforms[numCollisionShapes] = btTransform(bRotation, bPosition);
            ++numCollisionShapes;
        }
        else
        {
            LogWarning("Unknown collision mesh type found");
        }
    }

    // Any imported mesh will have a compound collision shape.
    // Engine assets like SM_Cube and SM_Sphere will still use non-compound shapes, as those are 
    // explicitly set up during editor initialization.
    SetCollisionShapes(numCollisionShapes, collisionShapes, collisionTransforms, true);

    mMaterial = Renderer::Get()->GetDefaultMaterial();

    ComputeBounds();

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
