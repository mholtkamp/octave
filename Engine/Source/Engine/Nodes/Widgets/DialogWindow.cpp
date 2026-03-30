#include "Nodes/Widgets/DialogWindow.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/ScrollContainer.h"
#include "Assets/Texture.h"
#include "InputDevices.h"
#include "Log.h"

#if EDITOR
#include "imgui.h"
#endif

FORCE_LINK_DEF(DialogWindow);
DEFINE_NODE(DialogWindow, Window);

#if EDITOR
static bool sRefreshButtonBar = false;
#endif

static const char* sButtonBarAlignmentStrings[] = {
    "Left",
    "Center",
    "Right",
    "Spread"
};

bool DialogWindow::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop->mOwner != nullptr);
    DialogWindow* dialog = static_cast<DialogWindow*>(prop->mOwner);

    bool success = false;

    // Confirm Button
    if (prop->mName == "Confirm Text")
    {
        dialog->SetConfirmText(*static_cast<const std::string*>(newValue));
        success = true;
    }
    else if (prop->mName == "Confirm Normal Color")
    {
        dialog->SetConfirmNormalColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Confirm Hovered Color")
    {
        dialog->SetConfirmHoveredColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Confirm Pressed Color")
    {
        dialog->SetConfirmPressedColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Confirm Texture")
    {
        dialog->SetConfirmTexture(*(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Show Confirm Button")
    {
        dialog->SetShowConfirmButton(*static_cast<const bool*>(newValue));
        success = true;
    }
    // Reject Button
    else if (prop->mName == "Reject Text")
    {
        dialog->SetRejectText(*static_cast<const std::string*>(newValue));
        success = true;
    }
    else if (prop->mName == "Reject Normal Color")
    {
        dialog->SetRejectNormalColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Reject Hovered Color")
    {
        dialog->SetRejectHoveredColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Reject Pressed Color")
    {
        dialog->SetRejectPressedColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Reject Texture")
    {
        dialog->SetRejectTexture(*(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Show Reject Button")
    {
        dialog->SetShowRejectButton(*static_cast<const bool*>(newValue));
        success = true;
    }
    // Button Bar
    else if (prop->mName == "Button Bar Height")
    {
        dialog->SetButtonBarHeight(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Button Spacing")
    {
        dialog->SetButtonSpacing(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Button Bar Alignment")
    {
        dialog->SetButtonBarAlignment((ButtonBarAlignment)*static_cast<const uint8_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Button Bar Color")
    {
        dialog->SetButtonBarColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Button Width")
    {
        dialog->SetButtonWidth(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Button Bar Padding")
    {
        dialog->SetButtonBarPadding(*static_cast<const float*>(newValue));
        success = true;
    }

    return success;
}

void DialogWindow::Create()
{
    Window::Create();
    SetName("DialogWindow");
    SetTitle("DialogWindow");

    // Button bar is created lazily in EnsureButtonBar()
    // This avoids conflicts during Clone operations
    mButtonBar = nullptr;
    mButtonBarBackground = nullptr;
    mConfirmButton = nullptr;
    mRejectButton = nullptr;

    MarkDirty();
}

void DialogWindow::Start()
{
    Window::Start();

    // Ensure button bar exists and signals are connected when play begins
    EnsureButtonBar();

    // Force reconnect signals at Start (in case they were connected in editor)
    mButtonSignalsConnected = false;
    if (mConfirmButton != nullptr)
    {
        mConfirmButton->DisconnectSignal("Activated", this);
        mConfirmButton->ConnectSignal("Activated", this, &DialogWindow::OnConfirmButtonActivated);
    }
    if (mRejectButton != nullptr)
    {
        mRejectButton->DisconnectSignal("Activated", this);
        mRejectButton->ConnectSignal("Activated", this, &DialogWindow::OnRejectButtonActivated);
    }
    mButtonSignalsConnected = true;
}

void DialogWindow::EnsureButtonBar()
{
    if (mButtonBar != nullptr)
    {
        // Ensure signals are connected (may be missing if created before code change)
        if (!mButtonSignalsConnected)
        {
            if (mConfirmButton != nullptr)
            {
                mConfirmButton->ConnectSignal("Activated", this, &DialogWindow::OnConfirmButtonActivated);
            }
            if (mRejectButton != nullptr)
            {
                mRejectButton->ConnectSignal("Activated", this, &DialogWindow::OnRejectButtonActivated);
            }
            mButtonSignalsConnected = true;
        }
        return;
    }

    // Create button bar canvas (transient to avoid rerouting to content)
    mButtonBar = CreateChild<Canvas>("ButtonBar");
    mButtonBar->SetTransient(true);
    mButtonBar->SetAnchorMode(AnchorMode::TopLeft);
    mButtonBar->SetDimensions(GetWidth(), mButtonBarHeight);
    mButtonBar->SetPosition(0.0f, GetHeight() - mButtonBarHeight);

    // Create button bar background
    mButtonBarBackground = mButtonBar->CreateChild<Quad>("ButtonBarBg");
    mButtonBarBackground->SetTransient(true);
    mButtonBarBackground->SetAnchorMode(AnchorMode::FullStretch);
    mButtonBarBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mButtonBarBackground->SetColor(mButtonBarColor);

    // Create confirm button
    mConfirmButton = mButtonBar->CreateChild<Button>("ConfirmButton");
    mConfirmButton->SetTransient(true);
    mConfirmButton->SetAnchorMode(AnchorMode::TopLeft);
    mConfirmButton->SetDimensions(mButtonWidth, mButtonBarHeight - 8.0f);
    mConfirmButton->SetTextString(mConfirmText);
    mConfirmButton->SetNormalColor(mConfirmNormalColor);
    mConfirmButton->SetHoveredColor(mConfirmHoveredColor);
    mConfirmButton->SetPressedColor(mConfirmPressedColor);
    mConfirmButton->SetVisible(mShowConfirmButton);
    mConfirmButton->ConnectSignal("Activated", this, &DialogWindow::OnConfirmButtonActivated);

    // Create reject button
    mRejectButton = mButtonBar->CreateChild<Button>("RejectButton");
    mRejectButton->SetTransient(true);
    mRejectButton->SetAnchorMode(AnchorMode::TopLeft);
    mRejectButton->SetDimensions(mButtonWidth, mButtonBarHeight - 8.0f);
    mRejectButton->SetTextString(mRejectText);
    mRejectButton->SetNormalColor(mRejectNormalColor);
    mRejectButton->SetHoveredColor(mRejectHoveredColor);
    mRejectButton->SetPressedColor(mRejectPressedColor);
    mRejectButton->SetVisible(mShowRejectButton);
    mRejectButton->ConnectSignal("Activated", this, &DialogWindow::OnRejectButtonActivated);

    mButtonSignalsConnected = true;
}

void DialogWindow::Tick(float deltaTime)
{
    Window::Tick(deltaTime);
}

void DialogWindow::OnConfirmButtonActivated(Node* listener, const std::vector<Datum>& args)
{
    LogDebug("Confirm Clicked");
    DialogWindow* dialog = static_cast<DialogWindow*>(listener);
    dialog->Confirm();
}

void DialogWindow::OnRejectButtonActivated(Node* listener, const std::vector<Datum>& args)
{
    DialogWindow* dialog = static_cast<DialogWindow*>(listener);
    dialog->Reject();
}

void DialogWindow::PreRender()
{
    Window::PreRender();

    // Ensure button bar exists (lazy initialization)
    EnsureButtonBar();

    // Update button bar layout
    UpdateButtonBar();
}

void DialogWindow::GatherProperties(std::vector<Property>& props)
{
    Window::GatherProperties(props);

    SCOPED_CATEGORY("Dialog");

    // Confirm Button
    props.push_back(Property(DatumType::String, "Confirm Text", this, &mConfirmText, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Confirm Normal Color", this, &mConfirmNormalColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Confirm Hovered Color", this, &mConfirmHoveredColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Confirm Pressed Color", this, &mConfirmPressedColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Asset, "Confirm Texture", this, &mConfirmTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Bool, "Show Confirm Button", this, &mShowConfirmButton, 1, HandlePropChange));

    // Reject Button
    props.push_back(Property(DatumType::String, "Reject Text", this, &mRejectText, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Reject Normal Color", this, &mRejectNormalColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Reject Hovered Color", this, &mRejectHoveredColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Reject Pressed Color", this, &mRejectPressedColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Asset, "Reject Texture", this, &mRejectTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Bool, "Show Reject Button", this, &mShowRejectButton, 1, HandlePropChange));

    // Button Bar
    props.push_back(Property(DatumType::Float, "Button Bar Height", this, &mButtonBarHeight, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Button Spacing", this, &mButtonSpacing, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Button Width", this, &mButtonWidth, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Button Bar Padding", this, &mButtonBarPadding, 1, HandlePropChange));
    props.push_back(Property(DatumType::Byte, "Button Bar Alignment", this, &mButtonBarAlignment, 1,
        HandlePropChange, NULL_DATUM, int32_t(ButtonBarAlignment::Count), sButtonBarAlignmentStrings));
    props.push_back(Property(DatumType::Color, "Button Bar Color", this, &mButtonBarColor, 1, HandlePropChange));

#if EDITOR
    props.push_back(Property(DatumType::Bool, "Refresh Button Bar", this, &sRefreshButtonBar));
#endif
}


void DialogWindow::UpdateButtonBar()
{
    if (mButtonBar == nullptr)
    {
        return;
    }

    // Update button bar size and position (manual layout since BottomStretch had issues)
    mButtonBar->SetDimensions(GetWidth(), mButtonBarHeight);
    mButtonBar->SetPosition(0.0f, GetHeight() - mButtonBarHeight);

    // Update background color
    if (mButtonBarBackground != nullptr)
    {
        mButtonBarBackground->SetColor(mButtonBarColor);
    }

    // Calculate visible button count and total width
    int visibleCount = 0;
    if (mShowConfirmButton) visibleCount++;
    if (mShowRejectButton) visibleCount++;

    float totalButtonWidth = visibleCount * mButtonWidth;
    if (visibleCount > 1)
    {
        totalButtonWidth += (visibleCount - 1) * mButtonSpacing;
    }

    float barWidth = GetWidth();
    float buttonY = (mButtonBarHeight - (mButtonBarHeight - 8.0f)) / 2.0f;
    float buttonHeight = mButtonBarHeight - 8.0f;

    // Calculate start position based on alignment
    float startX = mButtonBarPadding;
    float spreadSpacing = mButtonSpacing;

    switch (mButtonBarAlignment)
    {
        case ButtonBarAlignment::Left:
            startX = mButtonBarPadding;
            break;
        case ButtonBarAlignment::Center:
            startX = (barWidth - totalButtonWidth) / 2.0f;
            break;
        case ButtonBarAlignment::Right:
            startX = barWidth - totalButtonWidth - mButtonBarPadding;
            break;
        case ButtonBarAlignment::Spread:
            if (visibleCount > 1)
            {
                spreadSpacing = (barWidth - 2 * mButtonBarPadding - visibleCount * mButtonWidth) / (visibleCount - 1);
            }
            startX = mButtonBarPadding;
            break;
        default:
            break;
    }

    // Position buttons
    float currentX = startX;

    // Reject button comes first (on the left)
    if (mRejectButton != nullptr)
    {
        mRejectButton->SetVisible(mShowRejectButton);
        mRejectButton->SetDimensions(mButtonWidth, buttonHeight);
        mRejectButton->SetAnchorMode(AnchorMode::TopLeft);
        if (mShowRejectButton)
        {
            mRejectButton->SetPosition(currentX, buttonY);
            currentX += mButtonWidth + (mButtonBarAlignment == ButtonBarAlignment::Spread ? spreadSpacing : mButtonSpacing);
        }

        // Update colors
        mRejectButton->SetNormalColor(mRejectNormalColor);
        mRejectButton->SetHoveredColor(mRejectHoveredColor);
        mRejectButton->SetPressedColor(mRejectPressedColor);
        mRejectButton->SetTextString(mRejectText);
        if (mRejectTexture.Get<Texture>() != nullptr)
        {
            mRejectButton->SetNormalTexture(mRejectTexture.Get<Texture>());
        }
    }

    // Confirm button comes second (on the right)
    if (mConfirmButton != nullptr)
    {
        mConfirmButton->SetVisible(mShowConfirmButton);
        mConfirmButton->SetDimensions(mButtonWidth, buttonHeight);
        mConfirmButton->SetAnchorMode(AnchorMode::TopLeft);
        if (mShowConfirmButton)
        {
            mConfirmButton->SetPosition(currentX, buttonY);
        }

        // Update colors
        mConfirmButton->SetNormalColor(mConfirmNormalColor);
        mConfirmButton->SetHoveredColor(mConfirmHoveredColor);
        mConfirmButton->SetPressedColor(mConfirmPressedColor);
        mConfirmButton->SetTextString(mConfirmText);
        if (mConfirmTexture.Get<Texture>() != nullptr)
        {
            mConfirmButton->SetNormalTexture(mConfirmTexture.Get<Texture>());
        }
    }

    // Shrink content area to make room for button bar
    // Note: Window::UpdateLayout handles content positioning, we just need to
    // ensure the content container is aware of the button bar
    ScrollContainer* contentContainer = GetContentContainer();
    if (contentContainer != nullptr)
    {
        float contentH = GetHeight() - GetTitleBarHeight() - mButtonBarHeight - GetContentPaddingTop() - GetContentPaddingBottom();
        contentContainer->SetHeight(glm::max(1.0f, contentH));
    }
}

void DialogWindow::Confirm()
{
    EmitSignal("Confirm", { this });
    CallFunction("OnConfirm", { this });
}

void DialogWindow::Reject()
{
    EmitSignal("Reject", { this });
    CallFunction("OnReject", { this });
}

void DialogWindow::RefreshButtonBar()
{
    // Destroy existing button bar
    if (mButtonBar != nullptr)
    {
        if (mConfirmButton != nullptr)
        {
            mConfirmButton->DisconnectSignal("Activated", this);
        }
        if (mRejectButton != nullptr)
        {
            mRejectButton->DisconnectSignal("Activated", this);
        }

        mButtonBar->Destroy();
        mButtonBar = nullptr;
        mButtonBarBackground = nullptr;
        mConfirmButton = nullptr;
        mRejectButton = nullptr;
        mButtonSignalsConnected = false;
    }

    // EnsureButtonBar will recreate on next PreRender
    LogDebug("DialogWindow '%s': Button bar refreshed", GetName().c_str());
}

#if EDITOR
bool DialogWindow::DrawCustomProperty(Property& prop)
{
    if (prop.mName == "Refresh Button Bar")
    {
        if (ImGui::Button("Refresh Button Bar"))
        {
            RefreshButtonBar();
        }
        return true;
    }

    return false;
}
#endif

// Confirm Button Getters/Setters

void DialogWindow::SetConfirmText(const std::string& text)
{
    mConfirmText = text;
    MarkDirty();
}

const std::string& DialogWindow::GetConfirmText() const
{
    return mConfirmText;
}

void DialogWindow::SetConfirmNormalColor(glm::vec4 color)
{
    mConfirmNormalColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetConfirmNormalColor() const
{
    return mConfirmNormalColor;
}

void DialogWindow::SetConfirmHoveredColor(glm::vec4 color)
{
    mConfirmHoveredColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetConfirmHoveredColor() const
{
    return mConfirmHoveredColor;
}

void DialogWindow::SetConfirmPressedColor(glm::vec4 color)
{
    mConfirmPressedColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetConfirmPressedColor() const
{
    return mConfirmPressedColor;
}

void DialogWindow::SetConfirmTexture(Texture* texture)
{
    mConfirmTexture = texture;
    MarkDirty();
}

Texture* DialogWindow::GetConfirmTexture()
{
    return mConfirmTexture.Get<Texture>();
}

void DialogWindow::SetShowConfirmButton(bool show)
{
    mShowConfirmButton = show;
    MarkDirty();
}

bool DialogWindow::GetShowConfirmButton() const
{
    return mShowConfirmButton;
}

// Reject Button Getters/Setters

void DialogWindow::SetRejectText(const std::string& text)
{
    mRejectText = text;
    MarkDirty();
}

const std::string& DialogWindow::GetRejectText() const
{
    return mRejectText;
}

void DialogWindow::SetRejectNormalColor(glm::vec4 color)
{
    mRejectNormalColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetRejectNormalColor() const
{
    return mRejectNormalColor;
}

void DialogWindow::SetRejectHoveredColor(glm::vec4 color)
{
    mRejectHoveredColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetRejectHoveredColor() const
{
    return mRejectHoveredColor;
}

void DialogWindow::SetRejectPressedColor(glm::vec4 color)
{
    mRejectPressedColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetRejectPressedColor() const
{
    return mRejectPressedColor;
}

void DialogWindow::SetRejectTexture(Texture* texture)
{
    mRejectTexture = texture;
    MarkDirty();
}

Texture* DialogWindow::GetRejectTexture()
{
    return mRejectTexture.Get<Texture>();
}

void DialogWindow::SetShowRejectButton(bool show)
{
    mShowRejectButton = show;
    MarkDirty();
}

bool DialogWindow::GetShowRejectButton() const
{
    return mShowRejectButton;
}

// Button Bar Getters/Setters

void DialogWindow::SetButtonBarHeight(float height)
{
    mButtonBarHeight = height;
    MarkDirty();
}

float DialogWindow::GetButtonBarHeight() const
{
    return mButtonBarHeight;
}

void DialogWindow::SetButtonSpacing(float spacing)
{
    mButtonSpacing = spacing;
    MarkDirty();
}

float DialogWindow::GetButtonSpacing() const
{
    return mButtonSpacing;
}

void DialogWindow::SetButtonBarAlignment(ButtonBarAlignment alignment)
{
    mButtonBarAlignment = alignment;
    MarkDirty();
}

ButtonBarAlignment DialogWindow::GetButtonBarAlignment() const
{
    return mButtonBarAlignment;
}

void DialogWindow::SetButtonBarColor(glm::vec4 color)
{
    mButtonBarColor = color;
    MarkDirty();
}

glm::vec4 DialogWindow::GetButtonBarColor() const
{
    return mButtonBarColor;
}

void DialogWindow::SetButtonWidth(float width)
{
    mButtonWidth = width;
    MarkDirty();
}

float DialogWindow::GetButtonWidth() const
{
    return mButtonWidth;
}

void DialogWindow::SetButtonBarPadding(float padding)
{
    mButtonBarPadding = padding;
    MarkDirty();
}

float DialogWindow::GetButtonBarPadding() const
{
    return mButtonBarPadding;
}

// Internal Widget Access

Canvas* DialogWindow::GetButtonBar()
{
    return mButtonBar;
}

Button* DialogWindow::GetConfirmButton()
{
    return mConfirmButton;
}

Button* DialogWindow::GetRejectButton()
{
    return mRejectButton;
}
