#if API_VULKAN

#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "Renderer.h"

#include "Assertion.h"

// Referenced: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/

DescriptorSet DescriptorSet::Begin(const char* name)
{
    DescriptorSet retSet;
    retSet.mName = name;

    return retSet;
}

DescriptorSet& DescriptorSet::WriteImage(int32_t binding, Image* image)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindInfo.mObject = image;
    bindInfo.mBinding = binding;
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::WriteImageArray(int32_t binding, const std::vector<Image*>& imageArray)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindInfo.mObject = nullptr;
    bindInfo.mImageArray = imageArray;
    bindInfo.mBinding = binding;
    bindInfo.mCount = (uint32_t)imageArray.size();
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::WriteUniformBuffer(int32_t binding, UniformBuffer* uniformBuffer)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindInfo.mObject = uniformBuffer;
    bindInfo.mSize = (uint32_t)uniformBuffer->GetSize();
    bindInfo.mBinding = binding;
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::WriteUniformBuffer(int32_t binding, const UniformBlock& block)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindInfo.mObject = block.mUniformBuffer;
    bindInfo.mOffset = block.mOffset;
    bindInfo.mSize = block.mSize;
    bindInfo.mBinding = binding;
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::WriteStorageBuffer(int32_t binding, Buffer* storageBuffer)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindInfo.mObject = storageBuffer;
    bindInfo.mBinding = binding;
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::WriteStorageImage(int32_t binding, Image* storageImage)
{
    DescriptorBinding bindInfo;
    bindInfo.mType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindInfo.mObject = storageImage;
    bindInfo.mBinding = binding;
    mBindings.push_back(bindInfo);

    return *this;
}

DescriptorSet& DescriptorSet::Build()
{
    OCT_ASSERT(mDescriptorSet == VK_NULL_HANDLE);

    static std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.clear();

    for (uint32_t i = 0; i < mBindings.size(); ++i)
    {
        VkDescriptorSetLayoutBinding binding;
        binding.binding = mBindings[i].mBinding;
        binding.descriptorType = mBindings[i].mType;
        binding.descriptorCount = mBindings[i].mCount;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
        bindings.push_back(binding);
    }

    // Build or Reuse DescriptorSetLayout from LayoutCache.
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.pBindings = bindings.data();
    layoutInfo.bindingCount = (uint32_t)bindings.size();
    layoutInfo.flags = 0;

    VkDescriptorSetLayout layout = GetVulkanContext()->GetDescriptorLayoutCache().CreateLayout(&layoutInfo);

    // Allocate descriptor set from 
    mDescriptorSet = GetVulkanContext()->GetDescriptorPool().Allocate(layout);

    // Set debug name
    if (mName != nullptr)
    {
        SetDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)mDescriptorSet, mName);
    }

    // Update descriptor sets
    UpdateDescriptors();

    return *this;
}

void DescriptorSet::Bind(VkCommandBuffer cb, uint32_t index)
{
    Pipeline* pipeline = GetVulkanContext()->GetBoundPipeline();

    VkPipelineBindPoint bindPoint = pipeline->IsComputePipeline() ?
        VK_PIPELINE_BIND_POINT_COMPUTE :
        VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkPipelineLayout pipelineLayout = pipeline->GetPipelineLayout();

    static std::vector<uint32_t> dynOffsets;
    dynOffsets.clear();
    for (uint32_t i = 0; i < mBindings.size(); ++i)
    {
        // We use a dynamic buffer for uniforms
        if (mBindings[i].mType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        {
            dynOffsets.push_back(mBindings[i].mOffset);
        }
    }

    vkCmdBindDescriptorSets(
        cb,
        bindPoint,
        pipelineLayout,
        index,
        1,
        &mDescriptorSet,
        (uint32_t)dynOffsets.size(),
        dynOffsets.data());

    mFrameBuilt = GetFrameNumber();
}

VkDescriptorSet DescriptorSet::Get() const
{
    return mDescriptorSet;
}

VkDescriptorSetLayout DescriptorSet::GetLayout() const
{
    return mLayout;
}

void DescriptorSet::UpdateDescriptors()
{
    VkDevice device = GetVulkanDevice();

    // TODO: Merge all of the updates for this set into a single vkUpdateDescriptorSets() call.
    // Might want to store VkDescriptorImageInfo and VkDescriptorBufferInfo on Image and Buffer resources.
    // And we could possibly just use a static std::vector<> for the VkWriteDescriptorSet structs.
    for (uint32_t i = 0; i < mBindings.size(); ++i)
    {
        DescriptorBinding& binding = mBindings[i];
        if (binding.mObject != nullptr || binding.mImageArray.size() > 0)
        {
            if (binding.mType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                if (binding.mImageArray.size() == 0)
                {
                    Image* image = reinterpret_cast<Image*>(binding.mObject);

                    VkDescriptorImageInfo imageInfo = {};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = image->GetView();
                    imageInfo.sampler = image->GetSampler();

                    VkWriteDescriptorSet descriptorWrite = {};
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = mDescriptorSet;
                    descriptorWrite.dstBinding = i;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pImageInfo = &imageInfo;

                    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                }
                else
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
                        descriptorWrite.dstSet = mDescriptorSet;
                        descriptorWrite.dstBinding = i;
                        descriptorWrite.dstArrayElement = 0;
                        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrite.descriptorCount = (uint32_t)binding.mImageArray.size();
                        descriptorWrite.pImageInfo = sDescImageInfo.data();

                        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                    }
                }
            }
            else if (binding.mType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
            {
                UniformBuffer* uniformBuffer = reinterpret_cast<UniformBuffer*>(binding.mObject);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = uniformBuffer->Get();
                bufferInfo.range = binding.mSize;
                bufferInfo.offset = 0;

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSet;
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
            else if (binding.mType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
            {
                Buffer* buffer = reinterpret_cast<Buffer*>(binding.mObject);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->Get();
                bufferInfo.range = buffer->GetSize();
                bufferInfo.offset = 0;

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSet;
                descriptorWrite.dstBinding = i;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
            else if (binding.mType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            {
                Image* image = reinterpret_cast<Image*>(binding.mObject);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo.imageView = image->GetView();
                imageInfo.sampler = image->GetSampler();

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mDescriptorSet;
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