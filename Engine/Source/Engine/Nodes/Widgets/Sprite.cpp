#include "Nodes/Widgets/Sprite.h"

#include "AssetManager.h"

FORCE_LINK_DEF(Sprite);
DEFINE_NODE(Sprite, Quad);

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


void Sprite::AdvanceFrame()
{
    ++mFrame;
    if (mFrame >= mCurrentAnimation->mFrame.size())
    {
        mFrame = 0;
        if (!GetLoop())
        {
            SetPlay(false);
            return;
        }
    }
    SetTexture(mCurrentAnimation->mFrame[mFrame].Get<Texture>());

}

void Sprite::AddAnimation(std::string animationName)
{
    LogDebug("test");
    if (animationName == "")
    {
        int32_t i = 1;
        bool flag = false;
        while (!flag)
        {
            std::string checkName = "Animation ";
            checkName = checkName +  std::to_string(i);
            bool bflag = false;
            for (SpriteAnimation* animation : mAnimation)
            {
                if (animation->mName == checkName) bflag = true;
            }
            if (!bflag)
            {
                animationName = checkName;
                flag = true;
            }
            ++i;
        }
    }
    LogDebug("testa");
    for (SpriteAnimation* animation : mAnimation)
    {
        if (animationName == animation->mName)
        {
            LogDebug("Animations cannot share names");
            return;
        }
    }
    LogDebug("testb");
    SpriteAnimation* animation;
    LogDebug("testc");
    LogDebug(animationName.c_str());
    animation->mName = animationName;
    LogDebug("testd");
    animation->loop = true;
    LogDebug("teste");
    mAnimation.push_back(animation);
    LogDebug("testf");
    if (mCurrentAnimation == nullptr) SetAnimation(animationName);
    LogDebug("testg");

}
void Sprite::AddFrame(class Texture* texture, int32_t frameIndex, bool insert) //-1 frame means add to back, insert will take frame at index and push it back
{
    if (frameIndex == -1) mCurrentAnimation->mFrame.push_back(texture);
    else if (frameIndex >= mCurrentAnimation->mFrame.size()) mCurrentAnimation->mFrame.push_back(texture);
    else
    {

        if (!insert)
        {
            mCurrentAnimation->mFrame[frameIndex] = texture;
        }
        else
        {
            mCurrentAnimation->mFrame.push_back(nullptr);
            for (uint32_t i = mCurrentAnimation->mFrame.size() - 1; i > frameIndex; --i)
            {
                mCurrentAnimation->mFrame[i] = mCurrentAnimation->mFrame[i - 1];
            }
            mCurrentAnimation->mFrame[frameIndex] = texture;
        }

        if (mFrame == frameIndex) SetTexture(mCurrentAnimation->mFrame[mFrame].Get<Texture>());
    }
}
void Sprite::AddEmptyFrame(int32_t frameIndex, bool insert)
{
    AddFrame(nullptr, frameIndex, insert);
}

void Sprite::RemoveAnimation(uint32_t animationIndex)
{
    mAnimation.erase(mAnimation.begin() + animationIndex);
}
void Sprite::RemoveFrame(uint32_t frameIndex)
{
    mCurrentAnimation->mFrame.erase(mCurrentAnimation->mFrame.begin() + frameIndex);
}

void Sprite::SetAnimation(std::string animationName)
{
    for (SpriteAnimation* animation : mAnimation)
    {
        if (animation->mName == animationName)
        {
            mCurrentAnimation = animation;
            mFrame = mCurrentAnimation->mFrame.size(); //makes it so animations start on their first frame;
            AdvanceFrame();
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
        mFrame = mCurrentAnimation->mFrame.size(); //makes it so animations start on their first frame;
        AdvanceFrame();
        return;
    }
}
void Sprite::SetFrame(uint32_t frameIndex)
{
    mFrame = frameIndex - 1;
    AdvanceFrame();
}
void Sprite::SetFPS(float fps)
{
    mFPS = fps;
}
void Sprite::SetPlay(bool play)
{
    mPlaying = play;
}
void Sprite::SetLoop(bool loop)
{
    mCurrentAnimation->loop = loop;
}
void Sprite::SetAnimationName(std::string name)
{
    mCurrentAnimation->mName = name;
}

Texture* Sprite::GetFrame(uint32_t frameIndex)
{
    if (frameIndex < mCurrentAnimation->mFrame.size()) return mCurrentAnimation->mFrame[frameIndex].Get<Texture>();
}
uint32_t Sprite::GetAnimationLength(std::string animationName)
{
    if (animationName == "") return mCurrentAnimation->mFrame.size();
    for (SpriteAnimation* animation : mAnimation)
    {
        if (animationName == animation->mName) return animation->mFrame.size();
    }
    LogDebug("No animation '%s' found", animationName);
}
std::string Sprite::GetAnimationName()
{
    return mCurrentAnimation->mName;
}
bool Sprite::IsPlaying()
{
    return mPlaying;
}
bool Sprite::GetLoop()
{
    return mCurrentAnimation->loop;
}
uint32_t Sprite::GetFrameIndex()
{
    return mFrame;
}
uint32_t Sprite::GetNumAnimations()
{
    return mAnimation.size();
}
std::vector<SpriteAnimation*> Sprite::GetAnimations()
{
    return mAnimation;
}
