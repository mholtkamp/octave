#include "Nodes/Widgets/Canvas.h"
#include "UI/UIDocument.h"
#include "Renderer.h"
#include "Engine.h"
#include "Log.h"

#if EDITOR
#include "imgui.h"
#endif

static const char* AnchorModeToString(AnchorMode mode)
{
    switch (mode)
    {
    case AnchorMode::TopLeft: return "TopLeft";
    case AnchorMode::TopMid: return "TopMid";
    case AnchorMode::TopRight: return "TopRight";
    case AnchorMode::MidLeft: return "MidLeft";
    case AnchorMode::Mid: return "Mid";
    case AnchorMode::MidRight: return "MidRight";
    case AnchorMode::BottomLeft: return "BottomLeft";
    case AnchorMode::BottomMid: return "BottomMid";
    case AnchorMode::BottomRight: return "BottomRight";
    case AnchorMode::TopStretch: return "TopStretch";
    case AnchorMode::MidHorizontalStretch: return "MidHStretch";
    case AnchorMode::BottomStretch: return "BottomStretch";
    case AnchorMode::LeftStretch: return "LeftStretch";
    case AnchorMode::MidVerticalStretch: return "MidVStretch";
    case AnchorMode::RightStretch: return "RightStretch";
    case AnchorMode::FullStretch: return "FullStretch";
    default: return "Unknown";
    }
}

FORCE_LINK_DEF(Canvas);
DEFINE_NODE(Canvas, Widget);

Canvas::Canvas()
{
    mUseScissor = true;
}

Canvas::~Canvas()
{
    DestroyRuntimeDocument();
}

void Canvas::Start()
{
    Widget::Start();

    // Tear down any editor preview before creating the game document
    DestroyEditorPreview();

    UIDocument* sourceDoc = static_cast<UIDocument*>(mUIDocumentRef.Get());
    if (sourceDoc != nullptr && !sourceDoc->GetXmlSource().empty())
    {
        // Create a runtime copy so multiple Canvas nodes can reference the same asset
        mRuntimeDocument = new UIDocument();
        mRuntimeDocument->Create();
        mRuntimeDocument->SetXmlSource(sourceDoc->GetXmlSource());
        mRuntimeDocument->SetBasePath(sourceDoc->GetBasePath());
        mRuntimeDocument->SetSourceFilePath(sourceDoc->GetSourceFilePath());
        mRuntimeDocument->Instantiate();
        mRuntimeDocument->Mount(this);

        // Schedule debug dump after a few frames so layout has settled
        mDebugDumpCountdown = 3;
    }
}

void Canvas::Stop()
{
    DestroyRuntimeDocument();

    Widget::Stop();
}

void Canvas::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    if (mRuntimeDocument != nullptr)
    {
        mRuntimeDocument->Tick();
    }

    // Debug dump after layout has settled
    if (mDebugDumpCountdown > 0)
    {
        mDebugDumpCountdown--;
    }
    else if (mDebugDumpCountdown == 0)
    {
        mDebugDumpCountdown = -1;

        LogDebug("===== UIDocument Widget Tree Debug Dump =====");
        LogDebug("Canvas host: '%s'  Anchor=%s  Size=(%.2f, %.2f)  Offset=(%.2f, %.2f)",
            GetName().c_str(), AnchorModeToString(GetAnchorMode()),
            GetSize().x, GetSize().y, GetOffset().x, GetOffset().y);

        Rect hostRect = GetRect();
        LogDebug("  Rect=(%.1f, %.1f, %.1f, %.1f)  Scale=(%.3f, %.3f)  AbsScale=(%.3f, %.3f)  World=%s",
            hostRect.mX, hostRect.mY, hostRect.mWidth, hostRect.mHeight,
            GetScale().x, GetScale().y, mAbsoluteScale.x, mAbsoluteScale.y,
            GetWorld() ? "yes" : "NO");

        glm::uvec4 vp = Renderer::Get()->GetViewport();
        LogDebug("  Viewport=(%u, %u, %u, %u)  GlobalUiScale=%.3f",
            vp.x, vp.y, vp.z, vp.w, Renderer::Get()->GetGlobalUiScale());

        Widget* parentW = GetParentWidget();
        if (parentW)
        {
            Rect pr = parentW->GetRect();
            LogDebug("  ParentWidget: '%s'  Rect=(%.1f, %.1f, %.1f, %.1f)",
                parentW->GetName().c_str(), pr.mX, pr.mY, pr.mWidth, pr.mHeight);
        }
        else
        {
            LogDebug("  ParentWidget: NONE (using viewport as parent rect)");
        }

        // Dump child tree
        for (uint32_t i = 0; i < GetNumChildren(); ++i)
        {
            Widget* child = GetChildWidget(i);
            if (child)
            {
                DebugDumpWidgetTree(child, 1);
            }
        }

        LogDebug("===== End UIDocument Debug Dump =====");
    }
}

void Canvas::EditorTick(float deltaTime)
{
    Widget::EditorTick(deltaTime);

    UIDocument* sourceDoc = static_cast<UIDocument*>(mUIDocumentRef.Get());

    // Tear down preview if source was cleared
    if (sourceDoc == nullptr && mRuntimeDocument != nullptr)
    {
        DestroyRuntimeDocument();
    }
}

static bool sRegenerate = false;

void Canvas::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);

    SCOPED_CATEGORY("Canvas");

    outProps.push_back(Property(DatumType::Asset, "UI Document", this, &mUIDocumentRef, 1, Canvas::HandlePropChange, int32_t(UIDocument::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Regenerate Preview", this, &sRegenerate));
}

bool Canvas::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Canvas* canvas = static_cast<Canvas*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "UI Document")
    {
        canvas->mUIDocumentRef = *(Asset**)newValue;
        success = true;

#if EDITOR
        // Clear preview if document was removed; user can click "Regenerate Preview" to rebuild
        if (!IsPlayingInEditor() && canvas->mUIDocumentRef.Get() == nullptr)
        {
            canvas->DestroyRuntimeDocument();
        }
#endif
    }

    return success;
}

void Canvas::DebugDumpWidgetTree(Widget* widget, int depth)
{
    if (!widget) return;

    // Build indent string
    char indent[64] = {};
    int indentLen = depth * 2;
    if (indentLen > 60) indentLen = 60;
    for (int i = 0; i < indentLen; ++i) indent[i] = ' ';
    indent[indentLen] = '\0';

    const char* typeName = widget->GetClassName();

    LogDebug("%s[%s] '%s'  Anchor=%s  Size=(%.2f, %.2f)  Offset=(%.2f, %.2f)",
        indent, typeName, widget->GetName().c_str(),
        AnchorModeToString(widget->GetAnchorMode()),
        widget->GetSize().x, widget->GetSize().y,
        widget->GetOffset().x, widget->GetOffset().y);

    Rect r = widget->GetRect();
    glm::vec2 absScale = widget->GetAbsoluteScale();
    LogDebug("%s  Rect=(%.1f, %.1f, %.1f, %.1f)  AbsScale=(%.3f, %.3f)  Vis=%s  World=%s  Children=%u",
        indent, r.mX, r.mY, r.mWidth, r.mHeight,
        absScale.x, absScale.y,
        widget->IsVisible() ? "yes" : "no",
        widget->GetWorld() ? "yes" : "NO",
        widget->GetNumChildren());

    for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
    {
        Widget* child = widget->GetChildWidget(i);
        if (child)
        {
            DebugDumpWidgetTree(child, depth + 1);
        }
    }
}

void Canvas::SetUIDocument(UIDocument* doc)
{
    mUIDocumentRef = doc;
}

UIDocument* Canvas::GetUIDocument() const
{
    return static_cast<UIDocument*>(mUIDocumentRef.Get());
}

void Canvas::DestroyRuntimeDocument()
{
    if (mRuntimeDocument != nullptr)
    {
        mRuntimeDocument->Unmount();
        mRuntimeDocument->Destroy();
        delete mRuntimeDocument;
        mRuntimeDocument = nullptr;
    }
    mEditorPreviewActive = false;
}

void Canvas::GenerateEditorPreview()
{
    DestroyRuntimeDocument();

    UIDocument* sourceDoc = static_cast<UIDocument*>(mUIDocumentRef.Get());
    if (sourceDoc != nullptr && !sourceDoc->GetXmlSource().empty())
    {
        mRuntimeDocument = new UIDocument();
        mRuntimeDocument->Create();
        mRuntimeDocument->SetXmlSource(sourceDoc->GetXmlSource());
        mRuntimeDocument->SetBasePath(sourceDoc->GetBasePath());
        mRuntimeDocument->SetSourceFilePath(sourceDoc->GetSourceFilePath());
        mRuntimeDocument->Instantiate();
        mRuntimeDocument->Mount(this);
        mEditorPreviewActive = true;
    }
}

void Canvas::DestroyEditorPreview()
{
    if (mEditorPreviewActive)
    {
        DestroyRuntimeDocument();
    }
}

#if EDITOR
bool Canvas::DrawCustomProperty(Property& prop)
{
    if (prop.mName == "Regenerate Preview")
    {
        if (ImGui::Button("Regenerate Preview"))
        {
            GenerateEditorPreview();
        }
        return true;
    }

    return false;
}
#endif
