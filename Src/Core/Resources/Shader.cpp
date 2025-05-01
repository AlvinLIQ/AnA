#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Shader::Shader(Device* mDevice) : aDevice{mDevice}
{
    
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, 
    VkRenderPass& renderPass, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    Resource::ResourceManager::GetCurrent()->GetDefaultDescriptorSetConfig(descriptorSetConfigs);
    
    createPipelineLayout(pushConstantSize);
    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}
{
    createPipelineLayout(pushConstantSize);

    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    const std::vector<unsigned char>& fragShaderCode, std::vector<Descriptor>& _descriptors,
    size_t actualDescriptorCount, VkDeviceSize pushConstantSize) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}
{
    createPipelineLayout(pushConstantSize);

    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout, fragShaderCode);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, 
    const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass,
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    Resource::ResourceManager::GetCurrent()->GetDefaultDescriptorSetConfig(descriptorSetConfigs);

    createPipelineLayout(pushConstantSize);
    pipeline = new Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout, vertexTopology);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, 
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}
{
    createPipelineLayout(pushConstantSize);

    pipeline = new Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout, vertexTopology);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, 
    const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}
{
    createPipelineLayout(pushConstantSize);

    pipeline = new Pipeline(mDevice, taskShaderCode, meshShaderCode, fragShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}
{
    pipeline = new Pipeline(mDevice, pipelineConfig);
}

Shader::~Shader()
{
    if (pipeline != nullptr)
        delete pipeline;

    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

Pipeline* Shader::GetPipeline() const
{
    return pipeline;
}
const VkPipelineLayout& Shader::GetPipelineLayout() const
{
    return pipelineLayout;
}

std::vector<std::vector<VkDescriptorSet>>& Shader::GetDescriptorSets()
{
    return descriptorSets;
}

void Shader::createPipelineLayout(VkDeviceSize pushConstantSize)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptorCount);
    if (descriptorCount)
    {
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < descriptorCount; i++)
        {
            auto& descriptor = descriptors->data()[i];
            descriptorSetLayouts[i] = descriptor.GetLayout();
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                descriptorSets[i].push_back(descriptor.GetSets().size() ? descriptor.GetSets()[i % (int)descriptor.GetSets().size()] :
                nullptr);
            }
        }
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorCount;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    
    if (pushConstantSize)
    {
        VkPushConstantRange range;
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        if (aDevice->MeshShaderSupport())
            range.stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT;
        range.offset = 0;
        range.size = pushConstantSize;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &range;
    }

    if (vkCreatePipelineLayout(aDevice->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

const std::vector<Descriptor>& Shader::GetDescriptors() const
{
    return *descriptors;
}