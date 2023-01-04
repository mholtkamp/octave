#include "Assets/WidgetMap.h"
#include "World.h"
#include "Actor.h"
#include "Log.h"
#include "Engine.h"

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
            Widget* newWidget = Widget::CreateInstance(mWidgetDefs[i].mType);
            OCT_ASSERT(newWidget);

            std::vector<Property> dstProps;
            rootWidget->GatherProperties(dstProps);
            CopyPropertyValues(dstProps, mWidgetDefs[i].mProperties);

            if (i > 0)
            {
                Widget* parent = widgetList[mWidgetDefs[i].mParentIndex];
                parent->AddChild(newWidget);
            }

            widgetList.push_back(newWidget);
        }

        rootWidget = widgetList[0];
        OCT_ASSERT(rootWidget);
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
    if (widget != nullptr)
    {
        widgetList.push_back(widget);
        
        mWidgetDefs.push_back(WidgetDef());
        WidgetDef& widgetDef = mWidgetDefs.back();

        widgetDef.mType = widget->GetType();
        widgetDef.mParentIndex = FindWidgetIndex(widget->GetParent(), widgetList);

        // TODO: ONLY SAVE NON-DEFAULT properties
        widget->GatherProperties(widgetDef.mProperties, false);
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
