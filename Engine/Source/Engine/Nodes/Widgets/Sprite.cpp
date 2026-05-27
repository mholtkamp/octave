#include "Nodes/Widgets/Sprite.h"

#include "AssetManager.h"

FORCE_LINK_DEF(Sprite);
DEFINE_NODE(Sprite, Quad);

Sprite::Sprite() :
    mFrame(0)
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
    if (GTE && IsPlaying() && (mCurrentAnimation.mName.size() > 0))
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
    stream.WriteInt32(GetAnimationIndex());
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
    int32_t curAnimation = stream.ReadInt32();
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

         if (i == curAnimation) mCurrentAnimation = mAnimation[i];
    }
}


void Sprite::AdvanceFrame()
{
    ++mFrame;
    if (mFrame >= mCurrentAnimation.mFrame.size())
    {
        mFrame = 0;
        if (!GetLoop())
        {
            SetPlay(false);
            return;
        }
    }
    //this is where it crashes rn
    SetTexture(mCurrentAnimation.mFrame[mFrame].Get<Texture>());

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
    if (mCurrentAnimation.mName.size() == 0) SetAnimation(animationName);
    MarkDirty();

}
void Sprite::AddFrame(class Texture* texture, int32_t frameIndex, bool insert, int32_t animationIndex) //-1 frame means add to back, insert will take frame at index and push it back
{
    SpriteAnimation animation;
    if (animationIndex == -1)
    {
        animation = mCurrentAnimation;
        animationIndex = GetAnimationIndex();
    }
    else animation = mAnimation[animationIndex];

    if (frameIndex == -1) animation.mFrame.push_back(texture);
    else if (frameIndex >= animation.mFrame.size()) animation.mFrame.push_back(texture);
    else
    {

        if (!insert)
        {
            animation.mFrame[frameIndex] = texture;
        }
        else
        {
            animation.mFrame.push_back(nullptr);
            for (uint32_t i = animation.mFrame.size() - 1; i > frameIndex; --i)
            {
                animation.mFrame[i] = animation.mFrame[i - 1];
            }
            animation.mFrame[frameIndex] = texture;
        }

        if (mFrame == frameIndex) SetTexture(animation.mFrame[mFrame].Get<Texture>());
        mAnimation[animationIndex].mFrame = animation.mFrame;
        SetAnimation(animationIndex);
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
    if (animationIndex < 0) mCurrentAnimation.mFrame.erase(mCurrentAnimation.mFrame.begin() + frameIndex);
    else mAnimation[animationIndex].mFrame.erase(mAnimation[animationIndex].mFrame.begin() + frameIndex);
    MarkDirty();
}

void Sprite::SetAnimation(std::string animationName)
{
    for (SpriteAnimation animation : mAnimation)
    {
        if (animation.mName == animationName)
        {
            mCurrentAnimation = animation;
            mCurrentAnimation.mFrame = animation.mFrame;
            mFrame = mCurrentAnimation.mFrame.size(); //makes it so animations start on their first frame;
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
        mCurrentAnimation = mAnimation[animationIndex];
        mCurrentAnimation.mFrame = mAnimation[animationIndex].mFrame;
        mFrame = mCurrentAnimation.mFrame.size(); //makes it so animations start on their first frame;
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
    mCurrentAnimation.loop = loop;
    MarkDirty();
}
void Sprite::SetAnimationName(std::string name, uint32_t animationIndex)
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
        /*mCurrentAnimation.mName = name;*/
        return;
    }
    if (animationIndex >= mAnimation.size()) return;
    mAnimation[animationIndex].mName = name;
    mCurrentAnimation.mName = name;
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

Texture* Sprite::GetFrame(uint32_t frameIndex)
{
    if (frameIndex < mCurrentAnimation.mFrame.size()) return mCurrentAnimation.mFrame[frameIndex].Get<Texture>();
}
uint32_t Sprite::GetAnimationLength(std::string animationName)
{
    if (animationName == "") return mCurrentAnimation.mFrame.size();
    for (SpriteAnimation animation : mAnimation)
    {
        if (animationName == animation.mName) return animation.mFrame.size();
    }
    LogDebug("No animation '%s' found", animationName);
}
std::string Sprite::GetAnimationName()
{
    return mCurrentAnimation.mName;
}
bool Sprite::IsPlaying()
{
    return mPlaying;
}
bool Sprite::GetLoop()
{
    return mCurrentAnimation.loop;
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
    for (uint32_t i = 0; i < mAnimation.size(); ++i)
    {
        if (mCurrentAnimation.mName == mAnimation[i].mName) return i;
    }
    return -1;
}
SpriteAnimation Sprite::GetCurrentAnimation()
{
    return mCurrentAnimation;
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

    SCOPED_CATEGORY("Sprite");

    outProps.push_back(Property(DatumType::Bool, "Play", this, &mPlaying));
    outProps.push_back(Property(DatumType::Float, "FPS", this, &mFPS));

}

