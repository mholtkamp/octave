#include "Widgets/Selector.h"
#include "Widgets/Text.h"
#include "InputDevices.h"
#include "ScriptEvent.h"

#include "Components/ScriptComponent.h"

FORCE_LINK_DEF(Selector);
DEFINE_FACTORY(Selector, Widget);

Selector::Selector() :
    mSelectionIndex(0)
{

}

Selector::~Selector()
{

}

void Selector::Update()
{
    Button::Update();

    // mText should always display the selected string.
    if (IsDirty())
    {
        mText->SetText(GetSelectionString());
    }
}

void Selector::OnPressed()
{
    if (IsMouseButtonJustUp(MOUSE_LEFT))
    {
        Increment();
    }
    else
    {
        Decrement();
    }
    
    Button::OnPressed();
}

void Selector::AddSelection(const std::string & selection)
{
    mSelectionStrings.push_back(selection);
    MarkDirty();
}

void Selector::RemoveSelection(const std::string & selection)
{
    for (int32_t i = int32_t(mSelectionStrings.size()) - 1; i >= 0; ++i)
    {
        if (mSelectionStrings[i] == selection)
        {
            mSelectionStrings.erase(mSelectionStrings.begin() + i);
            MarkDirty();
            break;
        }
    }
}

void Selector::RemoveAllSelections()
{
    mSelectionStrings.clear();
    MarkDirty();
}

uint32_t Selector::GetNumSelections() const
{
    return (uint32_t)mSelectionStrings.size();
}

void Selector::SetSelectionByString(const std::string & string)
{
    for (uint32_t i = 0; i < mSelectionStrings.size(); ++i)
    {
        if (mSelectionStrings[i] == string)
        {
            SetSelectionIndex(i);
            break;
        }
    }
}

void Selector::SetSelectionIndex(int32_t index)
{
    if (mSelectionIndex != index)
    {
        mSelectionIndex = index;

        if (mSelectionIndex >= int32_t(mSelectionStrings.size()))
        {
            mSelectionIndex = 0;
        }
        else if (mSelectionIndex < 0)
        {
            mSelectionIndex = int32_t(mSelectionStrings.size()) - 1;
        }

        if (mSelectionStrings.empty())
        {
            mSelectionIndex = 0;
        }
    
        if (mSelectionChangeHandler.mFuncPointer != nullptr)
        {
            mSelectionChangeHandler.mFuncPointer(this);
        }
        if (mSelectionChangeHandler.mScriptTableName != "")
        {
            ScriptEvent::WidgetState(
                mSelectionChangeHandler.mScriptTableName,
                mSelectionChangeHandler.mScriptFuncName,
                this);
        }

        MarkDirty();
    }
}

void Selector::Increment()
{
    SetSelectionIndex(GetSelectionIndex() + 1);
}

void Selector::Decrement()
{
    SetSelectionIndex(GetSelectionIndex() - 1);
}

const std::string Selector::GetSelectionString() const
{
    std::string retString;

    if (!mSelectionStrings.empty() &&
        mSelectionIndex >= 0 &&
        mSelectionIndex < int32_t(mSelectionStrings.size()))
    {
        retString = mSelectionStrings[mSelectionIndex];
    }

    return retString;
}

int32_t Selector::GetSelectionIndex() const
{
    return mSelectionIndex;
}

void Selector::SetSelectionChangeHandler(SelectorHandlerFP handler)
{
    mSelectionChangeHandler.mFuncPointer = handler;
}

void Selector::SetScriptSelectionChangeHandler(const char* tableName, const char* funcName)
{
    mSelectionChangeHandler.mScriptTableName = tableName;
    mSelectionChangeHandler.mScriptFuncName = funcName;
}
