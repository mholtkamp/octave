#include "Nodes/3D/NavMesh3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Assets/StaticMesh.h"
#include "World.h"
#include "Line.h"
#include <cmath>

FORCE_LINK_DEF(NavMesh3D);
DEFINE_NODE(NavMesh3D, Box3D);

NavMesh3D::NavMesh3D()
{
    mName = "NavMesh3D";
    SetNavBounds(true);
}

NavMesh3D::~NavMesh3D()
{
}

const char* NavMesh3D::GetTypeName() const
{
    return "NavMesh3D";
}

void NavMesh3D::Create()
{
    Box3D::Create();
    SetNavBounds(true);
}

void NavMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    // Intentionally skip Primitive3D/Box3D property gather so NavMesh3D doesn't expose the Primitive chunk.
    Node3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Nav");
    outProps.push_back(Property(DatumType::Vector, "Extents", this, &mExtents, 1, Box3D::HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Nav Bounds", this, &mNavBounds));
    outProps.push_back(Property(DatumType::Bool, "Nav Overlay", this, &mNavOverlay));
    outProps.push_back(Property(DatumType::Color, "Nav Overlay Wire Color", this, &mNavOverlayWireColor));
    outProps.push_back(Property(DatumType::Float, "Nav Overlay Line Thickness", this, &mNavOverlayLineThickness));
    outProps.push_back(Property(DatumType::Bool, "Nav Negator", this, &mNavNegator));
    outProps.push_back(Property(DatumType::Bool, "Cull 90� Walls", this, &mCullWalls));
    outProps.push_back(Property(DatumType::Float, "Wall Cull Threshold", this, &mWallCullThreshold));
}

void NavMesh3D::SetNavBounds(bool navBounds)
{
    mNavBounds = navBounds;
}

bool NavMesh3D::IsNavBounds() const
{
    return mNavBounds;
}

bool NavMesh3D::IsNavOverlayEnabled() const
{
    return mNavOverlay;
}

bool NavMesh3D::IsNavNegatorEnabled() const
{
    return mNavNegator;
}

bool NavMesh3D::IsCullWallsEnabled() const
{
    return mCullWalls;
}

float NavMesh3D::GetWallCullThreshold() const
{
    return mWallCullThreshold;
}

void NavMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    // Use primitive debug behavior but avoid Box3D solid cube draw so NavMesh3D appears as an untriangulated wire box.
    Primitive3D::GatherProxyDraws(inoutDraws);

    World* world = GetWorld();
    if (!world)
    {
        return;
    }

    const glm::vec3 h = GetExtents() * 0.5f;
    const glm::mat4 m = GetTransform();

    auto toWorld = [&](float x, float y, float z) -> glm::vec3
    {
        glm::vec4 p = m * glm::vec4(x, y, z, 1.0f);
        return glm::vec3(p.x, p.y, p.z);
    };

    glm::vec3 c[8] = {
        toWorld(-h.x, -h.y, -h.z), toWorld( h.x, -h.y, -h.z),
        toWorld( h.x,  h.y, -h.z), toWorld(-h.x,  h.y, -h.z),
        toWorld(-h.x, -h.y,  h.z), toWorld( h.x, -h.y,  h.z),
        toWorld( h.x,  h.y,  h.z), toWorld(-h.x,  h.y,  h.z)
    };

    const glm::vec4 col(0.15f, 0.95f, 0.95f, 1.0f);
    const float life = 0.02f;

    auto addThickLine = [&](const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& lineCol)
    {
        world->AddLine(Line(p0, p1, lineCol, life));

        if (mNavOverlayLineThickness <= 0.0f)
        {
            return;
        }

        glm::vec3 dir = p1 - p0;
        float d2 = glm::dot(dir, dir);
        if (d2 <= 1e-8f)
        {
            return;
        }
        dir = glm::normalize(dir);

        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 side = glm::cross(dir, up);
        float s2 = glm::dot(side, side);
        if (s2 <= 1e-8f)
        {
            side = glm::cross(dir, glm::vec3(1.0f, 0.0f, 0.0f));
            s2 = glm::dot(side, side);
            if (s2 <= 1e-8f)
            {
                return;
            }
        }
        side = glm::normalize(side);

        // Thickness slider is treated as a small visual multiplier, not raw world units.
        const float t = fmaxf(0.0f, fminf(8.0f, mNavOverlayLineThickness));
        const float off = t * 0.0025f;
        if (off <= 0.0f)
        {
            return;
        }

        world->AddLine(Line(p0 + side * off, p1 + side * off, lineCol, life));
        world->AddLine(Line(p0 - side * off, p1 - side * off, lineCol, life));
    };

    auto edge = [&](int a, int b) { addThickLine(c[a], c[b], col); };

    edge(0,1); edge(1,2); edge(2,3); edge(3,0);
    edge(4,5); edge(5,6); edge(6,7); edge(7,4);
    edge(0,4); edge(1,5); edge(2,6); edge(3,7);

    if (!mNavOverlay)
    {
        return;
    }

    const glm::mat4 inv = glm::inverse(m);
    const bool isNegator = mNavNegator;
    const glm::vec4 navBaseCol = isNegator ? glm::vec4(1.0f, 0.25f, 0.25f, 1.0f) : mNavOverlayWireColor;
    const glm::vec4 navCol(navBaseCol.r, navBaseCol.g, navBaseCol.b, 1.0f);
    const float fillAlpha = fmaxf(0.05f, fminf(1.0f, navBaseCol.a * 0.25f));
    const glm::vec4 navFillCol(navBaseCol.r, navBaseCol.g, navBaseCol.b, fillAlpha);
    int32_t edgeBudget = 6000;

    std::vector<StaticMesh3D*> navMeshes;
    world->FindNodes<StaticMesh3D>(navMeshes);

    std::vector<NavMesh3D*> navBoxes;
    world->FindNodes<NavMesh3D>(navBoxes);

    auto inside = [&](const glm::vec3& p) -> bool
    {
        glm::vec4 lp4 = inv * glm::vec4(p, 1.0f);
        glm::vec3 lp(lp4.x, lp4.y, lp4.z);
        return fabsf(lp.x) <= h.x && fabsf(lp.y) <= h.y && fabsf(lp.z) <= h.z;
    };

    auto insideNegator = [&](const glm::vec3& p) -> bool
    {
        for (NavMesh3D* box : navBoxes)
        {
            if (!box || box == this || !box->IsNavBounds() || !box->IsNavNegatorEnabled())
            {
                continue;
            }

            glm::vec3 bh = box->GetExtents() * 0.5f;
            glm::mat4 binv = glm::inverse(box->GetTransform());
            glm::vec4 lp4 = binv * glm::vec4(p, 1.0f);
            glm::vec3 lp(lp4.x, lp4.y, lp4.z);
            if (fabsf(lp.x) <= bh.x && fabsf(lp.y) <= bh.y && fabsf(lp.z) <= bh.z)
            {
                return true;
            }
        }
        return false;
    };

    for (StaticMesh3D* meshNode : navMeshes)
    {
        if (!meshNode || !meshNode->IsNavmeshReady() || edgeBudget <= 0)
        {
            continue;
        }

        StaticMesh* sm = meshNode->GetStaticMesh();
        if (!sm)
        {
            continue;
        }

        Vertex* verts = sm->GetVertices();
        IndexType* idx = sm->GetIndices();
        uint32_t nv = sm->GetNumVertices();
        uint32_t ni = sm->GetNumIndices();
        if (!verts || !idx || nv == 0 || ni < 3)
        {
            continue;
        }

        const glm::mat4 tm = meshNode->GetTransform();
        for (uint32_t i = 0; i + 2 < ni && edgeBudget > 0; i += 3)
        {
            uint32_t ia = (uint32_t)idx[i + 0];
            uint32_t ib = (uint32_t)idx[i + 1];
            uint32_t ic = (uint32_t)idx[i + 2];
            if (ia >= nv || ib >= nv || ic >= nv)
            {
                continue;
            }

            glm::vec4 a4 = tm * glm::vec4(verts[ia].mPosition, 1.0f);
            glm::vec4 b4 = tm * glm::vec4(verts[ib].mPosition, 1.0f);
            glm::vec4 c4 = tm * glm::vec4(verts[ic].mPosition, 1.0f);
            glm::vec3 a(a4.x, a4.y, a4.z);
            glm::vec3 b(b4.x, b4.y, b4.z);
            glm::vec3 ctri(c4.x, c4.y, c4.z);

            glm::vec3 centroid = (a + b + ctri) / 3.0f;
            if (!inside(centroid))
            {
                continue;
            }

            if (mCullWalls)
            {
                glm::vec3 n = glm::cross(b - a, ctri - a);
                float nlen2 = glm::dot(n, n);
                if (nlen2 > 1e-8f)
                {
                    n = glm::normalize(n);
                    if (fabsf(n.y) < mWallCullThreshold)
                    {
                        continue;
                    }
                }
            }

            if (!isNegator && insideNegator(centroid))
            {
                continue;
            }

            addThickLine(a, b, navCol);
            addThickLine(b, ctri, navCol);
            addThickLine(ctri, a, navCol);
            edgeBudget -= 3;

            // Faux semi-transparent face fill using a few interior hatch lines.
            const int kFillSteps = 3;
            for (int s = 1; s <= kFillSteps && edgeBudget > 0; ++s)
            {
                float t = (float)s / (float)(kFillSteps + 1);
                glm::vec3 p0 = a + (b - a) * t;
                glm::vec3 p1 = a + (ctri - a) * t;
                world->AddLine(Line(p0, p1, navFillCol, life));
                edgeBudget -= 1;
            }
        }
    }
#endif
}

