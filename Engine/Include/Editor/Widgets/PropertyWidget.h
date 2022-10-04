#pragma once

#include "Widgets/Widget.h"
#include "Widgets/Canvas.h"
#include "Property.h"

class Text;
class TextField;
class CheckBox;
class Button;
class Selector;
class ComboBox;
class PropertyWidget;

PropertyWidget* CreatePropWidget(const Property& prop, bool arrayElement = false);

class PropertyWidget : public Canvas
{
public:

    PropertyWidget();
    virtual void Update() override;
    virtual void Write() = 0;
    virtual float GetHeight();
    virtual void SetProperty(const Property& prop, uint32_t index);
    const Property& GetProperty() const;
    Property& GetProperty();
    void TabTextField();
    virtual bool IsArray() const { return false; }
    void MarkArrayElement() { mArrayElement = true; }

protected:

    Property mProperty;
    PropertyOwnerType mOwnerType;
    uint32_t mIndex;
    bool mArrayElement;

    Text* mNameText;
};

class PropertyArrayWidget : public PropertyWidget
{
public:
    PropertyArrayWidget();
    virtual void Update() override;
    virtual float GetHeight();
    virtual void SetProperty(const Property& prop, uint32_t index);
    virtual void Write() override { };
    virtual bool IsArray() const override { return true; }
    std::vector<PropertyWidget*>& GetElementWidgets() { return mElementWidgets; }

protected:

    Button* mPopButton = nullptr;
    Button* mPushButton = nullptr;
    std::vector<PropertyWidget*> mElementWidgets;
};

class FloatProp : public PropertyWidget
{
public:
    FloatProp();
    virtual void Update() override;
    virtual void Write() override;
protected:
    TextField* mTextField;
};

class IntegerProp : public PropertyWidget
{
public:
    IntegerProp();
    virtual void Update() override;
    virtual void Write() override;
protected:
    TextField* mTextField;
};

class VectorProp : public PropertyWidget
{
public:
    VectorProp();
    virtual void Update() override;
    virtual void Write() override;
    virtual float GetHeight() override;
protected:
    Text* mTextX;
    Text* mTextY;
    Text* mTextZ;
    TextField* mTextFieldX;
    TextField* mTextFieldY;
    TextField* mTextFieldZ;
};

class ColorProp : public VectorProp
{
public:
    ColorProp();
    virtual void Update() override;
    virtual void Write() override;
    virtual float GetHeight() override;
protected:
    Text* mTextW;
    TextField* mTextFieldW;
};

class StringProp : public PropertyWidget
{
public:
    StringProp();
    virtual void Update() override;
    virtual void Write() override;
protected:
    TextField* mTextField;
};

class BoolProp : public PropertyWidget
{
public:
    BoolProp();
    virtual void Update() override;
    virtual void Write() override;
    virtual float GetHeight() override;
protected:
    CheckBox* mCheckBox;
};

class AssetProp : public PropertyWidget
{
public:
    AssetProp();
    virtual void Update() override;
    virtual void Write() override;
    void AssignAsset(Asset* asset);
    void AssignAsset(const std::string& name);

protected:

    Button* mAssignAssetButton;
    TextField* mTextField;
};

class EnumProp : public PropertyWidget
{
public:
    EnumProp();
    virtual void Update() override;
    virtual void Write() override;
    virtual void SetProperty(const Property& prop, uint32_t index) override;
protected:
    ComboBox* mSelector;
};

class ByteProp : public PropertyWidget
{
public:
    ByteProp();
    virtual void Update() override;
    virtual void Write() override;
protected:
    TextField* mTextField;
};

class ByteFlagProp : public PropertyWidget
{
public:
    ByteFlagProp();
    virtual void Update() override;
    virtual void Write() override;
protected:
    CheckBox* mCheckBoxes[8] = {};
};

class Vector2DProp : public PropertyWidget
{
public:
    Vector2DProp();
    virtual void Update() override;
    virtual void Write() override;
    virtual float GetHeight() override;
protected:
    Text* mTextX;
    Text* mTextY;
    TextField* mTextFieldX;
    TextField* mTextFieldY;
};
