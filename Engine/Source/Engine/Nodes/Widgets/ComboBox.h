#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;
class Text;
class Button;
class Canvas;

class OCTAVE_API ComboBox : public Widget
{
public:

    DECLARE_NODE(ComboBox, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;

    // Options
    void AddOption(const std::string& option);
    void RemoveOption(int32_t index);
    void ClearOptions();
    void SetOptions(const std::vector<std::string>& options);
    const std::vector<std::string>& GetOptions() const;
    int32_t GetOptionCount() const;

    // Selection
    void SetSelectedIndex(int32_t index);
    int32_t GetSelectedIndex() const;
    std::string GetSelectedOption() const;

    // State
    bool IsOpen() const;
    void Open();
    void Close();
    void Toggle();

    // Visual
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor();
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor();
    void SetDropdownColor(glm::vec4 color);
    glm::vec4 GetDropdownColor();
    void SetHoveredColor(glm::vec4 color);
    glm::vec4 GetHoveredColor();
    void SetMaxVisibleItems(int32_t count);
    int32_t GetMaxVisibleItems() const;

    // Children
    Quad* GetBackground();
    Text* GetTextWidget();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateAppearance();
    void UpdateDropdown();
    void CreateDropdownItems();
    void DestroyDropdownItems();
    void SyncOptionsString();

    // Options
    std::vector<std::string> mOptions;
    std::string mOptionsString;  // Newline-separated options for editor
    int32_t mSelectedIndex = -1;

    // State
    bool mIsOpen = false;
    int32_t mHoveredIndex = -1;

    // Visual
    TextureRef mBackgroundTexture;
    glm::vec4 mBackgroundColor = { 0.25f, 0.25f, 0.25f, 1.0f };
    glm::vec4 mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mDropdownColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 mHoveredColor = { 0.35f, 0.35f, 0.35f, 1.0f };
    glm::vec4 mArrowColor = { 0.7f, 0.7f, 0.7f, 1.0f };
    float mArrowWidth = 20.0f;
    float mItemHeight = 24.0f;
    int32_t mMaxVisibleItems = 5;

    // Children
    Quad* mBackground = nullptr;
    Text* mText = nullptr;
    Quad* mArrow = nullptr;

    // Dropdown (created dynamically when open)
    Canvas* mDropdownCanvas = nullptr;
    std::vector<Quad*> mDropdownItems;
    std::vector<Text*> mDropdownTexts;
};
