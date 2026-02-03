#include "Nodes/3D/Spline3d.h"
#include "Stream.h"
#include "Maths.h"
#include "Property.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Audio3d.h"
#include "AssetManager.h"
#include "Engine.h"
#include "World.h"
#include "Line.h"
#include "NodePath.h"
#include <algorithm>
#include <cstdio>

#if EDITOR
#include "imgui.h"
#endif

DEFINE_NODE(Spline3D, Node3D);

static bool sGeneratePoint = false;

struct SplinePointNode
{
    int32_t index;
    Node3D* node;
};

static void GatherSplinePointNodes(Node* owner, std::vector<SplinePointNode>& outPoints)
{
    outPoints.clear();

    const std::vector<NodePtr>& children = owner->GetChildren();
    for (uint32_t i = 0; i < children.size(); ++i)
    {
        Node* child = children[i].Get();
        if (!child) continue;

        const std::string& name = child->GetName();
        if (name.rfind("point", 0) == 0)
        {
            int idx = 0;
            if (sscanf(name.c_str(), "point%d", &idx) == 1)
            {
                Node3D* n3d = child->As<Node3D>();
                if (n3d)
                {
                    outPoints.push_back({ idx, n3d });
                }
            }
        }
    }

    std::sort(outPoints.begin(), outPoints.end(), [](const SplinePointNode& a, const SplinePointNode& b) { return a.index < b.index; });
}

void Spline3D::Create()
{
    Node3D::Create();
    SetName("Spline");

    // Create initial point
    Box3D* p1 = CreateChild<Box3D>("point1");
    if (p1)
    {
        p1->SetExtents(glm::vec3(0.4f));
        p1->SetPosition(glm::vec3(0.0f));
    }
}

void Spline3D::Start()
{
    Node3D::Start();
    mTravel = 0.0f;

    Node* root = GetWorld() ? GetWorld()->GetRootNode() : nullptr;

    auto resolveAttachment = [&](NodePtrWeak& ptr)
    {
        Node* node = ptr.Get();
        if (!node || !root)
            return;

        // Build path from the root so we can resolve into the PIE world.
        std::string path = FindRelativeNodePath(root, node);
        if (!path.empty())
        {
            Node* resolved = ResolveNodePath(root, path);
            if (resolved)
            {
                ptr = ResolveWeakPtr(resolved);
            }
        }
    };

    resolveAttachment(mAttachmentCamera);
    resolveAttachment(mAttachmentStaticMesh);
    resolveAttachment(mAttachmentSkeletalMesh);
    resolveAttachment(mAttachmentParticle3D);
    resolveAttachment(mAttachmentPointLight);
    resolveAttachment(mAttachmentAudio3D);

    std::vector<SplinePointNode> points;
    GatherSplinePointNodes(this, points);
    if (points.size() > 0)
    {
        glm::vec3 startPos = points[0].node->GetWorldPosition();

        Node* camNode = mAttachmentCamera.Get();
        if (camNode && camNode->As<Camera3D>())
        {
            mOrigCamTransform = camNode->As<Node3D>()->GetTransform();
            camNode->As<Node3D>()->SetWorldPosition(startPos);
        }

        Node* meshNode = mAttachmentStaticMesh.Get();
        if (meshNode && meshNode->As<StaticMesh3D>())
        {
            mOrigStaticTransform = meshNode->As<Node3D>()->GetTransform();
            meshNode->As<Node3D>()->SetWorldPosition(startPos);
        }

        Node* skelNode = mAttachmentSkeletalMesh.Get();
        if (skelNode && skelNode->As<SkeletalMesh3D>())
        {
            mOrigSkeletalTransform = skelNode->As<Node3D>()->GetTransform();
            skelNode->As<Node3D>()->SetWorldPosition(startPos);
        }

        Node* partNode = mAttachmentParticle3D.Get();
        if (partNode && partNode->As<Particle3D>())
        {
            mOrigParticleTransform = partNode->As<Node3D>()->GetTransform();
            partNode->As<Node3D>()->SetWorldPosition(startPos);
        }

        Node* lightNode = mAttachmentPointLight.Get();
        if (lightNode && lightNode->As<PointLight3D>())
        {
            mOrigPointLightTransform = lightNode->As<Node3D>()->GetTransform();
            lightNode->As<Node3D>()->SetWorldPosition(startPos);
        }

        Node* audioNode = mAttachmentAudio3D.Get();
        if (audioNode && audioNode->As<Audio3D>())
        {
            mOrigAudioTransform = audioNode->As<Node3D>()->GetTransform();
            audioNode->As<Node3D>()->SetWorldPosition(startPos);
        }
    }
}

void Spline3D::Stop()
{
    Node3D::Stop();
    mPlaying = false;

    // Option A: do not touch editor nodes on Stop
}

void Spline3D::Play()
{
    mPlaying = true;
}

void Spline3D::StopPlayback()
{
    mPlaying = false;
}

void Spline3D::Tick(float deltaTime)
{
    Node3D::Tick(deltaTime);

    if (!IsPlaying() || !mPlaying)
        return;

    Node* camNode = mAttachmentCamera.Get();
    Node* meshNode = mAttachmentStaticMesh.Get();
    Node* skelNode = mAttachmentSkeletalMesh.Get();
    Node* partNode = mAttachmentParticle3D.Get();
    Node* lightNode = mAttachmentPointLight.Get();
    Node* audioNode = mAttachmentAudio3D.Get();

    bool hasCam = (camNode && camNode->As<Camera3D>());
    bool hasMesh = (meshNode && meshNode->As<StaticMesh3D>());
    bool hasSkel = (skelNode && skelNode->As<SkeletalMesh3D>());
    bool hasPart = (partNode && partNode->As<Particle3D>());
    bool hasLight = (lightNode && lightNode->As<PointLight3D>());
    bool hasAudio = (audioNode && audioNode->As<Audio3D>());

    if (!hasCam && !hasMesh && !hasSkel && !hasPart && !hasLight && !hasAudio)
        return;

    std::vector<SplinePointNode> points;
    GatherSplinePointNodes(this, points);
    if (points.size() < 2)
        return;

    // Build distances
    float totalLen = 0.0f;
    std::vector<float> segLens;
    segLens.resize(points.size() - 1 + (mCloseLoop ? 1 : 0));

    auto getPointSpeed = [&](const std::string& pointName)
    {
        for (uint32_t i = 0; i < mPointSpeedEntries.size(); ++i)
        {
            if (mPointSpeedEntries[i].name == pointName)
                return glm::max(0.001f, mPointSpeedEntries[i].speed);
        }
        return 1.0f;
    };

    for (uint32_t i = 1; i < points.size(); ++i)
    {
        glm::vec3 p0 = points[i - 1].node->GetWorldPosition();
        glm::vec3 p1 = points[i].node->GetWorldPosition();
        float len = glm::length(p1 - p0);
        float mult = getPointSpeed(points[i - 1].node->GetName());
        float effLen = len / mult;
        segLens[i - 1] = effLen;
        totalLen += effLen;
    }

    if (mCloseLoop)
    {
        glm::vec3 p0 = points.back().node->GetWorldPosition();
        glm::vec3 p1 = points.front().node->GetWorldPosition();
        float len = glm::length(p1 - p0);
        float mult = getPointSpeed(points.back().node->GetName());
        float effLen = len / mult;
        segLens[points.size() - 1] = effLen;
        totalLen += effLen;
    }

    if (totalLen <= 0.0001f)
        return;

    mTravel += mSpeed * deltaTime;

    if (mLoop)
    {
        while (mTravel >= totalLen)
            mTravel -= totalLen;
    }
    else
    {
        if (mTravel >= totalLen)
            mTravel = totalLen;
    }

    float dist = mTravel;
    uint32_t segIndex = 0;
    for (uint32_t i = 0; i < segLens.size(); ++i)
    {
        if (dist <= segLens[i])
        {
            segIndex = i;
            break;
        }
        dist -= segLens[i];
    }

    glm::vec3 a;
    glm::vec3 b;
    if (segIndex < points.size() - 1)
    {
        a = points[segIndex].node->GetWorldPosition();
        b = points[segIndex + 1].node->GetWorldPosition();
    }
    else
    {
        // closing segment
        a = points.back().node->GetWorldPosition();
        b = points.front().node->GetWorldPosition();
    }

    float segLen = segLens[segIndex];
    float t = (segLen > 0.0001f) ? (dist / segLen) : 0.0f;

    glm::vec3 pos = glm::mix(a, b, t);
    glm::vec3 tangent = glm::normalize(b - a);

    if (mSmoothCurve && points.size() >= 4)
    {
        auto getPointPos = [&](int idx)
        {
            if (mCloseLoop)
            {
                int count = (int)points.size();
                int wrapped = (idx % count + count) % count;
                return points[wrapped].node->GetWorldPosition();
            }
            idx = glm::clamp(idx, 0, (int)points.size() - 1);
            return points[idx].node->GetWorldPosition();
        };

        int p1 = (int)segIndex;
        int p2 = (int)segIndex + 1;
        int p0 = p1 - 1;
        int p3 = p2 + 1;
        glm::vec3 cp0 = getPointPos(p0);
        glm::vec3 cp1 = getPointPos(p1);
        glm::vec3 cp2 = getPointPos(p2);
        glm::vec3 cp3 = getPointPos(p3);
        pos = Spline3D::CatmullRom(cp0, cp1, cp2, cp3, t);
        tangent = Maths::SafeNormalize(Spline3D::CatmullRomTangent(cp0, cp1, cp2, cp3, t));
    }

    auto applyMove = [&](Node* node, bool face)
    {
        node->As<Node3D>()->SetWorldPosition(pos);
        if (mFaceTangent && face)
            node->As<Node3D>()->LookAt(pos + tangent, glm::vec3(0,1,0));
    };

    if (hasCam) applyMove(camNode, true);
    if (hasMesh) applyMove(meshNode, true);
    if (hasSkel) applyMove(skelNode, true);
    if (hasPart) applyMove(partNode, false);
    if (hasLight) applyMove(lightNode, false);
    if (hasAudio) applyMove(audioNode, false);
}

void Spline3D::Copy(Node* srcNode, bool recurse)
{
    if (recurse && srcNode)
    {
        uint32_t srcCount = srcNode->GetNumChildren();
        while (GetNumChildren() < srcCount)
        {
            Node* srcChild = srcNode->GetChild(GetNumChildren());
            if (srcChild)
            {
                CreateChild(srcChild->GetType());
            }
            else
            {
                break;
            }
        }

        while (GetNumChildren() > srcCount)
        {
            Node* child = GetChild(GetNumChildren() - 1);
            if (child)
            {
                child->Destroy();
            }
            else
            {
                break;
            }
        }
    }

    Node3D::Copy(srcNode, recurse);
}

glm::vec3 Spline3D::CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

glm::vec3 Spline3D::CatmullRomTangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    float t2 = t * t;
    return 0.5f * ((-p0 + p2) +
        2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
        3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2);
}

void Spline3D::SaveStream(Stream& stream, Platform platform)
{
    Node3D::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mPoints.size());
    for (uint32_t i = 0; i < mPoints.size(); ++i)
    {
        stream.WriteVec3(mPoints[i]);
    }

    // Point speed entries
    stream.WriteUint32((uint32_t)mPointSpeedEntries.size());
    for (uint32_t i = 0; i < mPointSpeedEntries.size(); ++i)
    {
        stream.WriteString(mPointSpeedEntries[i].name);
        stream.WriteFloat(mPointSpeedEntries[i].speed);
    }
}

void Spline3D::LoadStream(Stream& stream, Platform platform, uint32_t version)
{
    Node3D::LoadStream(stream, platform, version);

    uint32_t count = stream.ReadUint32();
    mPoints.resize(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        mPoints[i] = stream.ReadVec3();
    }

    // Point speed entries (if present)
    if (stream.GetPos() < stream.GetSize())
    {
        uint32_t speedCount = stream.ReadUint32();
        mPointSpeedEntries.resize(speedCount);
        for (uint32_t i = 0; i < speedCount; ++i)
        {
            stream.ReadString(mPointSpeedEntries[i].name);
            mPointSpeedEntries[i].speed = stream.ReadFloat();
        }
    }
}

void Spline3D::GatherProperties(std::vector<Property>& props)
{
    Node3D::GatherProperties(props);

    {
        SCOPED_CATEGORY("Spline");
        props.push_back(Property(DatumType::Bool, "Generate Point", this, &sGeneratePoint));
        props.push_back(Property(DatumType::Node, "Point Speed Target", this, &mPointSpeedTarget, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Point Speed", this, &mPointSpeedValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Global Speed", this, &mSpeed));
        props.push_back(Property(DatumType::Bool, "Play", this, &mPlaying, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Loop", this, &mLoop));
        props.push_back(Property(DatumType::Bool, "Close Spline", this, &mCloseLoop));
        props.push_back(Property(DatumType::Bool, "Smooth Curve", this, &mSmoothCurve));
        props.push_back(Property(DatumType::Bool, "Face Tangent", this, &mFaceTangent));
    }

    {
        SCOPED_CATEGORY("Attachments");
        props.push_back(Property(DatumType::Node, "Camera", this, &mAttachmentCamera, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Static Mesh", this, &mAttachmentStaticMesh, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Skeletal Mesh", this, &mAttachmentSkeletalMesh, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Particle", this, &mAttachmentParticle3D, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Point Light", this, &mAttachmentPointLight, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Audio", this, &mAttachmentAudio3D, 1, HandlePropChange));
    }
}

void Spline3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    Node3D::GatherProxyDraws(inoutDraws);

#if DEBUG_DRAW_ENABLED
    std::vector<SplinePointNode> points;
    GatherSplinePointNodes(this, points);

    World* world = GetWorld();
    if (world)
    {
        for (uint32_t i = 1; i < points.size(); ++i)
        {
            glm::vec3 p0 = points[i - 1].node->GetWorldPosition();
            glm::vec3 p1 = points[i].node->GetWorldPosition();
            world->AddLine(Line(p0, p1, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.01f));
        }

        if (mCloseLoop && points.size() > 2)
        {
            glm::vec3 p0 = points.back().node->GetWorldPosition();
            glm::vec3 p1 = points.front().node->GetWorldPosition();
            world->AddLine(Line(p0, p1, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.01f));
        }
    }
#endif
}

bool Spline3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Spline3D* spline = static_cast<Spline3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Camera")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentCamera = WeakPtr<Node>();
            success = false; // allow datum to store null
        }
        else if (node->As<Camera3D>())
        {
            spline->mAttachmentCamera = newNode;

            // Do not re-parent; just keep reference

            // Snap to spline origin in local space
            Camera3D* cam = node->As<Camera3D>();
            if (cam)
            {
                cam->SetPosition(glm::vec3(0.0f));
            }

            success = false; // allow datum to store value for PIE cloning
        }
        else
        {
            // Ignore non-camera assignments
            success = true;
        }
    }
    else if (prop->mName == "Static Mesh")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentStaticMesh = WeakPtr<Node>();
            success = false; // allow datum to store null
        }
        else if (node->As<StaticMesh3D>())
        {
            spline->mAttachmentStaticMesh = newNode;

            std::vector<SplinePointNode> points;
            GatherSplinePointNodes(spline, points);
            if (!points.empty())
            {
                node->As<Node3D>()->SetWorldPosition(points[0].node->GetWorldPosition());
            }

            success = false; // allow datum to store value for PIE cloning
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Skeletal Mesh")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentSkeletalMesh = WeakPtr<Node>();
            success = false;
        }
        else if (node->As<SkeletalMesh3D>())
        {
            spline->mAttachmentSkeletalMesh = newNode;
            std::vector<SplinePointNode> points;
            GatherSplinePointNodes(spline, points);
            if (!points.empty())
            {
                node->As<Node3D>()->SetWorldPosition(points[0].node->GetWorldPosition());
            }
            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Particle")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentParticle3D = WeakPtr<Node>();
            success = false;
        }
        else if (node->As<Particle3D>())
        {
            spline->mAttachmentParticle3D = newNode;
            std::vector<SplinePointNode> points;
            GatherSplinePointNodes(spline, points);
            if (!points.empty())
            {
                node->As<Node3D>()->SetWorldPosition(points[0].node->GetWorldPosition());
            }
            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Point Light")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentPointLight = WeakPtr<Node>();
            success = false;
        }
        else if (node->As<PointLight3D>())
        {
            spline->mAttachmentPointLight = newNode;
            std::vector<SplinePointNode> points;
            GatherSplinePointNodes(spline, points);
            if (!points.empty())
            {
                node->As<Node3D>()->SetWorldPosition(points[0].node->GetWorldPosition());
            }
            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Audio")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentAudio3D = WeakPtr<Node>();
            success = false;
        }
        else if (node->As<Audio3D>())
        {
            spline->mAttachmentAudio3D = newNode;
            std::vector<SplinePointNode> points;
            GatherSplinePointNodes(spline, points);
            if (!points.empty())
            {
                node->As<Node3D>()->SetWorldPosition(points[0].node->GetWorldPosition());
            }
            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Point Speed Target")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mPointSpeedTarget = WeakPtr<Node>();
            success = false;
        }
        else if (node->GetName().rfind("point", 0) == 0)
        {
            spline->mPointSpeedTarget = newNode;

            // Load existing speed for this point (or default)
            const std::string& name = node->GetName();
            bool found = false;
            for (uint32_t i = 0; i < spline->mPointSpeedEntries.size(); ++i)
            {
                if (spline->mPointSpeedEntries[i].name == name)
                {
                    spline->mPointSpeedValue = spline->mPointSpeedEntries[i].speed;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                spline->mPointSpeedValue = 1.0f;
            }

            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "Point Speed")
    {
        float value = *(float*)newValue;
        spline->mPointSpeedValue = value;

        Node* node = spline->mPointSpeedTarget.Get();
        if (node)
        {
            const std::string& name = node->GetName();
            bool found = false;
            for (uint32_t i = 0; i < spline->mPointSpeedEntries.size(); ++i)
            {
                if (spline->mPointSpeedEntries[i].name == name)
                {
                    spline->mPointSpeedEntries[i].speed = value;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                Spline3D::PointSpeedEntry entry;
                entry.name = name;
                entry.speed = value;
                spline->mPointSpeedEntries.push_back(entry);
            }
        }

        success = false;
    }
    else if (prop->mName == "Play")
    {
        bool value = *(bool*)newValue;
        spline->mPlaying = value;
        success = false;
    }

    return success;
}

#if EDITOR
bool Spline3D::DrawCustomProperty(Property& prop)
{
    if (prop.mName == "Generate Point")
    {
        if (ImGui::Button("Generate Point"))
        {
            GeneratePoint();
        }
        return true;
    }

    return false;
}
#endif

void Spline3D::GeneratePoint()
{
    // Find max point index and last point
    int32_t maxIndex = 0;
    Node3D* lastPoint = nullptr;

    const std::vector<NodePtr>& children = GetChildren();
    for (uint32_t i = 0; i < children.size(); ++i)
    {
        Node* child = children[i].Get();
        if (!child) continue;

        const std::string& name = child->GetName();
        if (name.rfind("point", 0) == 0)
        {
            int idx = 0;
            if (sscanf(name.c_str(), "point%d", &idx) == 1)
            {
                if (idx > maxIndex)
                {
                    maxIndex = idx;
                    lastPoint = child->As<Node3D>();
                }
            }
        }
    }

    int32_t newIndex = maxIndex + 1;
    char newName[64];
    snprintf(newName, sizeof(newName), "point%d", newIndex);

    Box3D* newPoint = CreateChild<Box3D>(newName);
    if (newPoint)
    {
        newPoint->SetExtents(glm::vec3(0.4f));
        if (lastPoint)
        {
            newPoint->SetPosition(lastPoint->GetPosition());
        }
        else
        {
            newPoint->SetPosition(glm::vec3(0.0f));
        }
    }

}

void Spline3D::AddPoint(const glm::vec3& p)
{
    mPoints.push_back(p);
}

void Spline3D::ClearPoints()
{
    mPoints.clear();
}

uint32_t Spline3D::GetPointCount() const
{
    return (uint32_t)mPoints.size();
}

glm::vec3 Spline3D::GetPoint(uint32_t index) const
{
    if (index >= mPoints.size())
        return glm::vec3(0,0,0);
    return mPoints[index];
}

void Spline3D::SetPoint(uint32_t index, const glm::vec3& p)
{
    if (index >= mPoints.size())
        return;
    mPoints[index] = p;
}

glm::vec3 Spline3D::GetPositionAt(float t) const
{
    if (mPoints.size() == 0)
        return glm::vec3(0,0,0);
    if (mPoints.size() == 1)
        return mPoints[0];
    if (mPoints.size() == 2)
        return glm::mix(mPoints[0], mPoints[1], glm::clamp(t, 0.0f, 1.0f));

    // Catmull-Rom across segments
    uint32_t numSeg = (uint32_t)mPoints.size() - 3;
    if (numSeg == 0)
    {
        return CatmullRom(mPoints[0], mPoints[1], mPoints[2], mPoints[3], glm::clamp(t, 0.0f, 1.0f));
    }

    float ft = glm::clamp(t, 0.0f, 1.0f) * (float)numSeg;
    uint32_t seg = (uint32_t)glm::clamp((int)ft, 0, (int)numSeg - 1);
    float lt = ft - (float)seg;

    const glm::vec3& p0 = mPoints[seg + 0];
    const glm::vec3& p1 = mPoints[seg + 1];
    const glm::vec3& p2 = mPoints[seg + 2];
    const glm::vec3& p3 = mPoints[seg + 3];

    return CatmullRom(p0, p1, p2, p3, lt);
}

glm::vec3 Spline3D::GetTangentAt(float t) const
{
    if (mPoints.size() < 4)
        return glm::vec3(0,0,1);

    uint32_t numSeg = (uint32_t)mPoints.size() - 3;
    float ft = glm::clamp(t, 0.0f, 1.0f) * (float)numSeg;
    uint32_t seg = (uint32_t)glm::clamp((int)ft, 0, (int)numSeg - 1);
    float lt = ft - (float)seg;

    const glm::vec3& p0 = mPoints[seg + 0];
    const glm::vec3& p1 = mPoints[seg + 1];
    const glm::vec3& p2 = mPoints[seg + 2];
    const glm::vec3& p3 = mPoints[seg + 3];

    glm::vec3 tan = CatmullRomTangent(p0, p1, p2, p3, lt);
    if (glm::length(tan) > 0.0f)
        tan = glm::normalize(tan);
    return tan;
}
