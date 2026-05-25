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


    //virtual void GatherProperties(std::vector<Property>& outProps) override;
    //void GatherSpriteProperties(std::vector<Property>& outProps);

    void AddAnimation(std::string animationName = "");
    void AddFrame(class Texture* texture, int32_t frameIndex = -1, bool insert = false); //-1 frame means add to back, insert will take frame at index and push it back
    void AddEmptyFrame(int32_t frameIndex = -1, bool insert = false);

    void RemoveAnimation(uint32_t animationIndex);
    void RemoveFrame(uint32_t frameIndex);

    void SetAnimation(std::string animationName);
    void SetAnimation(uint32_t animationIndex);
    void SetFrame(uint32_t frameIndex = 0);
    void SetFPS(float fps);
    void SetPlay(bool play = true);
    void SetLoop(bool loop = true);
    void SetAnimationName(std::string name);

    Texture* GetFrame(uint32_t frameIndex);
    uint32_t GetAnimationLength(std::string animationName = "");
    std::string GetAnimationName();
    bool IsPlaying();
    bool GetLoop();
    uint32_t GetFrameIndex();
    uint32_t GetNumAnimations();
    std::vector<SpriteAnimation*> GetAnimations();




protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void AdvanceFrame();



    std::vector<SpriteAnimation*> mAnimation;
    SpriteAnimation* mCurrentAnimation;
    bool mPlaying;
    uint32_t mFrame;
    float mFPS;
    float mFrameTime;

};
