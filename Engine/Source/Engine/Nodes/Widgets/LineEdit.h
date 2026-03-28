#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;
class Text;

class OCTAVE_API LineEdit : public Widget
{
public:

    DECLARE_NODE(LineEdit, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;

    // Text manipulation
    void SetText(const std::string& text);
    const std::string& GetText() const;
    void SetPlaceholder(const std::string& placeholder);
    const std::string& GetPlaceholder() const;

    // Caret/selection
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

    // Focus
    void SetFocused(bool focused);
    bool IsFocused() const;

    // Options
    void SetPasswordMode(bool enabled);
    bool IsPasswordMode() const;
    void SetMaxLength(int32_t maxLen);
    int32_t GetMaxLength() const;
    void SetEditable(bool editable);
    bool IsEditable() const;

    // Visual
    void SetBackgroundTexture(Texture* texture);
    Texture* GetBackgroundTexture();
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor();
    void SetFocusedBackgroundColor(glm::vec4 color);
    glm::vec4 GetFocusedBackgroundColor();
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor();
    void SetPlaceholderColor(glm::vec4 color);
    glm::vec4 GetPlaceholderColor();
    void SetCaretColor(glm::vec4 color);
    glm::vec4 GetCaretColor();
    void SetSelectionColor(glm::vec4 color);
    glm::vec4 GetSelectionColor();
    void SetTextPadding(float padding);
    float GetTextPadding() const;

    // Children
    Quad* GetBackground();
    Text* GetTextWidget();
    Quad* GetCaret();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void InsertCharacter(char c);
    void InsertText(const std::string& text);
    void DeleteCharacter(bool forward);
    void MoveCaret(int32_t delta, bool select);
    void MoveCaretToWordBoundary(int32_t direction, bool select);

    float GetCaretXPosition();
    int32_t GetCharacterAtPosition(float x);
    void UpdateDisplayText();
    void UpdateCaretVisual();
    void UpdateSelectionVisual();
    void EnsureCaretVisible();
    void ExtendSelection();
    void EmitTextChanged();

    // Content
    std::string mTextContent;
    std::string mPlaceholderText = "Enter text...";

    // Editing state
    int32_t mCaretPosition = 0;
    int32_t mSelectionStart = -1;
    int32_t mSelectionEnd = -1;
    int32_t mSelectionAnchor = -1; // For shift+click selection
    bool mFocused = false;
    bool mEditable = true;

    // Caret blink
    float mCaretBlinkTime = 0.0f;
    float mCaretBlinkRate = 0.5f;
    bool mCaretVisible = true;

    // Options
    bool mPasswordMode = false;
    int32_t mMaxLength = 0;
    char mPasswordChar = '*';

    // Visual
    TextureRef mBackgroundTexture;
    glm::vec4 mBackgroundColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 mFocusedBackgroundColor = { 0.25f, 0.25f, 0.25f, 1.0f };
    glm::vec4 mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mPlaceholderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    glm::vec4 mCaretColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mSelectionColor = { 0.3f, 0.5f, 0.8f, 0.5f };
    float mTextPadding = 4.0f;
    float mScrollOffset = 0.0f;

    // Children
    Quad* mBackground = nullptr;
    Quad* mSelection = nullptr;
    Text* mText = nullptr;
    Quad* mCaret = nullptr;
};
