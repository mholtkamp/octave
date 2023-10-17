#pragma once

#include "LightComponent.h"

class DirectionalLightComponent : public LightComponent
{
public:

    DECLARE_NODE(DirectionalLightComponent, LightComponent);

    DirectionalLightComponent();
    ~DirectionalLightComponent();

    virtual void SetOwner(Actor* owner) override;
    virtual void Destroy() override;

    virtual void Tick(float deltaTime) override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual bool IsPointLightComponent() const override;
    virtual bool IsDirectionalLightComponent() const override;

    glm::vec3 GetDirection() const;
    void SetDirection(const glm::vec3& dir);

    const glm::mat4& GetViewProjectionMatrix() const;

protected:

    void GenerateViewProjectionMatrix();

    glm::mat4 mViewProjectionMatrix;
};