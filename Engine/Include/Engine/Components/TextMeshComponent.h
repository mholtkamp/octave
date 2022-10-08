#pragma once

#include "Components/MeshComponent.h"
#include "Vertex.h"
#include "Widgets/Text.h"

class Font;

class TextMeshComponent : public MeshComponent
{
public:
    DECLARE_COMPONENT(TextMeshComponent, MeshComponent);

    TextMeshComponent();
    ~TextMeshComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    TextMeshCompResource* GetResource();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;
    virtual void Tick(float deltaTime) override;

    virtual bool IsStaticMeshComponent() const override;
    virtual bool IsSkeletalMeshComponent() const override;
    virtual Material* GetMaterial() override;

    virtual void Render() override;

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetFont(Font* font);
    Font* GetFont() const;

    void SetColor(glm::vec4 color);
    glm::vec4 GetColor() const;

    void SetBlendMode(BlendMode blendMode);
    BlendMode GetBlendMode() const;

    void SetHorizontalJustification(float just);
    float GetHorizontalJustification() const;

    void SetVerticalJustification(float just);
    float GetVerticalJustification() const;

    virtual Bounds GetLocalBounds() const override;

    int32_t GetNumVisibleCharacters() const;

    const Vertex* GetVertices() const;

protected:

    static bool HandlePropChange(Datum* datum, const void* newValue);

    void MarkVerticesDirty();
    void UploadVertexData();
    void UpdateVertexData();
    void JustifyLine(glm::vec2& lineMinExtent, glm::vec2& lineMaxExtent, int32_t& lineVertStart);
    void UpdateBounds();

    FontRef mFont;
    std::string mText;
    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float mHorizontalJustification = 0.0f;
    float mVerticalJustification = 0.0f;
    BlendMode mBlendMode = BlendMode::Masked;

    std::vector<Vertex> mVertices;
    int32_t mVisibleCharacters = 0;
    bool mUploadVertices[MAX_FRAMES] = {};
    bool mReconstructVertices = false;
    Bounds mBounds;
    MaterialRef mDefaultMatInstance = nullptr;

    TextMeshCompResource mResource;
};

