#pragma once

#include "Nodes/Widgets/Quad.h"

class Sprite : public Quad
{
public:

    DECLARE_NODE(Sprite, Quad);

    friend class Button;

    Sprite();
    virtual ~Sprite();

    virtual void Create() override;
    virtual void Destroy() override;


    virtual void GatherProperties(std::vector<Property>& outProps) override;
    void GatherSpriteProperties(std::vector<Property>& outProps);



protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    struct Animation {
        std::string mName;
        std::vector<TextureRef> mTexture;
    };

    std::vector<Animation> mAnimation;

};
