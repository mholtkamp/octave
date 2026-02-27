#pragma once

#include "Nodes/3D/Box3d.h"

class NavMesh3D : public Box3D
{
public:

    DECLARE_NODE(NavMesh3D, Box3D);

    NavMesh3D();
    ~NavMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    void SetNavBounds(bool navBounds);
    bool IsNavBounds() const;
    bool IsNavOverlayEnabled() const;
    bool IsNavNegatorEnabled() const;
    bool IsCullWallsEnabled() const;
    float GetWallCullThreshold() const;
protected:
    bool mNavBounds = true;
    bool mNavOverlay = true;
    bool mNavNegator = false;
    bool mCullWalls = false;
    float mWallCullThreshold = 0.2f;
    float mNavOverlayLineThickness = 1.0f;
    glm::vec4 mNavOverlayWireColor = glm::vec4(0.1f, 1.0f, 0.25f, 1.0f);
};
