#pragma once

#include "Nodes/Widgets/Quad.h"

struct SpriteAnimation {
    std::string mName;
    std::vector<TextureRef> mFrame;
    bool loop;
};

class Sprite : public Quad
{
public:

    DECLARE_NODE(Sprite, Quad);

    friend class Button;

    Sprite();
    virtual ~Sprite();

    virtual void Create() override;
    //virtual void Destroy() override;

    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

    void TickCommon(float deltaTime);

    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void LoadStream(Stream& stream, Platform platform, uint32_t version) override;


    virtual void GatherProperties(std::vector<Property>& outProps) override;
    //void GatherSpriteProperties(std::vector<Property>& outProps);

    void AddAnimation(std::string animationName = "");
    void AddFrame(class Texture* texture, int32_t frameIndex = -1, bool insert = false, int32_t animationIndex = -1); //-1 frame means add to back, insert will take frame at index and push it back
    void AddEmptyFrame(int32_t frameIndex = -1, bool insert = false, int32_t animationIndex = -1);

    void RemoveAnimation(uint32_t animationIndex);
    void RemoveFrame(uint32_t frameIndex, int32_t animationIndex = -1);

    void SetAnimation(std::string animationName);
    void SetAnimation(uint32_t animationIndex);
    void SetFrame(uint32_t frameIndex = 0);
    void SetFPS(float fps);
    void SetPlay(bool play = true);
    void SetLoop(bool loop = true);
    void SetAnimationName(std::string name, uint32_t animationIndex = -1);
    void SetAnimationData(int32_t index, const SpriteAnimation& data);

    Texture* GetFrame(uint32_t frameIndex);
    uint32_t GetAnimationLength(std::string animationName = "");
    std::string GetAnimationName();
    bool IsPlaying();
    bool GetLoop();
    int32_t GetFrameIndex(uint32_t animationIndex);
    uint32_t GetNumAnimations();
    std::vector<SpriteAnimation> GetAnimations();
    int32_t GetAnimationIndex();
    SpriteAnimation GetCurrentAnimation();
    SpriteAnimation GetAnimation(uint32_t animationIndex);
    uint32_t GetNumFrames(uint32_t animationIndex);




protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void AdvanceFrame();



    std::vector<SpriteAnimation> mAnimation;
    SpriteAnimation mCurrentAnimation;
    bool mPlaying;
    int32_t mFrame;
    float mFPS;
    float mFrameTime;

};
