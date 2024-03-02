#pragma once

#include "Graphics/Vulkan/PostProcess/PostProcessPass.h"

struct TonemapUniforms
{
    int32_t mPathTracingEnabled = 0;
    int32_t mPad0 = 1337;
    int32_t mPad1 = 1337;
    int32_t mPad2 = 1337;
};

class TonemapPass : public PostProcessPass
{
public:

    virtual void Create() override;
    virtual void Destroy() override;

    virtual void Render(Image* input, Image* output) override;

    virtual bool IsEnabled() const;

    virtual void GatherProperties(std::vector<Property>& props) override;

protected:

    TonemapUniforms mUniforms;
};
