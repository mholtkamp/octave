#include "Nodes/Widgets/Sprite.h"

#include "AssetManager.h"

Sprite::Sprite() :
    mFrame(0),
    mCurrentAnimation(nullptr)
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
    if (GTE && IsPlaying() && (mCurrentAnimation != nullptr))
    {
        mFrameTime += deltaTime;
        if (mFrameTime >= (1 / mFPS))
        {
            AdvanceFrame();
            mFrameTime = 0.0f;
        }
    }
}

void Sprite::SetFPS(float fps)
{
    mFPS = fps;
}

bool Sprite::IsPlaying()
{
    return mPlaying;
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
    SetTexture(mCurrentAnimation.mFrame[mFrame]);

}

void AddAnimation(std::string animationName)
{
    if (animationName == "")
    {
        LogDebug("Animation name cannot be empty");
        return;
    }
    for (Animation animation : mAnimation)
    {
        if (animationName == animation.mName)
        {
            LogDebug("Animations cannot share names");
            return;
        }
    }
    Animation animation;
    animation.mName = animationName;
    animation.loop = true;
    mAnimation.push_back(Animation animation);
    if (mCurrentAnimation == nullptr) SetAnimation(animationName);

}
void AddFrame(class Texture* texture, int32_t frameIndex, bool insert) //-1 frame means add to back, insert will take frame at index and push it back
{
    if (frameIndex == -1) mCurrentAnimation.mFrame.push_back(texture);
    else if (frameIndex >= mCurrentAnimation.mFrame.size()) mCurrentAnimation.mFrame.push_back(texture);
    else
    {

        if (!insert)
        {
            mCurrentAnimation.mFrame[frameIndex] = texture;
        }
        else
        {
            mCurrentAnimation.mFrame.push_back(nullptr);
            for (uint32_t i = mCurrentAnimation.mFrame.size() - 1; i > frameIndex; --i)
            {
                mCurrentAnimation.mFrame[i] = mCurrentAnimation.mFrame[i - 1];
            }
            mCurrentAnimation.mFrame[frameIndex] = texture;
        }

        if (mFrame == frameIndex) SetTexture(mCurrentAnimation.mFrame[mFrame]);
    }
}
void AddEmptyFrame(int32_t frameIndex, bool insert)
{
    AddFrame(nullptr, frameIndex, insert);
}

void RemoveAnimation(std::string animationName)
{

}
void RemoveFrame(uint32_t frameIndex)
{

}

void SetAnimation(std::string animationName)
{

}
void SetAnimation(uint32_t animationIndex)
{

}
