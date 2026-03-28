#pragma once

#include "Nodes/Widgets/Widget.h"

class Text;
class InputField;

class OCTAVE_API LineEdit : public Widget
{
public:

    DECLARE_NODE(LineEdit, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void PreRender() override;

    // Title
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const;
    void SetTitleWidth(float width);
    float GetTitleWidth() const;

    // InputField passthrough - Text manipulation
    void SetText(const std::string& text);
    const std::string& GetText() const;
    void SetPlaceholder(const std::string& placeholder);
    const std::string& GetPlaceholder() const;

    // InputField passthrough - Caret/selection
    void SetCaretPosition(int32_t pos);
    int32_t GetCaretPosition() const;
    void SelectAll();
    void ClearSelection();
    bool HasSelection() const;
    std::string GetSelectedText() const;
    void DeleteSelection();
    void Select(int32_t start, int32_t end);
    int32_t GetSelectionStart() const;
    int32_t GetSelectionEnd() const;

    // InputField passthrough - Focus
    void SetFocused(bool focused);
    bool IsFocused() const;

    // InputField passthrough - Options
    void SetPasswordMode(bool enabled);
    bool IsPasswordMode() const;
    void SetMaxLength(int32_t maxLen);
    int32_t GetMaxLength() const;
    void SetEditable(bool editable);
    bool IsEditable() const;

    // Children
    Text* GetTitleWidget();
    InputField* GetInputField();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateLayout();

    // Title
    std::string mTitle = "Label";
    float mTitleWidth = 100.0f;
    float mSpacing = 8.0f;
    glm::vec4 mTitleColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Children
    Text* mTitleWidget = nullptr;
    InputField* mInputField = nullptr;
};
