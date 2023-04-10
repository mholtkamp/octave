#include "Assets/WidgetMap.h"
#include "World.h"
#include "Actor.h"
#include "Log.h"
#include "Engine.h"
#include "Widgets/ScriptWidget.h"

FORCE_LINK_DEF(WidgetMap);
DEFINE_ASSET(WidgetMap);

WidgetMap::WidgetMap()
{
    mType = WidgetMap::GetStaticType();
}

WidgetMap::~WidgetMap()
{

}

void WidgetMap::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    uint32_t numWidgetDefs = stream.ReadUint32();
    OCT_ASSERT(numWidgetDefs < 1000); // Something reasonable?
    mWidgetDefs.resize(numWidgetDefs);

    for (uint32_t i = 0; i < numWidgetDefs; ++i)
    {
        WidgetDef& def = mWidgetDefs[i];

        def.mType = (TypeId)stream.ReadUint32();
        def.mParentIndex = stream.ReadInt32();
        def.mChildSlot = stream.ReadInt32();
        def.mNativeChildSlot = stream.ReadInt32();
        
        stream.ReadAsset(def.mWidgetMap);
        def.mExposeVariable = stream.ReadBool();

        uint32_t numProps = stream.ReadUint32();
        def.mProperties.resize(numProps);
        for (uint32_t p = 0; p < numProps; ++p)
        {
            def.mProperties[p].ReadStream(stream, false);
        }
    }
}

void WidgetMap::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mWidgetDefs.size());

    for (uint32_t i = 0; i < mWidgetDefs.size(); ++i)
    {
        WidgetDef& def = mWidgetDefs[i];
        stream.WriteUint32((uint32_t)def.mType);
        stream.WriteInt32(def.mParentIndex);
        stream.WriteInt32(def.mChildSlot);
        stream.WriteInt32(def.mNativeChildSlot);

        stream.WriteAsset(def.mWidgetMap);
        stream.WriteBool(def.mExposeVariable);

        stream.WriteUint32((uint32_t)def.mProperties.size());
        for (uint32_t p = 0; p < def.mProperties.size(); ++p)
        {
            def.mProperties[p].WriteStream(stream);
        }
    }
}

void WidgetMap::Create()
{
    Asset::Create();
}

void WidgetMap::Destroy()
{
    Asset::Destroy();
}

void WidgetMap::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
}

glm::vec4 WidgetMap::GetTypeColor()
{
    return glm::vec4(0.7f, 0.9f, 0.2f, 1.0f);
}

const char* WidgetMap::GetTypeName()
{
    return "WidgetMap";
}

void WidgetMap::Create(Widget* srcWidget)
{
    mWidgetDefs.clear();

    if (srcWidget == nullptr)
        return;

    std::vector<Widget*> widgetList;
    AddWidgetDef(srcWidget, widgetList);
}

Widget* WidgetMap::Instantiate()
{
    Widget* rootWidget = nullptr;

    if (mWidgetDefs.size() > 0)
    {
        std::vector<Widget*> widgetList;

        for (uint32_t i = 0; i < mWidgetDefs.size(); ++i)
        {
            Widget* newWidget = nullptr;

            if (mWidgetDefs[i].mNativeChildSlot == -1)
            {
                if (mWidgetDefs[i].mWidgetMap != nullptr)
                {
                    WidgetMap* widgetMap = mWidgetDefs[i].mWidgetMap.Get<WidgetMap>();
                    newWidget = widgetMap->Instantiate();

#if EDITOR
                    newWidget->SetWidgetMap(widgetMap);
                    newWidget->SetExposeVariable(mWidgetDefs[i].mExposeVariable);
#endif
                }
                else
                {
                    newWidget = CreateWidget(mWidgetDefs[i].mType, false);
                }
            }
            else
            {
                // For things like Buttons where the constructor spawns child widgets.
                Widget* parent = widgetList[mWidgetDefs[i].mParentIndex];
                
                // Root widget should always be created.
                OCT_ASSERT(parent != nullptr);

                // Find the native child we are going to fill out.
                for (uint32_t w = 0; w < parent->GetNumChildren(); ++w)
                {
                    Widget* child = parent->GetChild(int32_t(w));
                    if (child->GetNativeChildSlot() == mWidgetDefs[i].mNativeChildSlot)
                    {
                        newWidget = child;
                        break;
                    }
                }
            }
            OCT_ASSERT(newWidget);

            std::vector<Property> dstProps;
            newWidget->GatherProperties(dstProps);
            CopyPropertyValues(dstProps, mWidgetDefs[i].mProperties);

            if (i > 0)
            {
                Widget* parent = widgetList[mWidgetDefs[i].mParentIndex];

                parent->AddChild(newWidget, mWidgetDefs[i].mChildSlot);

                if (mWidgetDefs[i].mExposeVariable &&
                    widgetList[0]->As<ScriptWidget>())
                {
                    widgetList[0]->As<ScriptWidget>()->SetField(newWidget->GetName().c_str(), newWidget);
                }
            }
            else
            {
                ScriptWidget* scriptWidget = newWidget->As<ScriptWidget>();
                if (scriptWidget)
                {
                    // Start the script so that any children marked as "Expose Variable"
                    // can have their variable set on the script table.
                    scriptWidget->StartScript();
                }
            }

            widgetList.push_back(newWidget);
        }

        rootWidget = widgetList[0];
        OCT_ASSERT(rootWidget);

        for (uint32_t i = 0; i < widgetList.size(); ++i)
        {
            if (!widgetList[i]->HasStarted())
            {
                widgetList[i]->Start();
            }
        }
    }

    return rootWidget;
}

//const Property* WidgetMap::GetProperty(const std::string& widgetName, const std::string& propName)
//{
//
//}
//
//WidgetDef* WidgetMap::FindWidgetDef(const std::string& name, int32_t* outIndex)
//{
//
//}

void WidgetMap::AddWidgetDef(Widget* widget, std::vector<Widget*>& widgetList)
{
    OCT_ASSERT(widget != nullptr);

#if EDITOR
    if (widget && 
        widget->GetWidgetMap() == this)
    {
        LogWarning("Recursive WidgetMap chain found.");
        return;
    }
#endif

    if (widget != nullptr)
    {
        widgetList.push_back(widget);

        Widget* parent = widget->GetParent();

        mWidgetDefs.push_back(WidgetDef());
        WidgetDef& widgetDef = mWidgetDefs.back();

        widgetDef.mType = widget->GetType();
        widgetDef.mParentIndex = FindWidgetIndex(parent, widgetList);

        WidgetMap* widgetMap = nullptr;
#if EDITOR
        widgetDef.mExposeVariable = widget->ShouldExposeVariable();
        widgetMap = widget->GetWidgetMap();
        widgetDef.mNativeChildSlot = widget->GetNativeChildSlot();
#endif
        widgetDef.mWidgetMap = widgetMap;

        // Find child slot
        if (parent != nullptr)
        {
            for (uint32_t i = 0; i < parent->GetNumChildren(); ++i)
            {
                if (parent->GetChild(int32_t(i)) == widget)
                {
                    widgetDef.mChildSlot = (int32_t)i;
                    break;
                }
            }

            OCT_ASSERT(widgetDef.mChildSlot != -1);
        }

        std::vector<Property> extProps;
        widget->GatherProperties(extProps);

        {
            Widget* defaultWidget = CreateWidget(widget->GetType(), false);
            std::vector<Property> defaultProps;
            defaultWidget->GatherProperties(defaultProps);

            widgetDef.mProperties.reserve(extProps.size());
            for (uint32_t i = 0; i < extProps.size(); ++i)
            {
                Property* defaultProp = FindProperty(defaultProps, extProps[i].mName);

                if (defaultProp == nullptr ||
                    extProps[i].mType == DatumType::Asset ||
                    extProps[i] != *defaultProp)
                {
                    widgetDef.mProperties.push_back(Property());
                    widgetDef.mProperties.back().DeepCopy(extProps[i], true);
                }
            }

            delete defaultWidget;
            defaultWidget = nullptr;
        }

        // Recursively add children. Do not add children of widgets spawned via maps.
        if (widgetMap == nullptr)
        {
            for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
            {
                AddWidgetDef(widget->GetChild(i), widgetList);
            }
        }
    }
}

int32_t WidgetMap::FindWidgetIndex(Widget* widget, const std::vector<Widget*>& widgetList)
{
    int32_t index = -1;

    for (uint32_t i = 0; i < widgetList.size(); ++i)
    {
        if (widgetList[i] == widget)
        {
            index = (int32_t)i;
            break;
        }
    }

    return index;
}
