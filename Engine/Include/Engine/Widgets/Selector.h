#pragma once

#include "Widgets/Button.h"
#include <string>
#include <vector>

#include "ScriptFunc.h"

typedef void(*SelectorHandlerFP)(class Selector* selector);

class Selector : public Button
{
public:

    DECLARE_WIDGET(Selector, Button);

    Selector();
    ~Selector();

    virtual void Update() override;

    virtual void OnPressed() override;

    virtual void AddSelection(const std::string& selection);
    virtual void RemoveSelection(const std::string& selection);
    virtual void RemoveAllSelections();

    uint32_t GetNumSelections() const;

    void SetSelectionByString(const std::string& string);
    void SetSelectionIndex(int32_t index);
    void Increment();
    void Decrement();

    const std::string GetSelectionString() const;
    int32_t GetSelectionIndex() const;

    void SetSelectionChangeHandler(SelectorHandlerFP handler);
    void SetScriptSelectionChangeHandler(const char* tableName, const char* funcName);

protected:

    ScriptableFP<SelectorHandlerFP> mSelectionChangeHandler;
    std::vector<std::string> mSelectionStrings;
    int32_t mSelectionIndex;
};
