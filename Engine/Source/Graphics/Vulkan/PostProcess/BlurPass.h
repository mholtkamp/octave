#pragma once

#include "Graphics/Vulkan/PostProcess/PostProcessPass.h"

struct BlurUniforms
{
    int32_t mHorizontal = 0;
    int32_t mNumSamples = 5;
    float mBlurSize = 0.05f;
    int32_t mPad0 = 1337;
};

class BlurPass : public PostProcessPass
{
public:

    virtual void Create() override;
    virtual void Destroy() override;

    virtual void Render(Image* input, Image* output) override;

    virtual void GatherProperties(std::vector<Property>& props) override;

protected:

    Image* mHalfBlurImage = nullptr;
    BlurUniforms mUniforms = {};
};
