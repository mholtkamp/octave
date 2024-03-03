#pragma once

#include "Graphics/Vulkan/PostProcess/PostProcessPass.h"

#define BLUR_MAX_SAMPLES 32

struct BlurUniforms
{
    int32_t mHorizontal = 0;
    int32_t mNumSamples = 5;
    float mBlurSize = 0.05f;
    float mSigmaRatio = 0.25f;

    int32_t mBoxBlur = 1;
    int32_t mPad0 = 1337;
    int32_t mPad1 = 1337;
    int32_t mPad2 = 1337;

    glm::vec4 mGaussianWeights[BLUR_MAX_SAMPLES] = { };
};

class BlurPass : public PostProcessPass
{
public:

    virtual void Create() override;
    virtual void Destroy() override;

    virtual void Render(Image* input, Image* output) override;

    virtual void GatherProperties(std::vector<Property>& props) override;

protected:

    Image* mXBlurImage = nullptr;
    BlurUniforms mUniforms = {};
};
