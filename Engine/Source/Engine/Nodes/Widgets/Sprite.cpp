#include "Nodes/Widgets/Sprite.h"

#include "AssetManager.h"

FORCE_LINK_DEF(Sprite);
DEFINE_NODE(Sprite, Quad);


bool Sprite::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Sprite* sprite = static_cast<Sprite*>(prop->mOwner);
    bool success = false;

     if (prop->mName.rfind("Frame ", 0) == 0)
    {
        int frameIndex = atoi(prop->mName.substr(6).c_str()) - 1;
        sprite->AddFrame(*(Texture**)newValue, frameIndex, false);
        success = true;
    }
    else if (prop->mName == "Play")
    {
        sprite->SetPlay(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Active Animation")
    {
        sprite->SetAnimation(*static_cast<const int32_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Active Frame")
    {
        sprite->SetFrame(*static_cast<const int32_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Animation Name")
    {
        sprite->SetAnimationName(*((std::string*) newValue), sprite->mCurrentAnimation);
        success = true;
    }
    else if (prop->mName == "Add Animation")
    {
        sprite->AddAnimation();
        success = true;
    }
    else if (prop->mName == "Remove Animation")
    {
        if (sprite->mAnimation.size() > 0 && sprite->mCurrentAnimation >= 0)
        {
            sprite->RemoveAnimation(sprite->mCurrentAnimation);
        }
        success = true;
    }
    else if (prop->mName == "Add Frame")
    {
        sprite->AddEmptyFrame();
        success = true;
    }
    else if (prop->mName == "Remove Frame")
    {
        if (sprite->GetNumFrames(sprite->mCurrentAnimation) > 0 && sprite->mCurrentAnimation >= 0 && sprite->mFrame >= 0)
        {
            sprite->RemoveFrame(sprite->mFrame);
        }
        success = true;
    }
    else if (prop->mName == "")
    {
        //just throwing space for textures dropdown to show up on lower frames
        success = true;
    }

    sprite->MarkDirty();

    return success;
}

Sprite::Sprite()
{
    SetName("Sprite");
    SetFPS(5.0f);
}

Sprite::~Sprite()
{

}

void Sprite::Create()
{
    Quad::Create();
}

void Sprite::Tick(float deltaTime)
{
    TickCommon(deltaTime);
    Node::Tick(deltaTime);
}

void Sprite::EditorTick(float deltaTime)
{
    TickCommon(deltaTime);
    Node::EditorTick(deltaTime);
}

void Sprite::TickCommon(float deltaTime)
{
    bool GTE = IsGameTickEnabled();

    if (GTE && IsPlaying() && (mCurrentAnimation >= 0))
    {
        mFrameTime += deltaTime;
        if (mFrameTime >= (1 / mFPS))
        {
            AdvanceFrame();
            mFrameTime = 0.0f;
        }
    }
}

void Sprite::SaveStream(Stream& stream, Platform platorm)
{
    stream.WriteUint32(mAnimation.size());
    // For serializing extra data besides properties
    for (SpriteAnimation animation : mAnimation)
    {
        stream.WriteString(animation.mName);
        stream.WriteBool(animation.loop);
        stream.WriteUint32(animation.mFrame.size());
        for (TextureRef frame : animation.mFrame)
        {
            stream.WriteAsset(frame);
        }
    }
}

void Sprite::LoadStream(Stream& stream, Platform platorm, uint32_t version)
{
    // For serializing extra data besides properties
    uint32_t numAnimations = stream.ReadUint32();
    mAnimation.resize(numAnimations);
    for (uint32_t i = 0; i < numAnimations; ++i)
    {
        stream.ReadString(mAnimation[i].mName);
        mAnimation[i].loop = stream.ReadBool();
        uint32_t numFrames = stream.ReadUint32();
        mAnimation[i].mFrame.resize(numFrames);
        for (uint32_t j = 0; j < numFrames; ++j)
        {
            stream.ReadAsset(mAnimation[i].mFrame[j]);
        }
    }
}


void Sprite::AdvanceFrame()
{
    ++mFrame;
    if (mFrame >= mAnimation[mCurrentAnimation].mFrame.size())
    {
        mFrame = 0;
        if (!GetLoop())
        {
            SetPlay(false);
            return;
        }
    }
    if (mAnimation[mCurrentAnimation].mFrame[mFrame] == nullptr)
        SetTexture(nullptr);
    else
        SetTexture(mAnimation[mCurrentAnimation].mFrame[mFrame].Get<Texture>());

}

void Sprite::AddAnimation(std::string animationName)
{
    if (animationName == "")
    {
        int32_t i = 1;
        bool flag = false;
        while (!flag)
        {
            std::string checkName = "Animation ";
            checkName = checkName +  std::to_string(i);
            bool bflag = false;
            for (SpriteAnimation animation : mAnimation)
            {
                if (animation.mName == checkName) bflag = true;
            }
            if (!bflag)
            {
                animationName = checkName;
                flag = true;
            }
            ++i;
        }
    }
    for (SpriteAnimation animation : mAnimation)
    {
        if (animationName == animation.mName)
        {
            LogDebug("Animations cannot share names");
            return;
        }
    }
    SpriteAnimation animation;
    animation.mName = animationName;
    animation.loop = true;
    mAnimation.push_back(animation);
    if (mCurrentAnimation == -1) SetAnimation(animationName);
    MarkDirty();

}
void Sprite::AddFrame(class Texture* texture, int32_t frameIndex, bool insert, int32_t animationIndex) //-1 frame means add to back, insert will take frame at index and push it back
{
    SpriteAnimation animation;
    if (animationIndex == -1)
    {
        animationIndex = mCurrentAnimation;
    }
    if (animationIndex == -1) return;
    if (frameIndex == -1) mAnimation[animationIndex].mFrame.push_back(texture);
    else if (frameIndex >= mAnimation[animationIndex].mFrame.size()) mAnimation[animationIndex].mFrame.push_back(texture);
    else
    {

        if (!insert)
        {
            mAnimation[animationIndex].mFrame[frameIndex] = texture;
        }
        else
        {
            mAnimation[animationIndex].mFrame.push_back(nullptr);
            for (uint32_t i = mAnimation[animationIndex].mFrame.size() - 1; i > frameIndex; --i)
            {
                mAnimation[animationIndex].mFrame[i] = mAnimation[animationIndex].mFrame[i - 1];
            }
            mAnimation[animationIndex].mFrame[frameIndex] = texture;
        }

        if (mFrame == frameIndex) SetTexture(mAnimation[animationIndex].mFrame[mFrame].Get<Texture>());
    }
    MarkDirty();
}
void Sprite::AddEmptyFrame(int32_t frameIndex, bool insert, int32_t animationIndex)
{
    AddFrame(nullptr, frameIndex, insert, animationIndex);
}

void Sprite::RemoveAnimation(uint32_t animationIndex)
{
    mAnimation.erase(mAnimation.begin() + animationIndex);
    MarkDirty();
}
void Sprite::RemoveFrame(uint32_t frameIndex, int32_t animationIndex)
{
    if (animationIndex < 0) mAnimation[mCurrentAnimation].mFrame.erase(mAnimation[mCurrentAnimation].mFrame.begin() + frameIndex);
    else mAnimation[animationIndex].mFrame.erase(mAnimation[animationIndex].mFrame.begin() + frameIndex);
    MarkDirty();
}

void Sprite::SetAnimation(std::string animationName)
{
    int i = -1;
    for (SpriteAnimation animation : mAnimation)
    {
        ++i;
        if (animation.mName == animationName)
        {

            mCurrentAnimation = i;
            mAnimation[mCurrentAnimation].mFrame = animation.mFrame;
            mFrame = mAnimation[mCurrentAnimation].mFrame.size(); //makes it so animations start on their first frame;
            if (mFrame > 0) AdvanceFrame();
            Quad::MarkDirty();
            return;
        }
    }
    LogDebug("No animation '%s' found", animationName);
}
void Sprite::SetAnimation(uint32_t animationIndex)
{
    if (animationIndex < mAnimation.size())
    {
        mCurrentAnimation = animationIndex;
        mFrame = mAnimation[mCurrentAnimation].mFrame.size(); //makes it so animations start on their first frame;
        if (mFrame > 0) AdvanceFrame();
        MarkDirty();
        return;
    }
}
void Sprite::SetFrame(uint32_t frameIndex)
{
    mFrame = frameIndex - 1;
    AdvanceFrame();
    MarkDirty();
}
void Sprite::SetFPS(float fps)
{
    mFPS = fps;
    MarkDirty();
}
void Sprite::SetPlay(bool play)
{
    mPlaying = play;
    MarkDirty();
}
void Sprite::SetLoop(bool loop)
{
    mAnimation[mCurrentAnimation].loop = loop;
    MarkDirty();
}
void Sprite::SetAnimationName(std::string name, int32_t animationIndex)
{
    for (SpriteAnimation animation : mAnimation)
    {
        if ((animation.mName == name) && (animation.mName != mAnimation[animationIndex].mName))
        {
            LogDebug("Animations cannot share names");
            return;
        }
    }
    if (animationIndex == -1)
    {
        SpriteAnimation animation = GetCurrentAnimation();
        animation.mName = name;
        SetAnimation(name);
        MarkDirty();
        return;
    }
    if (animationIndex >= mAnimation.size()) return;
    mAnimation[animationIndex].mName = name;
    MarkDirty();
}
void Sprite::SetAnimationData(int32_t index, const SpriteAnimation& data)
{
    bool flag = false;
    if (index == GetAnimationIndex()) flag = true;
    OCT_ASSERT(index >= 0 && index < int32_t(mAnimation.size()));
    if (index >= 0 &&
        index < int32_t(mAnimation.size()))
    {
        mAnimation[index] = data;
    }
    if (flag)
    {
        SetAnimation(index);
    }
    MarkDirty();
}

Texture* Sprite::GetFrame(uint32_t frameIndex, int32_t animationIndex)
{
    if (animationIndex < 0) animationIndex = mCurrentAnimation;
    if (animationIndex < 0 || frameIndex < 0) return nullptr;
    if (frameIndex < mAnimation[mCurrentAnimation].mFrame.size())
    {
        if (mAnimation[animationIndex].mFrame[frameIndex] == nullptr) return nullptr;
        return mAnimation[mCurrentAnimation].mFrame[frameIndex].Get<Texture>();
    }
    return nullptr;
}
uint32_t Sprite::GetAnimationLength(std::string animationName)
{
    if (animationName == "") return mAnimation[mCurrentAnimation].mFrame.size();
    for (SpriteAnimation animation : mAnimation)
    {
        if (animationName == animation.mName) return animation.mFrame.size();
    }
    LogDebug("No animation '%s' found", animationName);
    return 0;
}
std::string Sprite::GetAnimationName()
{
    return mAnimation[mCurrentAnimation].mName;
}
bool Sprite::IsPlaying()
{
    return mPlaying;
}
bool Sprite::GetLoop()
{
    return mAnimation[mCurrentAnimation].loop;
}
int32_t Sprite::GetFrameIndex(uint32_t animationIndex)
{
    if (mAnimation[animationIndex].mFrame.size() == 0) return -1;
    return mFrame;
}
uint32_t Sprite::GetNumAnimations()
{
    return mAnimation.size();
}
std::vector<SpriteAnimation> Sprite::GetAnimations()
{
    return mAnimation;
}
int32_t Sprite::GetAnimationIndex()
{
    if (mCurrentAnimation == -1) return -1;
    for (uint32_t i = 0; i < mAnimation.size(); ++i)
    {
        if (mAnimation[mCurrentAnimation].mName == mAnimation[i].mName) return i;
    }
    return -1;
}
SpriteAnimation Sprite::GetCurrentAnimation()
{
    return mAnimation[mCurrentAnimation];
}
SpriteAnimation Sprite::GetAnimation(uint32_t animationIndex)
{
    return mAnimation[animationIndex];
}
uint32_t Sprite::GetNumFrames(uint32_t animationIndex)
{
    return mAnimation[animationIndex].mFrame.size();
}

void Sprite::GatherProperties(std::vector<Property>& outProps)
{
    Quad::GatherProperties(outProps);

    {
        SCOPED_CATEGORY("Sprite");

        outProps.push_back(Property(DatumType::Bool, "Play", this, &mPlaying, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Float, "FPS", this, &mFPS));
        static bool adremAnim = false;
        outProps.push_back(Property(DatumType::Bool, "Add Animation", this, &adremAnim, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Bool, "Remove Animation", this, &adremAnim, 1, HandlePropChange));
        if (mAnimation.size() > 0)
        {
            if (mCurrentAnimation < 0) mCurrentAnimation = 0;
            outProps.push_back(Property(DatumType::Integer, "Active Animation", this, &mCurrentAnimation, 1, HandlePropChange, NULL_DATUM, int32_t(mAnimation.size() - 1)));
            animName = mAnimation[mCurrentAnimation].mName;
            outProps.push_back(Property(DatumType::String, "Animation Name", this, &animName, 1, HandlePropChange));
        }
    }
    if (mCurrentAnimation >= 0 && mCurrentAnimation < mAnimation.size())
    {
        SCOPED_CATEGORY("Frames")
        static bool adremFrame = false;
        outProps.push_back(Property(DatumType::Bool, "Add Frame", this, &adremFrame, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Bool, "Remove Frame", this, &adremFrame, 1, HandlePropChange));
        if (mAnimation[mCurrentAnimation].mFrame.size() > 0)
        {
            outProps.push_back(Property(DatumType::Integer, "Active Frame", this, &mFrame, 1, HandlePropChange, NULL_DATUM, int32_t(mAnimation[mCurrentAnimation].mFrame.size() - 1)));
        }
        for (uint32_t i = 0; i < mAnimation[mCurrentAnimation].mFrame.size(); ++i)
        {
            frameRef = mAnimation[mCurrentAnimation].mFrame;
            std::string frameName = std::string("Frame ") + std::to_string(i + 1);
            outProps.push_back(Property(DatumType::Asset, frameName, this, &frameRef[i], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        }
        outProps.push_back(Property(DatumType::Bool, "", this, &adremFrame, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Bool, "", this, &adremFrame, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Bool, "", this, &adremFrame, 1, HandlePropChange));

    }



}

