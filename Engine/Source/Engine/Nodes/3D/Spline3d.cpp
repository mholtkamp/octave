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
#include <cfloat>

#if EDITOR
#include "imgui.h"
#include "EditorState.h"
#endif

DEFINE_NODE(Spline3D, Node3D);

static bool sGeneratePoint = false;
static bool sGenerateLink11 = false;
static bool sSplineLinesVisible = true;

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

    for (uint32_t i = 12; i <= 64; ++i)
    {
        mFollowLinkExtra[i] = true;
        mLinkTriggeredExtra[i] = false;
        mLinkSpeedModifierExtra[i] = 1.0f;
    }

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

    if (mEnableLink11 && mGeneratedLinkCount < 11)
    {
        mGeneratedLinkCount = 11;
    }
    mGeneratedLinkCount = glm::clamp(mGeneratedLinkCount, 10, 64);
    mEnableLink11 = (mGeneratedLinkCount >= 11);

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
    resolveAttachment(mAttachmentNode3D);
    resolveAttachment(mLinkFrom);
    resolveAttachment(mLinkTo);

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

        Node* genericNode = mAttachmentNode3D.Get();
        if (genericNode && genericNode->As<Node3D>())
        {
            mOrigNodeTransform = genericNode->As<Node3D>()->GetTransform();
            genericNode->As<Node3D>()->SetWorldPosition(startPos);
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

void Spline3D::SetPaused(bool paused)
{
    mPause = paused;
}

void Spline3D::SetSplineLinesVisible(bool visible)
{
    sSplineLinesVisible = visible;
}

bool Spline3D::IsSplineLinesVisible()
{
    return sSplineLinesVisible;
}

bool Spline3D::IsPaused() const
{
    return mPause;
}

void Spline3D::SetFollowLinkEnabled(uint32_t index, bool enabled)
{
    switch (index)
    {
    case 1: mFollowLink1 = enabled; mLinkTriggered1 = false; break;
    case 2: mFollowLink2 = enabled; mLinkTriggered2 = false; break;
    case 3: mFollowLink3 = enabled; mLinkTriggered3 = false; break;
    case 4: mFollowLink4 = enabled; mLinkTriggered4 = false; break;
    case 5: mFollowLink5 = enabled; mLinkTriggered5 = false; break;
    case 6: mFollowLink6 = enabled; mLinkTriggered6 = false; break;
    case 7: mFollowLink7 = enabled; mLinkTriggered7 = false; break;
    case 8: mFollowLink8 = enabled; mLinkTriggered8 = false; break;
    case 9: mFollowLink9 = enabled; mLinkTriggered9 = false; break;
    case 10: mFollowLink10 = enabled; mLinkTriggered10 = false; break;
    case 11: mFollowLink11 = enabled; mLinkTriggered11 = false; break;
    default:
        if (index >= 12 && index <= 64)
        {
            mFollowLinkExtra[index] = enabled;
            mLinkTriggeredExtra[index] = false;
        }
        break;
    }
}

bool Spline3D::IsFollowLinkEnabled(uint32_t index) const
{
    switch (index)
    {
    case 1: return mFollowLink1;
    case 2: return mFollowLink2;
    case 3: return mFollowLink3;
    case 4: return mFollowLink4;
    case 5: return mFollowLink5;
    case 6: return mFollowLink6;
    case 7: return mFollowLink7;
    case 8: return mFollowLink8;
    case 9: return mFollowLink9;
    case 10: return mFollowLink10;
    case 11: return mFollowLink11;
    default:
        if (index >= 12 && index <= 64)
        {
            return mFollowLinkExtra[index];
        }
        return false;
    }
}

bool Spline3D::IsNearLinkFrom(uint32_t index, float epsilon) const
{
    Node* fromNode = nullptr;
    switch (index)
    {
    case 1: fromNode = mLinkFrom.Get(); break;
    case 2: fromNode = mLinkFrom2.Get(); break;
    case 3: fromNode = mLinkFrom3.Get(); break;
    case 4: fromNode = mLinkFrom4.Get(); break;
    case 5: fromNode = mLinkFrom5.Get(); break;
    case 6: fromNode = mLinkFrom6.Get(); break;
    case 7: fromNode = mLinkFrom7.Get(); break;
    case 8: fromNode = mLinkFrom8.Get(); break;
    case 9: fromNode = mLinkFrom9.Get(); break;
    case 10: fromNode = mLinkFrom10.Get(); break;
    case 11: fromNode = mLinkFrom11.Get(); break;
    default:
        if (index >= 12 && index <= 64) fromNode = mLinkFromExtra[index].Get();
        else return false;
        break;
    }

    Node3D* from3d = fromNode ? fromNode->As<Node3D>() : nullptr;
    if (!from3d)
        return false;

    Node* trackedNode = mAttachmentCamera.Get();
    if (!trackedNode) trackedNode = mAttachmentStaticMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentSkeletalMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentParticle3D.Get();
    if (!trackedNode) trackedNode = mAttachmentPointLight.Get();
    if (!trackedNode) trackedNode = mAttachmentAudio3D.Get();
    if (!trackedNode) trackedNode = mAttachmentNode3D.Get();

    Node3D* tracked3d = trackedNode ? trackedNode->As<Node3D>() : nullptr;
    if (!tracked3d)
        return false;

    float useEpsilon = glm::max(0.0001f, epsilon);
    float dist = glm::length(tracked3d->GetWorldPosition() - from3d->GetWorldPosition());
    return dist <= useEpsilon;
}

bool Spline3D::IsNearLinkTo(uint32_t index, float epsilon) const
{
    Node* toNode = nullptr;
    switch (index)
    {
    case 1: toNode = mLinkTo.Get(); break;
    case 2: toNode = mLinkTo2.Get(); break;
    case 3: toNode = mLinkTo3.Get(); break;
    case 4: toNode = mLinkTo4.Get(); break;
    case 5: toNode = mLinkTo5.Get(); break;
    case 6: toNode = mLinkTo6.Get(); break;
    case 7: toNode = mLinkTo7.Get(); break;
    case 8: toNode = mLinkTo8.Get(); break;
    case 9: toNode = mLinkTo9.Get(); break;
    case 10: toNode = mLinkTo10.Get(); break;
    case 11: toNode = mLinkTo11.Get(); break;
    default:
        if (index >= 12 && index <= 64) toNode = mLinkToExtra[index].Get();
        else return false;
        break;
    }

    Node3D* to3d = toNode ? toNode->As<Node3D>() : nullptr;
    if (!to3d)
        return false;

    Node* trackedNode = mAttachmentCamera.Get();
    if (!trackedNode) trackedNode = mAttachmentStaticMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentSkeletalMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentParticle3D.Get();
    if (!trackedNode) trackedNode = mAttachmentPointLight.Get();
    if (!trackedNode) trackedNode = mAttachmentAudio3D.Get();
    if (!trackedNode) trackedNode = mAttachmentNode3D.Get();

    Node3D* tracked3d = trackedNode ? trackedNode->As<Node3D>() : nullptr;
    if (!tracked3d)
        return false;

    float useEpsilon = glm::max(0.0001f, epsilon);
    float dist = glm::length(tracked3d->GetWorldPosition() - to3d->GetWorldPosition());
    return dist <= useEpsilon;
}

bool Spline3D::IsLinkDirectionForward(uint32_t index, float threshold) const
{
    Node* fromNode = nullptr;
    Node* toNode = nullptr;
    switch (index)
    {
    case 1: fromNode = mLinkFrom.Get(); toNode = mLinkTo.Get(); break;
    case 2: fromNode = mLinkFrom2.Get(); toNode = mLinkTo2.Get(); break;
    case 3: fromNode = mLinkFrom3.Get(); toNode = mLinkTo3.Get(); break;
    case 4: fromNode = mLinkFrom4.Get(); toNode = mLinkTo4.Get(); break;
    case 5: fromNode = mLinkFrom5.Get(); toNode = mLinkTo5.Get(); break;
    case 6: fromNode = mLinkFrom6.Get(); toNode = mLinkTo6.Get(); break;
    case 7: fromNode = mLinkFrom7.Get(); toNode = mLinkTo7.Get(); break;
    case 8: fromNode = mLinkFrom8.Get(); toNode = mLinkTo8.Get(); break;
    case 9: fromNode = mLinkFrom9.Get(); toNode = mLinkTo9.Get(); break;
    case 10: fromNode = mLinkFrom10.Get(); toNode = mLinkTo10.Get(); break;
    case 11: fromNode = mLinkFrom11.Get(); toNode = mLinkTo11.Get(); break;
    default:
        if (index >= 12 && index <= 64)
        {
            fromNode = mLinkFromExtra[index].Get();
            toNode = mLinkToExtra[index].Get();
        }
        else return false;
        break;
    }

    Node3D* from3d = fromNode ? fromNode->As<Node3D>() : nullptr;
    Node3D* to3d = toNode ? toNode->As<Node3D>() : nullptr;
    if (!from3d || !to3d)
        return false;

    glm::vec3 linkVec = to3d->GetWorldPosition() - from3d->GetWorldPosition();
    float linkLen = glm::length(linkVec);
    if (linkLen <= 0.0001f)
        return false;
    glm::vec3 linkDir = linkVec / linkLen;

    std::vector<SplinePointNode> points;
    GatherSplinePointNodes(const_cast<Spline3D*>(this), points);
    if (points.size() < 2)
        return false;

    int fromIdx = -1;
    for (uint32_t i = 0; i < points.size(); ++i)
    {
        if (points[i].node == from3d)
        {
            fromIdx = (int)i;
            break;
        }
    }

    glm::vec3 splineDir = glm::vec3(0.0f);

    if (fromIdx >= 0)
    {
        auto getPos = [&](int idx)
        {
            if (mCloseLoop)
            {
                int count = (int)points.size();
                int wrapped = (idx % count + count) % count;
                return points[(uint32_t)wrapped].node->GetWorldPosition();
            }

            idx = glm::clamp(idx, 0, (int)points.size() - 1);
            return points[(uint32_t)idx].node->GetWorldPosition();
        };

        glm::vec3 pPrev = getPos(fromIdx - 1);
        glm::vec3 pHere = getPos(fromIdx);
        glm::vec3 pNext = getPos(fromIdx + 1);

        glm::vec3 forward = pNext - pHere;
        glm::vec3 backward = pHere - pPrev;

        if (glm::length(forward) > 0.0001f)
        {
            splineDir = Maths::SafeNormalize(forward);
        }
        else if (glm::length(backward) > 0.0001f)
        {
            splineDir = Maths::SafeNormalize(backward);
        }
    }

    if (glm::length(splineDir) <= 0.0001f)
    {
        // Fallback: closest segment direction near the From node.
        glm::vec3 pos = from3d->GetWorldPosition();
        float bestDistSq = FLT_MAX;
        glm::vec3 bestSegDir = glm::vec3(1.0f, 0.0f, 0.0f);

        auto testSeg = [&](const glm::vec3& a, const glm::vec3& b)
        {
            glm::vec3 ab = b - a;
            float abLenSq = glm::dot(ab, ab);
            if (abLenSq <= 0.000001f)
                return;

            float t = glm::dot(pos - a, ab) / abLenSq;
            t = glm::clamp(t, 0.0f, 1.0f);
            glm::vec3 closest = a + ab * t;
            float distSq = glm::dot(pos - closest, pos - closest);
            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                bestSegDir = Maths::SafeNormalize(ab);
            }
        };

        for (uint32_t i = 1; i < points.size(); ++i)
        {
            testSeg(points[i - 1].node->GetWorldPosition(), points[i].node->GetWorldPosition());
        }

        if (mCloseLoop && points.size() > 1)
        {
            testSeg(points.back().node->GetWorldPosition(), points.front().node->GetWorldPosition());
        }

        splineDir = bestSegDir;
    }

    float d = glm::dot(splineDir, linkDir);
    return d >= threshold;
}

void Spline3D::CancelActiveLink()
{
    mLinkActive = false;
    mLinkTravel = 0.0f;
    mLinkLen = 0.0f;
    mLinkTargetSpline = nullptr;
}

bool Spline3D::TriggerLink(uint32_t index)
{
    Node* fromNode = nullptr;
    Node* toNode = nullptr;
    switch (index)
    {
    case 1: fromNode = mLinkFrom.Get(); toNode = mLinkTo.Get(); break;
    case 2: fromNode = mLinkFrom2.Get(); toNode = mLinkTo2.Get(); break;
    case 3: fromNode = mLinkFrom3.Get(); toNode = mLinkTo3.Get(); break;
    case 4: fromNode = mLinkFrom4.Get(); toNode = mLinkTo4.Get(); break;
    case 5: fromNode = mLinkFrom5.Get(); toNode = mLinkTo5.Get(); break;
    case 6: fromNode = mLinkFrom6.Get(); toNode = mLinkTo6.Get(); break;
    case 7: fromNode = mLinkFrom7.Get(); toNode = mLinkTo7.Get(); break;
    case 8: fromNode = mLinkFrom8.Get(); toNode = mLinkTo8.Get(); break;
    case 9: fromNode = mLinkFrom9.Get(); toNode = mLinkTo9.Get(); break;
    case 10: fromNode = mLinkFrom10.Get(); toNode = mLinkTo10.Get(); break;
    case 11: fromNode = mLinkFrom11.Get(); toNode = mLinkTo11.Get(); break;
    default:
        if (index >= 12 && index <= 64)
        {
            fromNode = mLinkFromExtra[index].Get();
            toNode = mLinkToExtra[index].Get();
        }
        else return false;
        break;
    }

    float selectedLinkSpeedModifier = 1.0f;
    switch (index)
    {
    case 1: selectedLinkSpeedModifier = mLinkSpeedModifier1; break;
    case 2: selectedLinkSpeedModifier = mLinkSpeedModifier2; break;
    case 3: selectedLinkSpeedModifier = mLinkSpeedModifier3; break;
    case 4: selectedLinkSpeedModifier = mLinkSpeedModifier4; break;
    case 5: selectedLinkSpeedModifier = mLinkSpeedModifier5; break;
    case 6: selectedLinkSpeedModifier = mLinkSpeedModifier6; break;
    case 7: selectedLinkSpeedModifier = mLinkSpeedModifier7; break;
    case 8: selectedLinkSpeedModifier = mLinkSpeedModifier8; break;
    case 9: selectedLinkSpeedModifier = mLinkSpeedModifier9; break;
    case 10: selectedLinkSpeedModifier = mLinkSpeedModifier10; break;
    case 11: selectedLinkSpeedModifier = mLinkSpeedModifier11; break;
    default:
        if (index >= 12 && index <= 64)
            selectedLinkSpeedModifier = mLinkSpeedModifierExtra[index];
        break;
    }

    Node3D* from3d = fromNode ? fromNode->As<Node3D>() : nullptr;
    Node3D* to3d = toNode ? toNode->As<Node3D>() : nullptr;
    if (!from3d || !to3d)
        return false;

    Node* targetParent = to3d->GetParent();
    Spline3D* targetSpline = targetParent ? targetParent->As<Spline3D>() : nullptr;
    if (!targetSpline)
        return false;

    std::vector<SplinePointNode> tpoints;
    GatherSplinePointNodes(targetSpline, tpoints);

    int targetIndex = -1;
    for (uint32_t i = 0; i < tpoints.size(); ++i)
    {
        if (tpoints[i].node == to3d)
        {
            targetIndex = (int)i;
            break;
        }
    }

    if (targetIndex < 0)
        return false;

    auto getPointSpeed = [&](const std::string& pointName)
    {
        for (uint32_t i = 0; i < targetSpline->mPointSpeedEntries.size(); ++i)
        {
            if (targetSpline->mPointSpeedEntries[i].name == pointName)
                return glm::max(0.001f, targetSpline->mPointSpeedEntries[i].speed);
        }
        return 1.0f;
    };

    float startDist = 0.0f;
    float prevDist = 0.0f;
    float totalLen = 0.0f;
    for (int i = 1; i <= (int)tpoints.size() - 1; ++i)
    {
        glm::vec3 p0 = tpoints[i - 1].node->GetWorldPosition();
        glm::vec3 p1 = tpoints[i].node->GetWorldPosition();
        float len = glm::length(p1 - p0);
        float mult = getPointSpeed(tpoints[i - 1].node->GetName());
        float effLen = len / mult;
        if (i <= targetIndex)
            startDist += effLen;
        if (i <= targetIndex - 1)
            prevDist += effLen;
        totalLen += effLen;
    }

    float closeSegLen = 0.0f;
    if (targetSpline->mCloseLoop && tpoints.size() > 1)
    {
        glm::vec3 p0 = tpoints.back().node->GetWorldPosition();
        glm::vec3 p1 = tpoints.front().node->GetWorldPosition();
        float len = glm::length(p1 - p0);
        float mult = getPointSpeed(tpoints.back().node->GetName());
        float effLen = len / mult;
        closeSegLen = effLen;
        totalLen += effLen;
    }

    if (targetIndex == 0 && targetSpline->mCloseLoop && totalLen > 0.0001f)
    {
        prevDist = glm::max(0.0f, totalLen - closeSegLen);
    }

    if (!targetSpline->mLoop)
    {
        startDist = glm::clamp(startDist, 0.0f, totalLen);
        prevDist = glm::clamp(prevDist, 0.0f, totalLen);
    }
    else if (totalLen > 0.0001f)
    {
        while (startDist >= totalLen) startDist -= totalLen;
        while (startDist < 0.0f) startDist += totalLen;
        while (prevDist >= totalLen) prevDist -= totalLen;
        while (prevDist < 0.0f) prevDist += totalLen;
    }

    targetSpline->mPlaying = false;

    mLinkActive = true;
    mLinkTravel = 0.0f;
    mLinkStart = from3d->GetWorldPosition();
    mLinkEnd = to3d->GetWorldPosition();
    mLinkLen = glm::length(mLinkEnd - mLinkStart);
    mLinkTargetSpline = ResolveWeakPtr(targetSpline);
    mLinkTargetStartDist = startDist;
    mLinkTargetPrevDist = prevDist;
    mLinkTargetTotalLen = totalLen;
    mActiveLinkSpeedModifier = glm::max(0.001f, selectedLinkSpeedModifier);

    return true;
}

void Spline3D::Tick(float deltaTime)
{
    Node3D::Tick(deltaTime);

    Node* trackedNode = mAttachmentCamera.Get();
    if (!trackedNode) trackedNode = mAttachmentStaticMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentSkeletalMesh.Get();
    if (!trackedNode) trackedNode = mAttachmentParticle3D.Get();
    if (!trackedNode) trackedNode = mAttachmentPointLight.Get();
    if (!trackedNode) trackedNode = mAttachmentAudio3D.Get();
    if (!trackedNode) trackedNode = mAttachmentNode3D.Get();

    Node3D* tracked3d = trackedNode ? trackedNode->As<Node3D>() : nullptr;
    if (tracked3d)
    {
        glm::vec3 pos = tracked3d->GetWorldPosition();
        if (mHasTrackedPos)
        {
            glm::vec3 delta = pos - mPrevTrackedPos;
            float lenSq = glm::dot(delta, delta);
            if (lenSq > 0.000001f)
            {
                mTrackedMoveDir = Maths::SafeNormalize(delta);
            }
        }

        mPrevTrackedPos = pos;
        mHasTrackedPos = true;
    }

    if (!IsPlaying() || !mPlaying || mPause)
        return;

    if (mLinkActive)
    {
        Node* camNode = mAttachmentCamera.Get();
        Node* meshNode = mAttachmentStaticMesh.Get();
        Node* skelNode = mAttachmentSkeletalMesh.Get();
        Node* partNode = mAttachmentParticle3D.Get();
        Node* lightNode = mAttachmentPointLight.Get();
        Node* audioNode = mAttachmentAudio3D.Get();
        Node* genericNode = mAttachmentNode3D.Get();

        bool hasCam = (camNode && camNode->As<Camera3D>());
        bool hasMesh = (meshNode && meshNode->As<StaticMesh3D>());
        bool hasSkel = (skelNode && skelNode->As<SkeletalMesh3D>());
        bool hasPart = (partNode && partNode->As<Particle3D>());
        bool hasLight = (lightNode && lightNode->As<PointLight3D>());
        bool hasAudio = (audioNode && audioNode->As<Audio3D>());
        bool hasNode = (genericNode && genericNode->As<Node3D>());

        if (!hasCam && !hasMesh && !hasSkel && !hasPart && !hasLight && !hasAudio && !hasNode)
            return;

        mLinkTravel += (mLinkSpeedModifier * mActiveLinkSpeedModifier) * deltaTime;
        float t = (mLinkLen > 0.0001f) ? (mLinkTravel / mLinkLen) : 1.0f;
        if (t > 1.0f) t = 1.0f;

        float tt = t;
        if (mLinkSmoothStep)
        {
            tt = tt * tt * (3.0f - 2.0f * tt);
        }
        glm::vec3 pos = glm::mix(mLinkStart, mLinkEnd, tt);
        glm::vec3 tangent = Maths::SafeNormalize(mLinkEnd - mLinkStart);

        auto applyMove = [&](Node* node, bool face)
        {
            node->As<Node3D>()->SetWorldPosition(pos);
            if (face && (mFaceTangent || mReverseFaceTangent))
            {
                glm::vec3 dir = mReverseFaceTangent ? -tangent : tangent;
                if (mLinkSmoothRotate)
                {
                    glm::quat targetRot = Maths::VectorToQuat(dir);
                    glm::quat curRot = node->As<Node3D>()->GetWorldRotationQuat();
                    float s = glm::clamp(deltaTime * 6.0f, 0.0f, 1.0f);
                    node->As<Node3D>()->SetWorldRotation(glm::slerp(curRot, targetRot, s));
                }
                else
                {
                    node->As<Node3D>()->LookAt(pos + dir, glm::vec3(0,1,0));
                }
            }
        };

        if (hasCam) applyMove(camNode, true);
        if (hasMesh) applyMove(meshNode, true);
        if (hasSkel) applyMove(skelNode, true);
        if (hasPart) applyMove(partNode, false);
        if (hasLight) applyMove(lightNode, false);
        if (hasAudio) applyMove(audioNode, false);
        if (hasNode) applyMove(genericNode, true);

        if (t >= 1.0f)
        {
            Spline3D* targetSpline = mLinkTargetSpline.Get() ? mLinkTargetSpline.Get()->As<Spline3D>() : nullptr;
            if (targetSpline)
            {
                // Preserve target spline path settings
                const bool targetLoop = targetSpline->mLoop;
                const bool targetClose = targetSpline->mCloseLoop;
                const bool targetSmooth = targetSpline->mSmoothCurve;

                // Transfer attachments
                targetSpline->mAttachmentCamera = mAttachmentCamera;
                targetSpline->mAttachmentStaticMesh = mAttachmentStaticMesh;
                targetSpline->mAttachmentSkeletalMesh = mAttachmentSkeletalMesh;
                targetSpline->mAttachmentParticle3D = mAttachmentParticle3D;
                targetSpline->mAttachmentPointLight = mAttachmentPointLight;
                targetSpline->mAttachmentAudio3D = mAttachmentAudio3D;
                targetSpline->mAttachmentNode3D = mAttachmentNode3D;

                // Transfer facing options only
                targetSpline->mFaceTangent = mFaceTangent;
                targetSpline->mReverseFaceTangent = mReverseFaceTangent;

                // Restore path settings (no inheritance)
                targetSpline->mLoop = targetLoop;
                targetSpline->mCloseLoop = targetClose;
                targetSpline->mSmoothCurve = targetSmooth;

                // Clear attachments on source
                mAttachmentCamera = WeakPtr<Node>();
                mAttachmentStaticMesh = WeakPtr<Node>();
                mAttachmentSkeletalMesh = WeakPtr<Node>();
                mAttachmentParticle3D = WeakPtr<Node>();
                mAttachmentPointLight = WeakPtr<Node>();
                mAttachmentAudio3D = WeakPtr<Node>();
                mAttachmentNode3D = WeakPtr<Node>();

                targetSpline->mTravel = mLinkTargetStartDist;
                if (targetSpline->mDisableBounce)
                {
                    // Prevent immediate re-trigger at the arrival point (all slots)
                    targetSpline->mLinkTriggered1 = true;
                    targetSpline->mLinkTriggered2 = true;
                    targetSpline->mLinkTriggered3 = true;
                    targetSpline->mLinkTriggered4 = true;
                    targetSpline->mLinkTriggered5 = true;
                    targetSpline->mLinkTriggered6 = true;
                    targetSpline->mLinkTriggered7 = true;
                    targetSpline->mLinkTriggered8 = true;
                    targetSpline->mLinkTriggered9 = true;
                    targetSpline->mLinkTriggered10 = true;
                    targetSpline->mLinkTriggered11 = true;
                    for (uint32_t li = 12; li <= 64; ++li)
                    {
                        targetSpline->mLinkTriggeredExtra[li] = true;
                    }
                }
                targetSpline->mPlaying = true;
                targetSpline->Play();
                mPlaying = false;
            }

            mLinkActive = false;
            mLinkTriggered1 = false;
            mLinkTriggered2 = false;
            mLinkTriggered3 = false;
            mLinkTriggered4 = false;
            mLinkTriggered5 = false;
            mLinkTriggered6 = false;
            mLinkTriggered7 = false;
            mLinkTriggered8 = false;
            mLinkTriggered9 = false;
            mLinkTriggered10 = false;
            mLinkTriggered11 = false;
            for (uint32_t li = 12; li <= 64; ++li)
            {
                mLinkTriggeredExtra[li] = false;
            }
        }

        return;
    }

    Node* camNode = mAttachmentCamera.Get();
    Node* meshNode = mAttachmentStaticMesh.Get();
    Node* skelNode = mAttachmentSkeletalMesh.Get();
    Node* partNode = mAttachmentParticle3D.Get();
    Node* lightNode = mAttachmentPointLight.Get();
    Node* audioNode = mAttachmentAudio3D.Get();
    Node* genericNode = mAttachmentNode3D.Get();

    bool hasCam = (camNode && camNode->As<Camera3D>());
    bool hasMesh = (meshNode && meshNode->As<StaticMesh3D>());
    bool hasSkel = (skelNode && skelNode->As<SkeletalMesh3D>());
    bool hasPart = (partNode && partNode->As<Particle3D>());
    bool hasLight = (lightNode && lightNode->As<PointLight3D>());
    bool hasAudio = (audioNode && audioNode->As<Audio3D>());
    bool hasNode = (genericNode && genericNode->As<Node3D>());

    if (!hasCam && !hasMesh && !hasSkel && !hasPart && !hasLight && !hasAudio && !hasNode)
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
        if (face && (mFaceTangent || mReverseFaceTangent))
        {
            glm::vec3 dir = mReverseFaceTangent ? -tangent : tangent;
            if (!mSmoothCurve && mSmoothRotate)
            {
                glm::quat targetRot = Maths::VectorToQuat(dir);
                glm::quat curRot = node->As<Node3D>()->GetWorldRotationQuat();
                float s = glm::clamp(deltaTime * 6.0f, 0.0f, 1.0f);
                node->As<Node3D>()->SetWorldRotation(glm::slerp(curRot, targetRot, s));
            }
            else
            {
                node->As<Node3D>()->LookAt(pos + dir, glm::vec3(0,1,0));
            }
        }
    };

    if (hasCam) applyMove(camNode, true);
    if (hasMesh) applyMove(meshNode, true);
    if (hasSkel) applyMove(skelNode, true);
    if (hasPart) applyMove(partNode, false);
    if (hasLight) applyMove(lightNode, false);
    if (hasAudio) applyMove(audioNode, false);
    if (hasNode) applyMove(genericNode, true);

    if (mFollowLink1 || mFollowLink2 || mFollowLink3 || mFollowLink4 || mFollowLink5 || mFollowLink6 || mFollowLink7 || mFollowLink8 || mFollowLink9 || mFollowLink10 || (mGeneratedLinkCount >= 11 && mFollowLink11))
    {
        auto tryLink = [&](NodePtrWeak& fromPtr, NodePtrWeak& toPtr, bool& linkTriggered, float linkSpeedModifier)
        {
            Node* fromNode = fromPtr.Get();
            Node* toNode = toPtr.Get();
            if (!fromNode || !toNode) return false;

            Node3D* from3d = fromNode->As<Node3D>();
            Node3D* to3d = toNode->As<Node3D>();
            if (!from3d || !to3d) return false;

            float distToFrom = glm::length(pos - from3d->GetWorldPosition());
            const float kLinkEpsilon = 0.05f;

            if (!linkTriggered && distToFrom <= kLinkEpsilon)
            {
                Node* targetParent = to3d->GetParent();
                Spline3D* targetSpline = targetParent ? targetParent->As<Spline3D>() : nullptr;
                if (!targetSpline) return false;

                std::vector<SplinePointNode> tpoints;
                GatherSplinePointNodes(targetSpline, tpoints);

                int targetIndex = -1;
                for (uint32_t i = 0; i < tpoints.size(); ++i)
                {
                    if (tpoints[i].node == to3d)
                    {
                        targetIndex = (int)i;
                        break;
                    }
                }

                if (targetIndex < 0) return false;

                auto getPointSpeed = [&](const std::string& pointName)
                {
                    for (uint32_t i = 0; i < targetSpline->mPointSpeedEntries.size(); ++i)
                    {
                        if (targetSpline->mPointSpeedEntries[i].name == pointName)
                            return glm::max(0.001f, targetSpline->mPointSpeedEntries[i].speed);
                    }
                    return 1.0f;
                };

                float startDist = 0.0f;
                float prevDist = 0.0f;
                float totalLen = 0.0f;
                for (int i = 1; i <= (int)tpoints.size() - 1; ++i)
                {
                    glm::vec3 p0 = tpoints[i - 1].node->GetWorldPosition();
                    glm::vec3 p1 = tpoints[i].node->GetWorldPosition();
                    float len = glm::length(p1 - p0);
                    float mult = getPointSpeed(tpoints[i - 1].node->GetName());
                    float effLen = len / mult;
                    if (i <= targetIndex)
                        startDist += effLen;
                    if (i <= targetIndex - 1)
                        prevDist += effLen;
                    totalLen += effLen;
                }

                float closeSegLen = 0.0f;
                if (targetSpline->mCloseLoop && tpoints.size() > 1)
                {
                    glm::vec3 p0 = tpoints.back().node->GetWorldPosition();
                    glm::vec3 p1 = tpoints.front().node->GetWorldPosition();
                    float len = glm::length(p1 - p0);
                    float mult = getPointSpeed(tpoints.back().node->GetName());
                    float effLen = len / mult;
                    closeSegLen = effLen;
                    totalLen += effLen;
                }

                if (targetIndex == 0 && targetSpline->mCloseLoop && totalLen > 0.0001f)
                {
                    prevDist = glm::max(0.0f, totalLen - closeSegLen);
                }

                if (!targetSpline->mLoop)
                {
                    startDist = glm::clamp(startDist, 0.0f, totalLen);
                    prevDist = glm::clamp(prevDist, 0.0f, totalLen);
                }
                else if (totalLen > 0.0001f)
                {
                    while (startDist >= totalLen) startDist -= totalLen;
                    while (startDist < 0.0f) startDist += totalLen;
                    while (prevDist >= totalLen) prevDist -= totalLen;
                    while (prevDist < 0.0f) prevDist += totalLen;
                }

                targetSpline->mPlaying = false;

                mLinkActive = true;
                mLinkTravel = 0.0f;
                mLinkStart = from3d->GetWorldPosition();
                mLinkEnd = to3d->GetWorldPosition();
                mLinkLen = glm::length(mLinkEnd - mLinkStart);
                mLinkTargetSpline = ResolveWeakPtr(targetSpline);
                mLinkTargetStartDist = startDist;
                mLinkTargetPrevDist = prevDist;
                mLinkTargetTotalLen = totalLen;
                mActiveLinkSpeedModifier = glm::max(0.001f, linkSpeedModifier);

                linkTriggered = true;
                return true;
            }
            else if (linkTriggered && distToFrom > kLinkEpsilon)
            {
                linkTriggered = false;
            }

            return false;
        };

        if (mFollowLink1 && tryLink(mLinkFrom, mLinkTo, mLinkTriggered1, mLinkSpeedModifier1)) return;
        if (mFollowLink2 && tryLink(mLinkFrom2, mLinkTo2, mLinkTriggered2, mLinkSpeedModifier2)) return;
        if (mFollowLink3 && tryLink(mLinkFrom3, mLinkTo3, mLinkTriggered3, mLinkSpeedModifier3)) return;
        if (mFollowLink4 && tryLink(mLinkFrom4, mLinkTo4, mLinkTriggered4, mLinkSpeedModifier4)) return;
        if (mFollowLink5 && tryLink(mLinkFrom5, mLinkTo5, mLinkTriggered5, mLinkSpeedModifier5)) return;
        if (mFollowLink6 && tryLink(mLinkFrom6, mLinkTo6, mLinkTriggered6, mLinkSpeedModifier6)) return;
        if (mFollowLink7 && tryLink(mLinkFrom7, mLinkTo7, mLinkTriggered7, mLinkSpeedModifier7)) return;
        if (mFollowLink8 && tryLink(mLinkFrom8, mLinkTo8, mLinkTriggered8, mLinkSpeedModifier8)) return;
        if (mFollowLink9 && tryLink(mLinkFrom9, mLinkTo9, mLinkTriggered9, mLinkSpeedModifier9)) return;
        if (mFollowLink10 && tryLink(mLinkFrom10, mLinkTo10, mLinkTriggered10, mLinkSpeedModifier10)) return;
        if (mGeneratedLinkCount >= 11 && mFollowLink11 && tryLink(mLinkFrom11, mLinkTo11, mLinkTriggered11, mLinkSpeedModifier11)) return;

        for (int32_t i = 12; i <= mGeneratedLinkCount && i <= 64; ++i)
        {
            if (mFollowLinkExtra[i] && tryLink(mLinkFromExtra[i], mLinkToExtra[i], mLinkTriggeredExtra[i], mLinkSpeedModifierExtra[i]))
                return;
        }
    }
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

    mGeneratedLinkCount = glm::clamp(mGeneratedLinkCount, 10, 64);
    mEnableLink11 = (mGeneratedLinkCount >= 11);

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
        props.push_back(Property(DatumType::Bool, "Smooth Rotate", this, &mSmoothRotate));
        props.push_back(Property(DatumType::Bool, "Face Tangent", this, &mFaceTangent));
        props.push_back(Property(DatumType::Bool, "Reverse Face Tangent", this, &mReverseFaceTangent));
    }

    {
        SCOPED_CATEGORY("Attachments");
        props.push_back(Property(DatumType::Node, "Camera", this, &mAttachmentCamera, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Static Mesh", this, &mAttachmentStaticMesh, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Skeletal Mesh", this, &mAttachmentSkeletalMesh, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Particle", this, &mAttachmentParticle3D, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Point Light", this, &mAttachmentPointLight, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Audio", this, &mAttachmentAudio3D, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "Node3D", this, &mAttachmentNode3D, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Spline Linking");
        props.push_back(Property(DatumType::Node, "From 1", this, &mLinkFrom, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 1", this, &mLinkTo, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 2", this, &mLinkFrom2, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 2", this, &mLinkTo2, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 3", this, &mLinkFrom3, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 3", this, &mLinkTo3, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 4", this, &mLinkFrom4, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 4", this, &mLinkTo4, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 5", this, &mLinkFrom5, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 5", this, &mLinkTo5, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 6", this, &mLinkFrom6, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 6", this, &mLinkTo6, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 7", this, &mLinkFrom7, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 7", this, &mLinkTo7, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 8", this, &mLinkFrom8, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 8", this, &mLinkTo8, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 9", this, &mLinkFrom9, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 9", this, &mLinkTo9, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "From 10", this, &mLinkFrom10, 1, HandlePropChange));
        props.push_back(Property(DatumType::Node, "To 10", this, &mLinkTo10, 1, HandlePropChange));

        props.push_back(Property(DatumType::Bool, "Generate Link", this, &sGenerateLink11));
        props.push_back(Property(DatumType::Integer, "Generated Link Slots", this, &mGeneratedLinkCount));

        if (mGeneratedLinkCount >= 11)
        {
            props.push_back(Property(DatumType::Node, "From 11", this, &mLinkFrom11, 1, HandlePropChange));
            props.push_back(Property(DatumType::Node, "To 11", this, &mLinkTo11, 1, HandlePropChange));
        }

        for (int32_t i = 12; i <= mGeneratedLinkCount && i <= 64; ++i)
        {
            std::string fromName = std::string("From ") + std::to_string(i);
            std::string toName = std::string("To ") + std::to_string(i);
            props.push_back(Property(DatumType::Node, fromName.c_str(), this, &mLinkFromExtra[i], 1, HandlePropChange));
            props.push_back(Property(DatumType::Node, toName.c_str(), this, &mLinkToExtra[i], 1, HandlePropChange));
        }

        props.push_back(Property(DatumType::Bool, "Follow Link 1", this, &mFollowLink1));
        props.push_back(Property(DatumType::Bool, "Follow Link 2", this, &mFollowLink2));
        props.push_back(Property(DatumType::Bool, "Follow Link 3", this, &mFollowLink3));
        props.push_back(Property(DatumType::Bool, "Follow Link 4", this, &mFollowLink4));
        props.push_back(Property(DatumType::Bool, "Follow Link 5", this, &mFollowLink5));
        props.push_back(Property(DatumType::Bool, "Follow Link 6", this, &mFollowLink6));
        props.push_back(Property(DatumType::Bool, "Follow Link 7", this, &mFollowLink7));
        props.push_back(Property(DatumType::Bool, "Follow Link 8", this, &mFollowLink8));
        props.push_back(Property(DatumType::Bool, "Follow Link 9", this, &mFollowLink9));
        props.push_back(Property(DatumType::Bool, "Follow Link 10", this, &mFollowLink10));
        if (mGeneratedLinkCount >= 11)
        {
            props.push_back(Property(DatumType::Bool, "Follow Link 11", this, &mFollowLink11));
        }
        for (int32_t i = 12; i <= mGeneratedLinkCount && i <= 64; ++i)
        {
            std::string followName = std::string("Follow Link ") + std::to_string(i);
            props.push_back(Property(DatumType::Bool, followName.c_str(), this, &mFollowLinkExtra[i]));
        }

        props.push_back(Property(DatumType::Float, "Link 1 Speed Modifier", this, &mLinkSpeedModifier1));
        props.push_back(Property(DatumType::Float, "Link 2 Speed Modifier", this, &mLinkSpeedModifier2));
        props.push_back(Property(DatumType::Float, "Link 3 Speed Modifier", this, &mLinkSpeedModifier3));
        props.push_back(Property(DatumType::Float, "Link 4 Speed Modifier", this, &mLinkSpeedModifier4));
        props.push_back(Property(DatumType::Float, "Link 5 Speed Modifier", this, &mLinkSpeedModifier5));
        props.push_back(Property(DatumType::Float, "Link 6 Speed Modifier", this, &mLinkSpeedModifier6));
        props.push_back(Property(DatumType::Float, "Link 7 Speed Modifier", this, &mLinkSpeedModifier7));
        props.push_back(Property(DatumType::Float, "Link 8 Speed Modifier", this, &mLinkSpeedModifier8));
        props.push_back(Property(DatumType::Float, "Link 9 Speed Modifier", this, &mLinkSpeedModifier9));
        props.push_back(Property(DatumType::Float, "Link 10 Speed Modifier", this, &mLinkSpeedModifier10));
        if (mGeneratedLinkCount >= 11)
        {
            props.push_back(Property(DatumType::Float, "Link 11 Speed Modifier", this, &mLinkSpeedModifier11));
        }
        for (int32_t i = 12; i <= mGeneratedLinkCount && i <= 64; ++i)
        {
            std::string speedName = std::string("Link ") + std::to_string(i) + " Speed Modifier";
            props.push_back(Property(DatumType::Float, speedName.c_str(), this, &mLinkSpeedModifierExtra[i]));
        }

        props.push_back(Property(DatumType::Bool, "Disable Ping-Pong", this, &mDisableBounce));
        props.push_back(Property(DatumType::Bool, "Pause", this, &mPause));
        props.push_back(Property(DatumType::Float, "Link Speed Modifier", this, &mLinkSpeedModifier));
        props.push_back(Property(DatumType::Bool, "Link Smooth Step", this, &mLinkSmoothStep));
        props.push_back(Property(DatumType::Bool, "Link Smooth Rotate", this, &mLinkSmoothRotate));
    }
}

void Spline3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    Node3D::GatherProxyDraws(inoutDraws);

#if DEBUG_DRAW_ENABLED
    bool showSplineLines = sSplineLinesVisible;

#if EDITOR
    // Always show spline lines while editing; toggle only affects PIE/runtime.
    if (GetEditorState() != nullptr && !GetEditorState()->mPlayInEditor)
    {
        showSplineLines = true;
    }
#endif

    if (!showSplineLines)
        return;

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

            // Arrow head to indicate direction (p0 -> p1)
            glm::vec3 dir = Maths::SafeNormalize(p1 - p0);
            glm::vec3 side = Maths::SafeNormalize(glm::cross(dir, glm::vec3(0,1,0)));
            if (glm::length(side) < 0.0001f)
                side = Maths::SafeNormalize(glm::cross(dir, glm::vec3(1,0,0)));
            glm::vec3 up = Maths::SafeNormalize(glm::cross(side, dir));

            float arrowLen = 0.3f;
            float arrowWidth = 0.3f;
            glm::vec3 head = p1;
            glm::vec3 base = head - dir * arrowLen;
            // White arrows
            world->AddLine(Line(base + side * arrowWidth, head, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
            world->AddLine(Line(base - side * arrowWidth, head, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
            world->AddLine(Line(base + up * arrowWidth, head, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
            world->AddLine(Line(base - up * arrowWidth, head, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));

            // Also add a small arrow near the start of the spline
            if (i == 1)
            {
                glm::vec3 startHead = p0;
                glm::vec3 sbase = startHead - dir * arrowLen;
                world->AddLine(Line(sbase + side * arrowWidth, startHead, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
                world->AddLine(Line(sbase - side * arrowWidth, startHead, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
                world->AddLine(Line(sbase + up * arrowWidth, startHead, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
                world->AddLine(Line(sbase - up * arrowWidth, startHead, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.01f));
            }
        }

        if (mCloseLoop && points.size() > 2)
        {
            glm::vec3 p0 = points.back().node->GetWorldPosition();
            glm::vec3 p1 = points.front().node->GetWorldPosition();
            world->AddLine(Line(p0, p1, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.01f));
        }

        auto drawLink = [&](NodePtrWeak& fromPtr, NodePtrWeak& toPtr)
        {
            Node* from = fromPtr.Get();
            Node* to = toPtr.Get();
            if (from && to)
            {
                Node3D* from3d = from->As<Node3D>();
                Node3D* to3d = to->As<Node3D>();
                if (from3d && to3d)
                {
                    glm::vec3 p0 = from3d->GetWorldPosition();
                    glm::vec3 p1 = to3d->GetWorldPosition();
                    world->AddLine(Line(p0, p1, glm::vec4(0.2f, 1.0f, 1.0f, 1.0f), 0.01f));
                }
            }
        };

        drawLink(mLinkFrom, mLinkTo);
        drawLink(mLinkFrom2, mLinkTo2);
        drawLink(mLinkFrom3, mLinkTo3);
        drawLink(mLinkFrom4, mLinkTo4);
        drawLink(mLinkFrom5, mLinkTo5);
        drawLink(mLinkFrom6, mLinkTo6);
        drawLink(mLinkFrom7, mLinkTo7);
        drawLink(mLinkFrom8, mLinkTo8);
        drawLink(mLinkFrom9, mLinkTo9);
        drawLink(mLinkFrom10, mLinkTo10);
        if (mGeneratedLinkCount >= 11)
        {
            drawLink(mLinkFrom11, mLinkTo11);
        }
        for (int32_t i = 12; i <= mGeneratedLinkCount && i <= 64; ++i)
        {
            drawLink(mLinkFromExtra[i], mLinkToExtra[i]);
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
    else if (prop->mName == "Node3D")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mAttachmentNode3D = WeakPtr<Node>();
            success = false;
        }
        else if (node->As<Node3D>())
        {
            spline->mAttachmentNode3D = newNode;
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
    else if (prop->mName == "From")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mLinkFrom = WeakPtr<Node>();
            success = false;
        }
        else if (node->GetName().rfind("point", 0) == 0)
        {
            spline->mLinkFrom = newNode;
            success = false;
        }
        else
        {
            success = true;
        }
    }
    else if (prop->mName == "To")
    {
        const WeakPtr<Node>& newNode = *(const WeakPtr<Node>*)newValue;
        Node* node = newNode.Get();

        if (node == nullptr)
        {
            spline->mLinkTo = WeakPtr<Node>();
            success = false;
        }
        else if (node->GetName().rfind("point", 0) == 0)
        {
            spline->mLinkTo = newNode;
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

    if (prop.mName == "Generate Link")
    {
        if (ImGui::Button("Generate Next Link") && mGeneratedLinkCount < 64)
        {
            ++mGeneratedLinkCount;
            if (mGeneratedLinkCount == 11)
            {
                mEnableLink11 = true;
            }
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


