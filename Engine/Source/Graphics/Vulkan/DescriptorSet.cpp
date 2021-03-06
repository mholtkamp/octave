#if API_VULKAN

#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "Renderer.h"

#include <assert.h>

using namespace std;

DescriptorSet::DescriptorSet(VkDescriptorSetLayout layout)
{
    VkDevice device = GetVulkanDevice();
    VkDescriptorPool pool = GetVulkanContext()->GetDescriptorPool();

    VkDescriptorSetLayout layouts[] = { layout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        if (vkAllocateDescriptorSets(device, &allocInfo, &mDescriptorSets[i]) != VK_SUCCESS)
        {
            LogError("Failed to allocate descriptor set");
            assert(0);
        }
    }
}

DescriptorSet::~DescriptorSet()
{
    VkDevice device = GetVulkanDevice();
    VkDescriptorPool pool = GetVulkanContext()->GetDescriptorPool();

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        vkFreeDescriptorSets(device, pool, 1, &mDescriptorSets[i]);
        mDescriptorSets[i] = VK_NULL_HANDLE;
    }
}

void DescriptorSet::UpdateImageDescriptor(int32_t binding, Image* image)
{
    assert(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::Image;
    mBindings[binding].mObject = image;
    MarkDirty();
}

void DescriptorSet::UpdateUniformDescriptor(int32_t binding, UniformBuffer* uniformBuffer)
{
    assert(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::Uniform;
    mBindings[binding].mObject = uniformBuffer;
    MarkDirty();
}

void DescriptorSet::Bind(VkCommandBuffer cb, uint32_t index, VkPipelineLayout pipelineLayout)
{
    uint32_t frameIndex = GetFrameIndex();

    if (mDirty[frameIndex])
    {
        RefreshBindings(frameIndex);
        mDirty[frameIndex] = false;
    }

    vkCmdBindDescriptorSets(
        cb,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        index,
        1,
        &mDescriptorSets[frameIndex],
        0,
        nullptr);
}

VkDescriptorSet DescriptorSet::Get()
{
    uint32_t frameIndex = GetVulkanContext()->GetFrameIndex();
    return mDescriptorSets[frameIndex];
}

VkDescriptorSet DescriptorSet::Get(uint32_t frameIndex)
{
    return mDescriptorSets[frameIndex];
}

void DescriptorSet::MarkDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mDirty[i] = true;
    }
}

void DescriptorSet::RefreshBindings(uint32_t frameIndex)
{
    VkDevice device = GetVulkanDevice();

    for (uint32_t i = 0; i < MAX_DESCRIPTORS_PER_SET; ++i)
    {
        DescriptorBinding& binding = mBindings[i];
        if (binding.mObject != nullptr)
        {
            if (binding.mType == DescriptorType::Image)
            {
                Image* image = reinterpret_cast<Image*>(binding.mObject);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = image->GetView();
                imageInfo.sampler = image->GetSampler();

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSets[frameIndex];
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
            else if (binding.mType == DescriptorType::Uniform)
            {
                UniformBuffer* uniformBuffer = reinterpret_cast<UniformBuffer*>(binding.mObject);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = uniformBuffer->Get(frameIndex);
                bufferInfo.range = uniformBuffer->GetBuffer(frameIndex)->GetSize();
                bufferInfo.offset = 0;

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSets[frameIndex];
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }
}

#endif