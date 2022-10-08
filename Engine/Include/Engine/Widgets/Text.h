#pragma once

#include "Widgets/Widget.h"
#include "Vertex.h"

#include "AssetRef.h"

#include "Graphics/Graphics.h"

class Font;

enum class Justification : uint8_t
{
    Left,
    Center,
    Right,
    Top,
    Bottom,

    Count
};

class Text : public Widget
{
public:

    DECLARE_FACTORY(Text, Widget);

    Text();
    virtual ~Text();

    TextResource* GetResource();

    virtual void Update() override;
    virtual void SetColor(glm::vec4 color) override;

    virtual void MarkDirty();

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

    void SetHorizontalJustification(Justification just);
    Justification GetHorizontalJustification() const;
    void SetVerticalJustification(Justification just);
    Justification GetVerticalJustification() const;

    bool IsWordWrapEnabled() const;
    void EnableWordWrap(bool wrap);

    void SetText(const std::string& text);
    void SetText(const char* text);
    const std::string& GetText() const;

    VertexUI* GetVertices();
    uint32_t GetNumCharactersAllocated() const;
    uint32_t GetNumVisibleCharacters() const;

    void MarkVerticesDirty();

    float GetTextWidth();
    float GetTextHeight();
    glm::vec2 GetScaledMinExtent();
    glm::vec2 GetScaledMaxExtent();

    virtual bool ContainsPoint(int32_t x, int32_t y) override;

    virtual void Render() override;

    glm::vec2 GetJustifiedOffset();
    static float GetJustificationRatio(Justification just);

protected:

    void UpdateVertexData();
    void UploadVertexData();
    void JustifyLine(VertexUI* vertices, Justification just, int32_t& lineVertStart, int32_t numVerts);

    FontRef mFont;
    std::string mText;
    float mCutoff;
    float mOutlineSize;
    float mSize;
    float mSoftness;
    glm::vec4 mOutlineColor;
    VertexUI* mVertices;
    bool mWordWrap = false;

    glm::vec2 mMinExtent = {};
    glm::vec2 mMaxExtent = {};
    int32_t mVisibleCharacters; // ( \n excluded )
    uint32_t mNumCharactersAllocated;
    Justification mHoriJust = Justification::Left;
    Justification mVertJust = Justification::Top;
    bool mUploadVertices[MAX_FRAMES] = {};
    bool mReconstructVertices = false;

    // Graphics Resource
    TextResource mResource;
};