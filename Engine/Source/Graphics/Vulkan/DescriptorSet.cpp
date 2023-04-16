#if API_VULKAN

#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "Renderer.h"

#include "Assertion.h"

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
            OCT_ASSERT(0);
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
    OCT_ASSERT(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::Image;
    mBindings[binding].mObject = image;
    mBindings[binding].mImageArray.clear();
    MarkDirty();
}

void DescriptorSet::UpdateImageArrayDescriptor(int32_t binding, const std::vector<Image*>& imageArray)
{
    OCT_ASSERT(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::ImageArray;
    mBindings[binding].mObject = nullptr;
    mBindings[binding].mImageArray = imageArray;
}

void DescriptorSet::UpdateUniformDescriptor(int32_t binding, UniformBuffer* uniformBuffer)
{
    OCT_ASSERT(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::Uniform;
    mBindings[binding].mObject = uniformBuffer;
    mBindings[binding].mImageArray.clear();
    MarkDirty();
}

void DescriptorSet::UpdateStorageBufferDescriptor(int32_t binding, Buffer* storageBuffer)
{
    OCT_ASSERT(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::StorageBuffer;
    mBindings[binding].mObject = storageBuffer;
    mBindings[binding].mImageArray.clear();
    MarkDirty();
}

void DescriptorSet::UpdateStorageImageDescriptor(int32_t binding, Image* storageImage)
{
    OCT_ASSERT(binding >= 0 && binding < MAX_DESCRIPTORS_PER_SET);
    mBindings[binding].mType = DescriptorType::StorageImage;
    mBindings[binding].mObject = storageImage;
    mBindings[binding].mImageArray.clear();
    MarkDirty();
}

void DescriptorSet::Bind(VkCommandBuffer cb, uint32_t index, VkPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint)
{
    uint32_t frameIndex = GetFrameIndex();

    if (mDirty[frameIndex])
    {
        RefreshBindings(frameIndex);
        mDirty[frameIndex] = false;
    }

    vkCmdBindDescriptorSets(
        cb,
        bindPoint,
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
        if (binding.mObject != nullptr || binding.mType == DescriptorType::ImageArray)
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
            else if (binding.mType == DescriptorType::ImageArray)
            {
                static std::vector<VkDescriptorImageInfo> sDescImageInfo;
                sDescImageInfo.resize(binding.mImageArray.size());

                if (binding.mImageArray.size() > 0)
                {
                    for (uint32_t i = 0; i < binding.mImageArray.size(); ++i)
                    {
                        VkDescriptorImageInfo& imageInfo = sDescImageInfo[i];
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfo.imageView = binding.mImageArray[i]->GetView();
                        imageInfo.sampler = binding.mImageArray[i]->GetSampler();
                    }

                    VkWriteDescriptorSet descriptorWrite = {};
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = mDescriptorSets[frameIndex];
                    descriptorWrite.dstBinding = i;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount = (uint32_t)binding.mImageArray.size();
                    descriptorWrite.pImageInfo = sDescImageInfo.data();

                    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                }
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
            else if (binding.mType == DescriptorType::StorageBuffer)
            {
                Buffer* buffer = reinterpret_cast<Buffer*>(binding.mObject);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->Get();
                bufferInfo.range = buffer->GetSize();
                bufferInfo.offset = 0;

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSets[frameIndex];
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
            else if (binding.mType == DescriptorType::StorageImage)
            {
                Image* image = reinterpret_cast<Image*>(binding.mObject);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo.imageView = image->GetView();
                imageInfo.sampler = image->GetSampler();

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSets[frameIndex];
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }
}

#endif