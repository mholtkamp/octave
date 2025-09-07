#pragma once

#include "Rect.h"
#include "EngineTypes.h"
#include "Factory.h"
#include "AssetRef.h"

#include "Nodes/Node.h"

#include "Maths.h"
#include <vector>
#include <string>

#include "Graphics/GraphicsConstants.h"

class Widget;

enum class AnchorMode : uint8_t
{
    TopLeft,
    TopMid,
    TopRight,
    MidLeft,
    Mid,
    MidRight,
    BottomLeft,
    BottomMid,
    BottomRight,

    TopStretch,
    MidHorizontalStretch,
    BottomStretch,

    LeftStretch,
    MidVerticalStretch,
    RightStretch,

    FullStretch,

    Count
};

enum MarginFlag : uint8_t
{
    MF_Left         = 1 << 0,
    MF_Top          = 1 << 1,
    MF_Right        = 1 << 2,
    MF_Bottom       = 1 << 3
};

class Widget : public Node
{
public:

    DECLARE_NODE(Widget, Node);

    Widget();

    virtual void Start() override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void SetVisible(bool visible) override;

    // Refresh any data used for rendering based on this widget's state. Use dirty flag.
    // Recursively update children.
    virtual void PrepareTick(std::vector<NodePtrWeak>& outTickNodes, bool game, bool recurse) override;
    virtual void Render() override;
    virtual VertexType GetVertexType() const override;

    virtual bool IsWidget() const override;
    Widget* GetParentWidget();
    const Widget* GetParentWidget() const;
    Widget* GetChildWidget(int32_t index);

    virtual void PreRender();

    Rect GetRect();

    void SetX(float x);
    void SetY(float y);
    void SetWidth(float width);
    void SetHeight(float height);

    void SetXRatio(float x);
    void SetYRatio(float y);
    void SetWidthRatio(float width);
    void SetHeightRatio(float height);

    void SetLeftMargin(float left);
    void SetTopMargin(float top);
    void SetRightMargin(float right);
    void SetBottomMargin(float bottom);

    virtual void SetPosition(float x, float y);
    virtual void SetDimensions(float width, float height);
    void SetPosition(glm::vec2 position);
    void SetDimensions(glm::vec2 dimensions);
    void SetRect(float x, float y, float width, float height);
    void SetRect(glm::vec2 position, glm::vec2 dimensions);
    void SetRect(Rect rect);

    void SetRatios(float x, float y, float width, float height);
    void SetMargins(float left, float top, float right, float bottom);

    // Offset may be stored in pixels or parent-percentage.
    // Use SetPosition() to set offset from pixel space.
    // Use GetPosition(), GetX(), GetY(), to get offset in pixel space.
    void SetOffset(float x, float y);
    glm::vec2 GetOffset() const;

    // Size may be stored in pixels or parent-percentage.
    // Use SetDimensions() to set size from pixel space.
    // Use GetDimensions(), GetWidth(), GetHeight(), to get size in pixel space.
    void SetSize(float x, float y);
    glm::vec2 GetSize() const;

    AnchorMode GetAnchorMode() const;
    void SetAnchorMode(AnchorMode anchorMode);
    bool AnchorStretchesX(AnchorMode mode) const;
    bool AnchorStretchesY(AnchorMode mode) const;
    inline bool StretchX() const { return AnchorStretchesX(mAnchorMode); }
    inline bool StretchY() const { return AnchorStretchesY(mAnchorMode); }
    glm::vec2 GetAnchorRatio() const;

    float GetX() const;
    float GetY() const;
    float GetWidth() const;
    float GetHeight() const;

    glm::vec2 GetPosition() const;
    glm::vec2 GetDimensions() const;

    void UpdateRect();
    void UpdateColor();
    void FitInsideParent();

    float GetParentWidth() const;
    float GetParentHeight() const;

    virtual void SetColor(glm::vec4 color);
    glm::vec4 GetColor() const;

    void SetOpacity(uint8_t opacity);
    uint8_t GetOpacity() const;

    void SetOpacityFloat(float opacity);
    float GetOpacityFloat() const;
    
    virtual bool ShouldHandleInput();

    virtual void MarkDirty();
    void MarkClean();
    bool IsDirty() const;

    static float InterfaceToNormalized(float interfaceCoord, float interfaceSize);
    static bool IsMouseInside(Rect rect);

    bool ContainsMouse(bool testScissor = true) const;
    virtual bool ContainsPoint(int32_t x, int32_t y);
    void MoveToMousePosition();

    const glm::mat3& GetTransform();
    void SetRotation(float degrees);
    float GetRotation() const;

    void SetPivot(glm::vec2 pivot);
    glm::vec2 GetPivot() const;

    void SetScale(glm::vec2 scale);
    glm::vec2 GetScale() const;

    bool IsScissorEnabled() const;
    void EnableScissor(bool enable);
    Rect GetScissorRect() const;

protected:

    static WeakPtr<Widget> sWidgetToClean;
    static const char* sAnchorModeStrings[];

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    virtual void SetParent(Node* parent) override;

    float PixelsToRatioX(float x) const;
    float PixelsToRatioY(float y) const;
    float RatioToPixelsX(float x) const;
    float RatioToPixelsY(float y) const;

    void PushScissor();
    void PopScissor();
    void SetScissor(Rect& area);

    Rect mRect; // The screen pos/dimensions that are computed on Update().
    Rect mScissorRect;
    glm::mat3 mTransform;
    glm::vec4 mColor;
    glm::vec2 mOffset;
    glm::vec2 mSize;
    glm::vec2 mPivot;
    glm::vec2 mScale;
    glm::vec2 mAbsoluteScale;
    float mRotation;
    AnchorMode mAnchorMode;
    uint8_t mActiveMargins;
    bool mUseScissor;
    uint8_t mOpacity;

private:
    bool mDirty[MAX_FRAMES] = {};
};
