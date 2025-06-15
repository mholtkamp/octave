#include "Shader.h"
#include "Log.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

//using namespace spirv_cross;

VkShaderStageFlags GetShaderStageFlags(ShaderStage stage)
{
    VkShaderStageFlags flags = 0;

    switch (stage)
    {
    case ShaderStage::Vertex: flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; break;
    case ShaderStage::Fragment: flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; break;
    case ShaderStage::Compute: flags = VK_SHADER_STAGE_COMPUTE_BIT; break;
    default: OCT_ASSERT(0); break;
    }

    return flags;
}

Shader::Shader(const std::string& filePath, ShaderStage stage, const std::string& name)
{
    mName = name;
    mStage = stage;

    const char* data = nullptr;
    uint32_t size = 0;
    Stream stream;

    if (filePath != "")
    {
        stream.ReadFile(filePath.c_str(), true);

        data = stream.GetData();
        size = stream.GetSize();
    }

    if (data != nullptr && size > 0)
    {
        Create(data, size);
    }
    else
    {
        LogError("Failed to load shader code.");
    }

}

Shader::Shader(const char* data, uint32_t size, ShaderStage stage, const std::string& name)
{
    mName = name;
    mStage = stage;

    Create(data, size);
}

Shader::~Shader()
{
    VkDevice device = GetVulkanDevice();

    if (mModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device, mModule, nullptr);
        mModule = VK_NULL_HANDLE;
    }
}

void Shader::Create(const char* data, uint32_t size)
{
    if (mModule != VK_NULL_HANDLE)
    {
        LogError("Shader::Create() was called on a pre-existing shader.");
        return;
    }

    if (data == nullptr || size == 0)
    {
        LogError("Shader::Create() was called with invalid data/size");
        return;
    }

    VkDevice device = GetVulkanDevice();
    VkShaderModuleCreateInfo ciModule = {};
    ciModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ciModule.codeSize = size;

    std::vector<uint32_t> codeLong(size / sizeof(uint32_t));
    memcpy(codeLong.data(), data, size);
    ciModule.pCode = codeLong.data();

    if (vkCreateShaderModule(device, &ciModule, nullptr, &mModule) != VK_SUCCESS)
    {
        LogError("Failed to create shader module");
        OCT_ASSERT(0);
    }

    if (mName != "")
    {
        SetDebugObjectName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)mModule, mName.c_str());
    }

    // Use Spirv Cross to determine descriptor set layouts and save them on this object.
    //spirv_cross::Compiler comp(codeLong.data(), codeLong.size());
    spirv_cross::Compiler comp(codeLong);
    spirv_cross::ShaderResources resources = comp.get_shader_resources();

    auto addBinding = [&](const spirv_cross::Resource& res, VkDescriptorType descType)
    {
        uint32_t set = comp.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding = comp.get_decoration(res.id, spv::DecorationBinding);

        const spirv_cross::SPIRType type = comp.get_type(res.type_id);
        uint32_t count = 1;

        if (type.array.size() > 0)
        {
            // Only supporting 1 dimensional arrays.
            count = type.array[0];
        }

        // We manually setup the global descriptor set. See below.
        if (set == 0)
        {
            return;
        }

        if (set < MAX_BOUND_DESCRIPTOR_SETS)
        {
            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.descriptorCount = count;
            layoutBinding.descriptorType = descType;
            layoutBinding.binding = binding;
            layoutBinding.pImmutableSamplers = nullptr;

            // We can set this later when creating the VkDescriptorSetLayout.
            // If graphics, just set stages to Vertex + Fragment. For compute, just compute stage.
            // We need to merge the descriptors for vertex and fragment I think.
            //layoutBinding.stageFlags = GetShaderStageFlags(mStage);
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

            mDescriptorBindings[set].push_back(layoutBinding);
        }
        else
        {
            LogWarning("Descriptor set above MAX_BOUND_DESCRIPTOR_SETS encountered.");
        }
    };

    for (uint32_t i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        addBinding(resources.uniform_buffers[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    }

    for (uint32_t i = 0; i < resources.sampled_images.size(); ++i)
    {
        addBinding(resources.sampled_images[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }

    for (uint32_t i = 0; i < resources.storage_buffers.size(); ++i)
    {
        addBinding(resources.storage_buffers[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }

    for (uint32_t i = 0; i < resources.storage_images.size(); ++i)
    {
        addBinding(resources.storage_images[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    }

    // Setup global descriptor set.
    {
        VkShaderStageFlags allStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

        // Global Uniform Buffer at 0.0
        VkDescriptorSetLayoutBinding globalUniformBinding = {};
        globalUniformBinding.descriptorCount = 1;
        globalUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        globalUniformBinding.binding = 0;
        globalUniformBinding.pImmutableSamplers = nullptr;
        globalUniformBinding.stageFlags = allStages;
        mDescriptorBindings[0].push_back(globalUniformBinding);

        // Shadow Map image at 0.1
        VkDescriptorSetLayoutBinding shadowMapBinding = {};
        shadowMapBinding.descriptorCount = 1;
        shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadowMapBinding.binding = 1;
        shadowMapBinding.pImmutableSamplers = nullptr;
        shadowMapBinding.stageFlags = allStages;
        mDescriptorBindings[0].push_back(shadowMapBinding);
    }

    // We will access these descriptor set layouts when creating a pipeline using this shader.
}
