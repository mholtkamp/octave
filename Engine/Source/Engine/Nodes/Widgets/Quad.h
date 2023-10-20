#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Assets/Texture.h"
#include "Vertex.h"
#include "AssetRef.h"

#include "glm/glm.hpp"

class Quad : public Widget
{
public:

    DECLARE_NODE(Quad, Widget);

    Quad();
    virtual ~Quad();

    QuadResource* GetResource();

    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Tick(float deltaTime) override;

    void SetTexture(class Texture* texture);
    Texture* GetTexture();

    virtual void SetColor(glm::vec4 color) override;

    void SetColor(glm::vec4 colors[4]);
    void SetColor(glm::vec4 topLeft,
                  glm::vec4 topRight,
                  glm::vec4 bottomLeft,
                  glm::vec4 bottomRight);

    void SetTint(glm::vec4 tint);
    glm::vec4 GetTint() const;

    void SetUvScale(glm::vec2 scale);
    glm::vec2 GetUvScale() const;

    void SetUvOffset(glm::vec2 offset);
    glm::vec2 GetUvOffset() const;

    virtual void Render() override;

    VertexUI* GetVertices();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void InitVertexData();
    void UpdateVertexData();

    TextureRef mTexture;
    VertexUI mVertices[4];

    glm::vec4 mTint;
    glm::vec2 mUvScale;
    glm::vec2 mUvOffset;

    // Graphics Resource
    QuadResource mResource;
};