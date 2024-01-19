#include "VulkanTypes.h"
#include "Log.h"


bool PipelineState::operator==(const PipelineState& other) const
{
    bool equal = (memcmp(this, &other, sizeof(PipelineState)) == 0);
    return equal;
}

size_t PipelineState::Hash() const
{
    size_t ret = 0x1337;
    uint8_t* data = (uint8_t*)this;
    uint32_t size = sizeof(PipelineState);

    for (uint32_t i = 0; i < size; ++i)
    {
        size_t highOrder = ret & 0xf800000000000000;
        ret = ret << 5;
        ret = ret ^ (highOrder >> 59);
        ret = ret ^ data[i];
    }

    //LogDebug("PipelineState hash = %llX", ret);

    return ret;
}

