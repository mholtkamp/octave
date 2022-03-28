#pragma once

#include "LightComponent.h"

class DirectionalLightComponent : public LightComponent
{
public:

    DECLARE_COMPONENT(DirectionalLightComponent, LightComponent);

    DirectionalLightComponent();
    ~DirectionalLightComponent();

    virtual void SetOwner(Actor* owner) override;
    virtual void Destroy() override;

    virtual void Tick(float deltaTime) override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual bool IsPointLightComponent() const override;
    virtual bool IsDirectionalLightComponent() const override;

    const glm::vec3& GetDirection() const;
    void SetDirection(const glm::vec3& dir);

    const glm::mat4& GetViewProjectionMatrix() const;

protected:

    void GenerateViewProjectionMatrix();

    glm::vec3 mDirection;
    glm::mat4 mViewProjectionMatrix;
};