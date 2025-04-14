#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

static VkPipelineLayout* pDefaultPipelineLayout = nullptr;

Shader::Shader(Device* mDevice) : aDevice{mDevice}
{
    
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, 
    VkRenderPass& renderPass, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    Resource::ResourceManager::GetCurrent()->GetDefaultDescriptorSetConfig(descriptorSetConfigs);
    createDescriptors(descriptorSetConfigs);
    
    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    createDescriptors(descriptorSetConfigs);
    
    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    const std::vector<unsigned char>& fragShaderCode, std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs,
    VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    createDescriptors(descriptorSetConfigs);
    
    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout, fragShaderCode);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, 
    const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass,
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    Resource::ResourceManager::GetCurrent()->GetDefaultDescriptorSetConfig(descriptorSetConfigs);
    createDescriptors(descriptorSetConfigs);

    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout, vertexTopology);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs,
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    createDescriptors(descriptorSetConfigs);

    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    createDescriptors(descriptorSetConfigs);

    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(pushConstantSize);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, taskShaderCode, meshShaderCode, fragShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig, 
    std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs) : aDevice{mDevice}
{
    pipeline = new Pipeline(mDevice, pipelineConfig);
    createDescriptors(descriptorSetConfigs);
}

Shader::~Shader()
{
    delete pipeline;
    for (auto& descriptor : descriptors)
        delete descriptor;
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
const std::vector<Descriptor*>& Shader::GetDescriptors() const
{
    return descriptors;
}

std::vector<std::vector<VkDescriptorSet>>& Shader::GetDescriptorSets()
{
    return descriptorSets;
}

void Shader::createPipelineLayout(VkDeviceSize pushConstantSize)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptors.size());
    for (size_t i = 0; i < descriptors.size(); i++)
    {
        descriptorSetLayouts[i] = descriptors[i]->GetLayout();
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptors.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    
    if (pushConstantSize)
    {
        VkPushConstantRange range;
        range.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
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

void Shader::createDescriptors(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto& descriptorConfigs : descriptorSetConfigs)
    {
        auto descriptor = new Descriptor(aDevice, descriptorConfigs.data(),
            static_cast<uint32_t>(descriptorConfigs.size()), MAX_FRAMES_IN_FLIGHT);
        descriptors.push_back(descriptor);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (descriptor->GetSets().size())
            {
                descriptorSets[i].push_back(descriptor->GetSets()[i % (int)descriptor->GetSets().size()]);
            }
            else
            {
                descriptorSets[i].push_back(nullptr);
            }
        }
    }
}