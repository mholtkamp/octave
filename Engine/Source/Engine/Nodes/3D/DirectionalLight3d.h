#pragma once

#include "Light3d.h"

class DirectionalLight3D : public Light3D
{
public:

    DECLARE_NODE(DirectionalLight3D, Light3D);

    DirectionalLight3D();
    ~DirectionalLight3D();

    virtual void Destroy() override;

    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual bool IsPointLight3D() const override;
    virtual bool IsDirectionalLight3D() const override;

    glm::vec3 GetDirection() const;
    void SetDirection(const glm::vec3& dir);

    const glm::mat4& GetViewProjectionMatrix() const;

protected:

    void TickCommon(float deltaTime);
    void GenerateViewProjectionMatrix();

    glm::mat4 mViewProjectionMatrix;
};