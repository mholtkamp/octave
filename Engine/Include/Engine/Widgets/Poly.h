#pragma once

#include "Widgets/Widget.h"
#include "Assets/Texture.h"
#include "AssetRef.h"
#include "Vertex.h"

class Poly : public Widget
{
public:

    DECLARE_FACTORY(Poly, Widget);

    Poly();
    virtual ~Poly();

    PolyResource* GetResource();

    virtual void Update() override;
    virtual void Render() override;

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

protected:

    std::vector<VertexUI> mVertices;
    TextureRef mTexture; // TODO: Add support for filled polygons
    bool mUploadVertices[MAX_FRAMES] = {};

    PolyResource mResource;
};

