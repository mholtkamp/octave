#include "Nodes/Widgets/ComboBox.h"
#include "Nodes/Widgets/VerticalList.h"
#include "Renderer.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"

#include "InputDevices.h"

FORCE_LINK_DEF(ComboBox);
DEFINE_WIDGET(ComboBox, Selector);

void HandleComboBoxButtonPress(Button* button)
{
    ComboBoxButton* comboButton = (ComboBoxButton*) button;
    ComboBox* comboBox = comboButton->mComboBox;

    if (comboBox->GetListSelectDelay() > 0)
    {
        // We just opened up the list. Avoid automatically selecting the first entry.
        return;
    }

    VerticalList* list = comboBox->GetList();

    uint32_t i = 0;

    for (i = 0; i < list->GetNumListItems(); ++i)
    {
        if (list->GetListItem(i) == button)
        {
            comboBox->SetSelectionIndex(i);
            comboBox->ShowList(false);
            break;
        }
    }

    OCT_ASSERT(i < list->GetNumListItems());
}

ComboBox::ComboBox()
{
    mList = new VerticalList();
    mList->SetDisplayCount(8);
}

ComboBox::~ComboBox()
{
    if (mList->GetParent() == nullptr)
    {
        delete mList;
        mList = nullptr;
    }
}

void ComboBox::Update()
{
    Selector::Update();

    if (ShouldHandleInput() &&
        Renderer::Get()->GetModalWidget() == mList)
    {
        if (IsMouseButtonJustDown(MouseCode::MOUSE_LEFT) &&
            !mList->ContainsMouse())
        {
            // Mouse was clicked but the list doesn't contain the mouse.
            // So remove the list from the renderer widget list.
            ShowList(false);
        }
    }

    mListSelectDelay = glm::max(int32_t(0), mListSelectDelay - 1);
}

bool ComboBox::ShouldHandleInput()
{
    // If the modal widget is this list, then we still want to handle input so
    // we can cancel it if the user clicks outside the box.
    return Widget::ShouldHandleInput() || (Renderer::Get()->GetModalWidget() == mList);
}

void ComboBox::OnPressed()
{
    // Override Selector's OnPressed behavior
    Button::OnPressed();

    if (mList != Renderer::Get()->GetModalWidget())
    {
        ShowList(true);
        mList->SetPosition(mRect.mX, mRect.mY);
    }
}

void ComboBox::AddSelection(const std::string& selection)
{
    Selector::AddSelection(selection);

    float buttonWidth = this->GetWidth();
    float buttonHeight = this->GetHeight();

    // Create new button for our dropdown list. Make sure its position in the list
    // matches its selection string index in Selector.
    ComboBoxButton* button = new ComboBoxButton();
    button->mComboBox = this;
    button->SetTextString(selection);
    button->GetText()->SetTextSize(mTextSize);
    button->SetDimensions(buttonWidth, buttonHeight);
    button->SetPressedHandler(HandleComboBoxButtonPress);
    button->GetQuad()->SetColor({0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f},
                                {0.0f, 0.0f, 0.8f, 1.0f}, {0.0f, 0.0f, 0.8f, 1.0f});
    mList->AddListItem(button);
}

void ComboBox::RemoveSelection(const std::string& selection)
{
    for (uint32_t i = 0; i < mSelectionStrings.size(); ++i)
    {
        if (mSelectionStrings[i] == selection)
        {
            Widget* removedWidget = mList->RemoveListItem(i);
            delete removedWidget;
            removedWidget = nullptr;

            break;
        }
    }

    // For safety, call the parent RemoveSelection to remove the string from mSelectionStrings.
    // We could have handled it in the above loop.
    Selector::RemoveSelection(selection);
}

void ComboBox::RemoveAllSelections()
{
    Selector::RemoveAllSelections();

    while (mList->GetNumListItems() > 0)
    {
        Widget* removedWidget = mList->RemoveListItem(uint32_t(0));
        delete removedWidget;
    }
}

VerticalList* ComboBox::GetList()
{
    return mList;
}

void ComboBox::ShowList(bool show)
{
    Renderer* renderer = Renderer::Get();
    mList->SetVisible(show);

    if (show)
    {
        renderer->SetModalWidget(mList);
        mListSelectDelay = 2;
    }
    else
    {
        if (renderer->GetModalWidget() == mList)
        {
            renderer->SetModalWidget(nullptr);
        }
    }
}

int32_t ComboBox::GetListSelectDelay() const
{
    return mListSelectDelay;
}

void ComboBox::SetTextSize(float size)
{
    mTextSize = size;

    mText->SetTextSize(size);

    for (uint32_t i = 0; i < mList->GetNumListItems(); ++i)
    {
        Button* button = (Button*)mList->GetListItem(i);
        button->GetText()->SetTextSize(mTextSize);
    }
}

float ComboBox::GetTextSize() const
{
    return mTextSize;
}
