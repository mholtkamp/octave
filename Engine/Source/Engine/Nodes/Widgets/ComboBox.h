#pragma once

#include "Nodes/Widgets/Selector.h"

class VerticalList;

class ComboBoxButton : public Button
{
public:
    class ComboBox* mComboBox = nullptr;
};

class ComboBox : public Selector
{
public:

    DECLARE_WIDGET(ComboBox, Selector);

    ComboBox();
    ~ComboBox();

    virtual void Update() override;
    virtual bool ShouldHandleInput() override;

    virtual void OnPressed() override;

    virtual void AddSelection(const std::string& selection) override;
    virtual void RemoveSelection(const std::string& selection) override;
    virtual void RemoveAllSelections() override;

    VerticalList* GetList();
    void ShowList(bool show);
    int32_t GetListSelectDelay() const;

    void SetTextSize(float size);
    float GetTextSize() const;

protected:

    VerticalList* mList = nullptr;
    int32_t mListSelectDelay = 2;
    float mTextSize = 16.0f;
};