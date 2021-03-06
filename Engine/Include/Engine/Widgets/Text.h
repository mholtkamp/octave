#pragma once

#include "Widgets/Widget.h"
#include "Vertex.h"

#include "AssetRef.h"

#include "Graphics/Graphics.h"

class Font;

class Text : public Widget
{
public:

    DECLARE_FACTORY(Text, Widget);

    Text();
    virtual ~Text();

    TextResource* GetResource();

    virtual void Update() override;
    virtual void SetColor(glm::vec4 color) override;

    void SetFont(Font* font);
    Font* GetFont();

    void SetOutlineColor(glm::vec4 color);
    glm::vec4 GetOutlineColor() const;

    void SetSize(float size);
    float GetSize() const;
    float GetScaledSize() const;

    float GetOutlineSize() const;
    float GetSoftness() const;
    float GetCutoff() const;

    void SetText(const std::string& text);
    void SetText(const char* text);
    const std::string& GetText() const;

    VertexUI* GetVertices();
    uint32_t GetNumCharactersAllocated() const;
    uint32_t GetNumVisibleCharacters() const;

    void MarkVertexBufferDirty();
    bool IsVertexBufferDirty() const;

    float GetTextWidth();
    float GetTextHeight();
    glm::vec2 GetScaledMinExtent();
    glm::vec2 GetScaledMaxExtent();

    virtual bool ContainsPoint(int32_t x, int32_t y) override;

    virtual void Render() override;

protected:

    void UpdateVertexData();

    FontRef mFont;
    std::string mText;
    float mCutoff;
    float mOutlineSize;
    float mSize;
    float mSoftness;
    glm::vec4 mOutlineColor;
    VertexUI* mVertices;

    glm::vec2 mMinExtent = {};
    glm::vec2 mMaxExtent = {};
    int32_t mVisibleCharacters; // ( \n excluded )
    uint32_t mNumCharactersAllocated;
    bool mVertexBufferDirty[MAX_FRAMES] = {};

    // Graphics Resource
    TextResource mResource;
};