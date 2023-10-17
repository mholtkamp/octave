#pragma once

#include "Asset.h"
#include "Property.h"
#include "Factory.h"
#include "Widgets/Widget.h"

struct WidgetDef
{
    TypeId mType = INVALID_TYPE_ID;
    int32_t mParentIndex = -1;
    int32_t mChildSlot = -1;
    int32_t mNativeChildSlot = -1;
    WidgetMapRef mWidgetMap;
    std::vector<Property> mProperties;
    bool mExposeVariable = false;
};

class WidgetMap : public Asset
{
public:

    DECLARE_ASSET(WidgetMap, Asset);

    WidgetMap();
    ~WidgetMap();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void Create(Widget* srcWidget);
    Widget* Instantiate();

    //const Property* GetProperty(const std::string& widgetName, const std::string& propName);

protected:

    //WidgetDef* FindWidgetDef(const std::string& name, int32_t* outIndex = nullptr);
    
    void AddWidgetDef(Widget* widget, std::vector<Widget*>& widgetList);
    int32_t FindWidgetIndex(Widget* widget, const std::vector<Widget*>& widgetList);

    std::vector<WidgetDef> mWidgetDefs;
};
