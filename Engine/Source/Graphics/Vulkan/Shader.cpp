#include "Shader.h"
#include "Log.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

Shader::Shader(const char* filePath, const char* name)
{
    mName = name;

    const char* data = nullptr;
    uint32_t size = 0;

    if (filePath != "")
    {
        Stream stream;
        stream.ReadFile(filePath, true);

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

Shader::Shader(const char* data, uint32_t size, const char* name)
{
    mName = name;
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

    for (uint32_t i = 0; i < MAX_BOUND_DESCRIPTOR_SETS; ++i)
    {
        if (mDescriptorSetLayouts[i] != VK_NULL_HANDLE)
        {

            vkDestroyDescriptorSetLayout(device, mDescriptorSetLayouts[i], nullptr);
            mDescriptorSetLayouts[i] = VK_NULL_HANDLE;
        }
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

    std::vector<uint32_t> codeLong(size / sizeof(uint32_t) + 1);
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

    // NEXT! Use SPIR-V Reflect to determine descriptor set layouts and save them on this object.
}
