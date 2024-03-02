#pragma once

#include "Graphics/Vulkan/Image.h"
#include "Property.h"

#include <string>

class PostProcessPass
{
public:

    virtual void Create();
    virtual void Destroy();
    virtual void Resize();
    virtual void Render(Image* Input, Image* Output);

    const std::string& GetName() const;

    virtual bool IsEnabled() const;
    void SetEnabled(bool enable);

    virtual void GatherProperties(std::vector<Property>& props);

protected:

    std::string mName;
    bool mEnabled = false;
};