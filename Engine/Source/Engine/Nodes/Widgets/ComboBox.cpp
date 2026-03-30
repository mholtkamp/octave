
#include "Nodes/Widgets/ComboBox.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Canvas.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "Assets/Texture.h"

#include <sstream>

FORCE_LINK_DEF(ComboBox);
DEFINE_NODE(ComboBox, Widget);

bool ComboBox::sHandleMouseInput = true;
bool ComboBox::sHandleKeyboardInput = true;
bool ComboBox::sHandleGamepadInput = true;

bool ComboBox::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    ComboBox* comboBox = static_cast<ComboBox*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Options")
    {
        // Parse newline-separated options string into vector
        const std::string& optionsStr = *reinterpret_cast<const std::string*>(newValue);
        comboBox->mOptions.clear();

        std::istringstream stream(optionsStr);
        std::string line;
        while (std::getline(stream, line))
        {
            if (!line.empty())
            {
                comboBox->mOptions.push_back(line);
            }
        }

        // Clamp selected index
        if (comboBox->mSelectedIndex >= (int32_t)comboBox->mOptions.size())
        {
            comboBox->mSelectedIndex = comboBox->mOptions.empty() ? -1 : 0;
        }
    }

    comboBox->MarkDirty();

    return success;
}

void ComboBox::Create()
{
    Super::Create();

    SetName("ComboBox");

    mBackground = CreateChild<Quad>("Background");
    mText = CreateChild<Text>("Text");
    mArrow = CreateChild<Quad>("Arrow");

    mBackground->SetTransient(true);
    mText->SetTransient(true);
    mArrow->SetTransient(true);

#if EDITOR
    mBackground->mHiddenInTree = true;
    mText->mHiddenInTree = true;
    mArrow->mHiddenInTree = true;
#endif

    // Setup background - FullStretch
    mBackground->SetAnchorMode(AnchorMode::FullStretch);
    mBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mBackground->SetColor(mBackgroundColor);

    // Setup text - FullStretch with right margin for arrow
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetHorizontalJustification(Justification::Left);
    mText->SetVerticalJustification(Justification::Center);
    mText->SetColor(mTextColor);

    // Setup arrow indicator - TopLeft (positioned on right)
    mArrow->SetAnchorMode(AnchorMode::TopLeft);
    mArrow->SetColor(mArrowColor);

    // Default dimensions
    SetDimensions(150, 30);

    MarkDirty();
}

void ComboBox::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("ComboBox");

        props.push_back(Property(DatumType::String, "Options", this, &mOptionsString, 1, HandlePropChange));
        props.push_back(Property(DatumType::Integer, "Selected Index", this, &mSelectedIndex, 1, HandlePropChange));
        props.push_back(Property(DatumType::Integer, "Max Visible Items", this, &mMaxVisibleItems, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Item Height", this, &mItemHeight, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Arrow Width", this, &mArrowWidth, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Visual");

        props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Dropdown Color", this, &mDropdownColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Hovered Color", this, &mHoveredColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Arrow Color", this, &mArrowColor, 1, HandlePropChange));
    }
}

void ComboBox::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (!ShouldHandleInput())
    {
        return;
    }

    if (IsDirty())
    {
        UpdateRect();
    }

    bool containsMouse = ContainsMouse();

    // ===== MOUSE INPUT =====
    if (sHandleMouseInput)
    {
        // Handle click to toggle dropdown
        if (IsPointerJustDown(0))
        {
            if (containsMouse && !mIsOpen)
            {
                Open();
            }
            else if (mIsOpen)
            {
                // Check if clicked on dropdown item
                bool clickedOnDropdown = false;
                if (mDropdownCanvas != nullptr)
                {
                    int32_t mouseX, mouseY;
                    GetMousePosition(mouseX, mouseY);

                    for (int32_t i = 0; i < (int32_t)mDropdownItems.size(); i++)
                    {
                        Rect itemRect = mDropdownItems[i]->GetRect();
                        if (itemRect.ContainsPoint((float)mouseX, (float)mouseY))
                        {
                            SetSelectedIndex(mScrollOffset + i);
                            clickedOnDropdown = true;
                            break;
                        }
                    }
                }

                Close();
            }
        }

        // Update hover state for dropdown items (mouse tracking)
        if (mIsOpen && mDropdownCanvas != nullptr)
        {
            int32_t mouseX, mouseY;
            GetMousePosition(mouseX, mouseY);

            int32_t newHoveredIndex = -1;
            for (int32_t i = 0; i < (int32_t)mDropdownItems.size(); i++)
            {
                Rect itemRect = mDropdownItems[i]->GetRect();
                if (itemRect.ContainsPoint((float)mouseX, (float)mouseY))
                {
                    newHoveredIndex = mScrollOffset + i;
                    break;
                }
            }

            if (newHoveredIndex != mHoveredIndex && newHoveredIndex >= 0)
            {
                mHoveredIndex = newHoveredIndex;
                UpdateDropdown();
            }
        }

        // ===== SCROLL WHEEL INPUT =====
        int32_t scrollDelta = GetScrollWheelDelta();
        if (scrollDelta != 0)
        {
            if (mIsOpen)
            {
                // Scroll moves hovered index in dropdown
                CycleHoveredIndex(scrollDelta > 0 ? -1 : 1);
            }
            else if (containsMouse)
            {
                // Scroll cycles selection without opening
                CycleSelectedIndex(scrollDelta > 0 ? -1 : 1);
            }
        }
    }

    // ===== KEYBOARD INPUT =====
    if (sHandleKeyboardInput && (containsMouse || mIsOpen))
    {
        if (mIsOpen)
        {
            // Dropdown is open - navigate and select
            if (IsKeyJustDownRepeat(OCTAVE_KEY_UP))
            {
                CycleHoveredIndex(-1);
            }

            if (IsKeyJustDownRepeat(OCTAVE_KEY_DOWN))
            {
                CycleHoveredIndex(1);
            }

            if (IsKeyJustDown(OCTAVE_KEY_ENTER) || IsKeyJustDown(OCTAVE_KEY_SPACE))
            {
                if (mHoveredIndex >= 0)
                {
                    SetSelectedIndex(mHoveredIndex);
                }
                Close();
            }

            if (IsKeyJustDown(OCTAVE_KEY_ESCAPE))
            {
                Close();
            }
        }
        else if (containsMouse)
        {
            // Dropdown closed - open or cycle selection
            if (IsKeyJustDown(OCTAVE_KEY_ENTER) || IsKeyJustDown(OCTAVE_KEY_SPACE))
            {
                Open();
            }

            if (IsKeyJustDownRepeat(OCTAVE_KEY_UP))
            {
                CycleSelectedIndex(-1);
            }

            if (IsKeyJustDownRepeat(OCTAVE_KEY_DOWN))
            {
                CycleSelectedIndex(1);
            }
        }
    }

    // ===== GAMEPAD INPUT =====
    if (sHandleGamepadInput && (containsMouse || mIsOpen))
    {
        if (mIsOpen)
        {
            // Dropdown is open - navigate and select
            if (IsGamepadButtonJustDown(GAMEPAD_UP, 0) ||
                IsGamepadButtonJustDown(GAMEPAD_L_UP, 0))
            {
                CycleHoveredIndex(-1);
            }

            if (IsGamepadButtonJustDown(GAMEPAD_DOWN, 0) ||
                IsGamepadButtonJustDown(GAMEPAD_L_DOWN, 0))
            {
                CycleHoveredIndex(1);
            }

            if (IsGamepadButtonJustDown(GAMEPAD_A, 0))
            {
                if (mHoveredIndex >= 0)
                {
                    SetSelectedIndex(mHoveredIndex);
                }
                Close();
            }

            if (IsGamepadButtonJustDown(GAMEPAD_B, 0))
            {
                Close();
            }
        }
        else if (containsMouse)
        {
            // Dropdown closed - open or cycle selection
            if (IsGamepadButtonJustDown(GAMEPAD_A, 0))
            {
                Open();
            }

            if (IsGamepadButtonJustDown(GAMEPAD_UP, 0) ||
                IsGamepadButtonJustDown(GAMEPAD_L_UP, 0))
            {
                CycleSelectedIndex(-1);
            }

            if (IsGamepadButtonJustDown(GAMEPAD_DOWN, 0) ||
                IsGamepadButtonJustDown(GAMEPAD_L_DOWN, 0))
            {
                CycleSelectedIndex(1);
            }
        }
    }
}

void ComboBox::EditorTick(float deltaTime)
{
    Super::EditorTick(deltaTime);

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void ComboBox::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void ComboBox::UpdateAppearance()
{
    if (mBackground == nullptr || mText == nullptr || mArrow == nullptr)
    {
        return;
    }

    float width = GetWidth();
    float height = GetHeight();

    // Update background
    mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    mBackground->SetColor(mBackgroundColor);

    // Update text margins and content
    mText->SetMargins(4.0f, 0.0f, mArrowWidth, 0.0f);
    mText->SetColor(mTextColor);

    if (mSelectedIndex >= 0 && mSelectedIndex < (int32_t)mOptions.size())
    {
        mText->SetText(mOptions[mSelectedIndex]);
    }
    else
    {
        mText->SetText("");
    }

    // Position arrow on right, centered vertically
    float arrowSize = glm::min(mArrowWidth, height) * 0.5f;
    float arrowX = width - mArrowWidth + (mArrowWidth - arrowSize) * 0.5f;
    float arrowY = (height - arrowSize) * 0.5f;
    mArrow->SetPosition(arrowX, arrowY);
    mArrow->SetDimensions(arrowSize, arrowSize);
    mArrow->SetColor(mArrowColor);

    // Update dropdown if open
    if (mIsOpen)
    {
        UpdateDropdown();
    }
}

void ComboBox::UpdateDropdown()
{
    if (mDropdownCanvas == nullptr)
    {
        return;
    }

    // Update position in case ComboBox moved (dropdown is attached to root)
    Rect myRect = GetRect();
    mDropdownCanvas->SetPosition(myRect.mX, myRect.mY + myRect.mHeight);

    // Update item text and colors based on scroll offset
    for (int32_t i = 0; i < (int32_t)mDropdownItems.size(); i++)
    {
        int32_t optionIndex = mScrollOffset + i;

        // Update text to show correct option
        if (optionIndex < (int32_t)mOptions.size() && i < (int32_t)mDropdownTexts.size())
        {
            mDropdownTexts[i]->SetText(mOptions[optionIndex]);
        }

        // Update colors based on actual option index
        if (optionIndex == mHoveredIndex)
        {
            mDropdownItems[i]->SetColor(mHoveredColor);
        }
        else if (optionIndex == mSelectedIndex)
        {
            mDropdownItems[i]->SetColor(glm::vec4(mHoveredColor.r * 0.8f, mHoveredColor.g * 0.8f, mHoveredColor.b * 0.8f, 1.0f));
        }
        else
        {
            mDropdownItems[i]->SetColor(mDropdownColor);
        }
    }
}

void ComboBox::CreateDropdownItems()
{
    if (mOptions.empty())
    {
        return;
    }

    // Find root widget to attach dropdown (renders on top of siblings)
    Widget* rootWidget = this;
    while (rootWidget->GetParentWidget() != nullptr)
    {
        rootWidget = rootWidget->GetParentWidget();
    }

    // Create dropdown canvas at root level for proper z-order
    mDropdownCanvas = rootWidget->CreateChild<Canvas>("DropdownCanvas");
    mDropdownCanvas->SetTransient(true);
    mDropdownCanvas->SetAnchorMode(AnchorMode::TopLeft);

    float width = GetWidth();
    int32_t visibleItems = glm::min((int32_t)mOptions.size(), mMaxVisibleItems);
    float dropdownHeight = visibleItems * mItemHeight;

    // Position using absolute screen coordinates (below the ComboBox)
    Rect myRect = GetRect();
    mDropdownCanvas->SetPosition(myRect.mX, myRect.mY + myRect.mHeight);
    mDropdownCanvas->SetDimensions(width, dropdownHeight);

    // Create items (visual slots, text set in UpdateDropdown)
    for (int32_t i = 0; i < visibleItems; i++)
    {
        Quad* itemBg = mDropdownCanvas->CreateChild<Quad>(("Item" + std::to_string(i)).c_str());
        itemBg->SetTransient(true);
        itemBg->SetAnchorMode(AnchorMode::TopLeft);
        itemBg->SetPosition(0.0f, i * mItemHeight);
        itemBg->SetDimensions(width, mItemHeight);
        itemBg->SetColor(mDropdownColor);
        mDropdownItems.push_back(itemBg);

        Text* itemText = mDropdownCanvas->CreateChild<Text>(("ItemText" + std::to_string(i)).c_str());
        itemText->SetTransient(true);
        itemText->SetAnchorMode(AnchorMode::TopLeft);
        itemText->SetPosition(4.0f, i * mItemHeight);
        itemText->SetDimensions(width - 8.0f, mItemHeight);
        itemText->SetHorizontalJustification(Justification::Left);
        itemText->SetVerticalJustification(Justification::Center);
        itemText->SetColor(mTextColor);
        mDropdownTexts.push_back(itemText);
    }

    // Initialize text and colors based on scroll offset
    UpdateDropdown();
}

void ComboBox::DestroyDropdownItems()
{
    mDropdownItems.clear();
    mDropdownTexts.clear();

    if (mDropdownCanvas != nullptr)
    {
        mDropdownCanvas->Destroy();
        mDropdownCanvas = nullptr;
    }

    mHoveredIndex = -1;
}

void ComboBox::SyncOptionsString()
{
    mOptionsString.clear();
    for (size_t i = 0; i < mOptions.size(); i++)
    {
        if (i > 0)
        {
            mOptionsString += "\n";
        }
        mOptionsString += mOptions[i];
    }
}

void ComboBox::AddOption(const std::string& option)
{
    mOptions.push_back(option);
    SyncOptionsString();
    MarkDirty();
}

void ComboBox::RemoveOption(int32_t index)
{
    if (index >= 0 && index < (int32_t)mOptions.size())
    {
        mOptions.erase(mOptions.begin() + index);

        if (mSelectedIndex >= (int32_t)mOptions.size())
        {
            mSelectedIndex = (int32_t)mOptions.size() - 1;
        }

        SyncOptionsString();
        MarkDirty();
    }
}

void ComboBox::ClearOptions()
{
    mOptions.clear();
    mOptionsString.clear();
    mSelectedIndex = -1;
    MarkDirty();
}

void ComboBox::SetOptions(const std::vector<std::string>& options)
{
    mOptions = options;
    if (mSelectedIndex >= (int32_t)mOptions.size())
    {
        mSelectedIndex = mOptions.empty() ? -1 : 0;
    }
    SyncOptionsString();
    MarkDirty();
}

const std::vector<std::string>& ComboBox::GetOptions() const
{
    return mOptions;
}

int32_t ComboBox::GetOptionCount() const
{
    return (int32_t)mOptions.size();
}

void ComboBox::SetSelectedIndex(int32_t index)
{
    index = glm::clamp(index, -1, (int32_t)mOptions.size() - 1);
    if (mSelectedIndex != index)
    {
        mSelectedIndex = index;
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("SelectionChanged", { this });
            CallFunction("OnSelectionChanged", { this });
        }
    }
}

int32_t ComboBox::GetSelectedIndex() const
{
    return mSelectedIndex;
}

std::string ComboBox::GetSelectedOption() const
{
    if (mSelectedIndex >= 0 && mSelectedIndex < (int32_t)mOptions.size())
    {
        return mOptions[mSelectedIndex];
    }
    return "";
}

bool ComboBox::IsOpen() const
{
    return mIsOpen;
}

void ComboBox::Open()
{
    if (!mIsOpen && !mOptions.empty())
    {
        mIsOpen = true;

        // Initialize hovered index to current selection
        mHoveredIndex = (mSelectedIndex >= 0) ? mSelectedIndex : 0;

        // Reset scroll offset to show selection centered (if possible)
        int32_t visibleCount = glm::min((int32_t)mOptions.size(), mMaxVisibleItems);
        if (mSelectedIndex >= 0)
        {
            // Try to center the selected item
            mScrollOffset = glm::max(0, mSelectedIndex - visibleCount / 2);
            // Clamp to valid range
            int32_t maxOffset = glm::max(0, (int32_t)mOptions.size() - visibleCount);
            mScrollOffset = glm::min(mScrollOffset, maxOffset);
        }
        else
        {
            mScrollOffset = 0;
        }

        CreateDropdownItems();
        MarkDirty();
    }
}

void ComboBox::Close()
{
    if (mIsOpen)
    {
        mIsOpen = false;
        mScrollOffset = 0;
        DestroyDropdownItems();
        MarkDirty();
    }
}

void ComboBox::Toggle()
{
    if (mIsOpen)
    {
        Close();
    }
    else
    {
        Open();
    }
}

void ComboBox::SetBackgroundColor(glm::vec4 color)
{
    if (mBackgroundColor != color)
    {
        mBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 ComboBox::GetBackgroundColor()
{
    return mBackgroundColor;
}

void ComboBox::SetTextColor(glm::vec4 color)
{
    if (mTextColor != color)
    {
        mTextColor = color;
        MarkDirty();
    }
}

glm::vec4 ComboBox::GetTextColor()
{
    return mTextColor;
}

void ComboBox::SetDropdownColor(glm::vec4 color)
{
    if (mDropdownColor != color)
    {
        mDropdownColor = color;
        MarkDirty();
    }
}

glm::vec4 ComboBox::GetDropdownColor()
{
    return mDropdownColor;
}

void ComboBox::SetHoveredColor(glm::vec4 color)
{
    if (mHoveredColor != color)
    {
        mHoveredColor = color;
        MarkDirty();
    }
}

glm::vec4 ComboBox::GetHoveredColor()
{
    return mHoveredColor;
}

void ComboBox::SetMaxVisibleItems(int32_t count)
{
    if (mMaxVisibleItems != count)
    {
        mMaxVisibleItems = glm::max(1, count);
        MarkDirty();
    }
}

int32_t ComboBox::GetMaxVisibleItems() const
{
    return mMaxVisibleItems;
}

Quad* ComboBox::GetBackground()
{
    return mBackground;
}

Text* ComboBox::GetTextWidget()
{
    return mText;
}

void ComboBox::CycleSelectedIndex(int32_t delta)
{
    if (mOptions.empty())
    {
        return;
    }

    int32_t newIndex = mSelectedIndex + delta;

    // Wrap around
    if (newIndex < 0)
    {
        newIndex = (int32_t)mOptions.size() - 1;
    }
    else if (newIndex >= (int32_t)mOptions.size())
    {
        newIndex = 0;
    }

    SetSelectedIndex(newIndex);
}

void ComboBox::CycleHoveredIndex(int32_t delta)
{
    if (mOptions.empty() || mDropdownItems.empty())
    {
        return;
    }

    int32_t newIndex = mHoveredIndex + delta;

    // Clamp to valid range across ALL options (not just visible)
    int32_t maxIndex = (int32_t)mOptions.size() - 1;
    newIndex = glm::clamp(newIndex, 0, maxIndex);

    if (mHoveredIndex != newIndex)
    {
        mHoveredIndex = newIndex;

        // Scroll to keep hovered item visible
        int32_t visibleCount = glm::min((int32_t)mOptions.size(), mMaxVisibleItems);
        if (mHoveredIndex < mScrollOffset)
        {
            mScrollOffset = mHoveredIndex;
        }
        else if (mHoveredIndex >= mScrollOffset + visibleCount)
        {
            mScrollOffset = mHoveredIndex - visibleCount + 1;
        }

        UpdateDropdown();
    }
}

void ComboBox::SetHandleMouse(bool inHandle)
{
    sHandleMouseInput = inHandle;
}

void ComboBox::SetHandleKeyboard(bool inHandle)
{
    sHandleKeyboardInput = inHandle;
}

void ComboBox::SetHandleGamepad(bool inHandle)
{
    sHandleGamepadInput = inHandle;
}
