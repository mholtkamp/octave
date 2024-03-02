#pragma once

#include "PostProcess/BlurPass.h"
#include "PostProcess/TonemapPass.h"
#include "VulkanTypes.h"
#include "Renderer.h"

class PostProcessChain
{
public:

    PostProcessChain();

    void Create();
    void Destroy();
    void Resize();
    void Render();

    void GatherProperties(std::vector<Property>& props);

protected:

    // Passes
    BlurPass mBlurPass;
    TonemapPass mTonemapPass;

    PostProcessPass* mPasses[(uint32_t)PostProcessPassId::Count] = {};

    // Second scene color image to ping-pong between
    Image* mAuxSceneColor = nullptr;
};
