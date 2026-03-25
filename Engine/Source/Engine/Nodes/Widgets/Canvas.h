#pragma once

#include "Nodes/Widgets/Widget.h"
#include "AssetRef.h"

class UIDocument;

class Canvas : public Widget
{
public:

    DECLARE_NODE(Canvas, Widget);

    Canvas();
    virtual ~Canvas();

    virtual void Start() override;
    virtual void Stop() override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

#if EDITOR
    virtual bool DrawCustomProperty(Property& prop) override;
#endif

    void SetUIDocument(UIDocument* doc);
    UIDocument* GetUIDocument() const;
    UIDocument* GetRuntimeDocument() const { return mRuntimeDocument; }

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

private:

    void DestroyRuntimeDocument();
    void GenerateEditorPreview();
    void DestroyEditorPreview();
    static void DebugDumpWidgetTree(Widget* widget, int depth);

    AssetRef mUIDocumentRef;
    UIDocument* mRuntimeDocument = nullptr;
    bool mEditorPreviewActive = false;
    int32_t mDebugDumpCountdown = -1;
};
