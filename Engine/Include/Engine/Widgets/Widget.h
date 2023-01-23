#pragma once

#include "Rect.h"
#include "EngineTypes.h"
#include "Factory.h"

#include "AssetRef.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Graphics/GraphicsConstants.h"

class Widget;
class WidgetMap;

#define DECLARE_WIDGET(Base, Parent) \
        DECLARE_FACTORY(Base, Widget); \
        DECLARE_RTTI(Base, Parent);

#define DEFINE_WIDGET(Base, Parent) \
        DEFINE_FACTORY(Base, Widget); \
        DEFINE_RTTI(Base);

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

Widget* CreateWidget(TypeId widgetType, bool start = true);
Widget* CreateWidget(const std::string& className, bool start = true);
void DestroyWidget(Widget* widget);

template<typename WidgetClass>
inline WidgetClass* CreateWidget(bool start = true)
{
    WidgetClass* retWidget = (WidgetClass*)CreateWidget(WidgetClass::GetStaticType(), start);
    return retWidget;
}


class Widget : public RTTI
{
public:

    DECLARE_FACTORY_MANAGER(Widget);
    DECLARE_FACTORY(Widget, Widget);
    DECLARE_RTTI(Widget, RTTI);

    Widget();
    virtual ~Widget();

    virtual void GatherProperties(std::vector<Property>& outProps);

    Widget* Clone();

    // Issue gpu commands to display the widget.
    // Recursively render children.
    void RecursiveRender();
    virtual void Render();

    // Refresh any data used for rendering based on this widget's state. Use dirty flag.
    // Recursively update children.
    void RecursiveUpdate();
    virtual void Update();

    virtual void Start();
    virtual void Stop();

    void SetName(const std::string& name);
    const std::string& GetName() const;

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
    void FitInsideParent();

    float GetParentWidth() const;
    float GetParentHeight() const;

    virtual void SetVisible(bool visible);
    bool IsVisible() const;
    virtual void SetColor(glm::vec4 color);
    glm::vec4 GetColor() const;
    
    virtual bool ShouldHandleInput();

    void AddChild(Widget* widget, int32_t index = -1);
    Widget* RemoveChild(Widget* widget);
    Widget* RemoveChild(int32_t index);
    Widget* GetChild(int32_t index);
    Widget* GetParent();
    void DetachFromParent();
    uint32_t GetNumChildren() const;
    Widget* FindChild(const std::string& name, bool recurse = false);

    virtual void MarkDirty();
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

    bool IsScriptOwned() const;
    void SetScriptOwned(bool scriptOwned);

    bool HasParent(Widget* widget);

    template<typename T>
    T* CreateChildWidget()
    {
        T* ret = new T();
        ret->Start();
        AddChild(ret);
        return ret;
    }

    static void ResetScissor();

protected:

    static bool HandlePropChange(Datum* datum, const void* newValue);
    static Rect sCurrentScissor;

    float PixelsToRatioX(float x) const;
    float PixelsToRatioY(float y) const;
    float RatioToPixelsX(float x) const;
    float RatioToPixelsY(float y) const;

    void PushScissor();
    void PopScissor();
    void SetScissor(Rect& area);

    Widget* mParent;
    std::vector<Widget*> mChildren;
    std::string mName;

    Rect mRect; // The screen pos/dimensions that are computed on Update().
    Rect mCachedScissorRect;
    Rect mCachedParentScissorRect;
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
    bool mVisible;
    bool mScriptOwned;
    bool mStarted;

private:
    bool mDirty[MAX_FRAMES] = {};


#if EDITOR
public:
    WidgetMap* GetWidgetMap();
    void SetWidgetMap(WidgetMap* map);

    bool ShouldExposeVariable() const;
    void SetExposeVariable(bool expose);
protected:
    // This is only used by the Widget editor.
    WidgetMapRef mWidgetMap;
    bool mExposeVariable = false;

#endif
};
