#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Assets/Texture.h"
#include "Vertex.h"
#include "AssetRef.h"

#include "glm/glm.hpp"

enum class ObjectFit : uint8_t
{
    Fill,
    Contain,
    Cover,
    None,
    Count
};

class OCTAVE_API Quad : public Widget
{
public:

    DECLARE_NODE(Quad, Widget);

    static constexpr int32_t kCornerSegments = 8;
    static constexpr int32_t kMaxQuadVertices = 2 + 4 * (kCornerSegments + 1); // 38

    friend class Button;

    Quad();
    virtual ~Quad();

    virtual void Create() override;
    virtual void Destroy() override;

    QuadResource* GetResource();

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    void GatherQuadProperties(std::vector<Property>& outProps);

    virtual DrawData GetDrawData() override;

    virtual void Render() override;
    virtual void PreRender() override;

    void SetTexture(class Texture* texture);
    Texture* GetTexture();

    virtual void SetColor(glm::vec4 color) override;

    void SetColor(glm::vec4 colors[4]);
    void SetColor(glm::vec4 topLeft,
                  glm::vec4 topRight,
                  glm::vec4 bottomLeft,
                  glm::vec4 bottomRight);

    void SetUvScale(glm::vec2 scale);
    glm::vec2 GetUvScale() const;

    void SetUvOffset(glm::vec2 offset);
    glm::vec2 GetUvOffset() const;

    void SetObjectFit(ObjectFit fit);
    ObjectFit GetObjectFit() const;

    void SetCornerRadius(float radius);
    float GetCornerRadius() const;

    void SetBorderWidth(float width);
    float GetBorderWidth() const;

    void SetBorderColor(glm::vec4 color);
    glm::vec4 GetBorderColor() const;

    VertexUI* GetVertices();
    uint32_t GetNumVertices() const;

    QuadResource* GetBorderResource();
    VertexUI* GetBorderVertices();
    uint32_t GetBorderNumVertices() const;

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void InitVertexData();
    void UpdateVertexData();
    void UpdateBorders();

    static uint32_t GenerateRoundedFan(
        VertexUI* outVertices,
        float posX, float posY, float posW, float posH,
        float cornerRadius,
        float uvX0, float uvY0, float uvX1, float uvY1);

    TextureRef mTexture;
    VertexUI mVertices[kMaxQuadVertices];
    uint32_t mNumVertices = 0;

    glm::vec2 mUvScale;
    glm::vec2 mUvOffset;
    ObjectFit mObjectFit = ObjectFit::Fill;
    float mCornerRadius = 0.0f;

    float mBorderWidth = 0.0f;
    glm::vec4 mBorderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Border geometry (rendered as background pad behind main quad)
    VertexUI mBorderVertices[kMaxQuadVertices];
    uint32_t mBorderNumVertices = 0;
    QuadResource mBorderResource;
    bool mBorderResourceCreated = false;

    // Graphics Resource
    QuadResource mResource;
};
