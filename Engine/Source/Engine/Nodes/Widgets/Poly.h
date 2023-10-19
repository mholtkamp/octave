#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Assets/Texture.h"
#include "AssetRef.h"
#include "Vertex.h"

class Poly : public Widget
{
public:

    DECLARE_WIDGET(Poly, Widget);

    Poly();
    virtual ~Poly();

    PolyResource* GetResource();

    virtual void Update() override;
    virtual void Render() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;

    void MarkVerticesDirty();

    VertexUI* GetVertices();

    void AddVertex(glm::vec2 position,
        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 
        glm::vec2 uv = glm::vec2(0.0f, 0.0f));
    void ClearVertices();
    uint32_t GetNumVertices();
    VertexUI GetVertex(uint32_t index);

    void SetTexture(Texture* texture);
    Texture* GetTexture();

    float GetLineWidth() const;
    void SetLineWidth(float width);

protected:

    std::vector<VertexUI> mVertices;
    TextureRef mTexture; // TODO: Add support for filled polygons
    float mLineWidth = 2.0f;
    
    bool mUploadVertices[MAX_FRAMES] = {};
    PolyResource mResource;
};

