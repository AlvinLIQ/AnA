#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

static VkPipelineLayout* pDefaultPipelineLayout = nullptr;

Shader::Shader(Device& mDevice) : aDevice{mDevice}
{
    
}

Shader::Shader(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass) : aDevice{mDevice}
{

}

Shader::Shader(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass) : aDevice{mDevice}
{
    auto descriptorConfigs = Resource::ResourceManager::GetCurrent()->GetDefault();
    for (auto& descriptorConfig : descriptorConfigs)
        descriptors.push_back(new Descriptor(mDevice, descriptorConfig));
    if (pDefaultPipelineLayout == nullptr)
    {
        createPipelineLayout(descriptorConfigs);
    }
    else
    {
        pipelineLayout = *pDefaultPipelineLayout;
    }
    pipeline = new Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig) : aDevice{mDevice}
{
    pipeline = new Pipeline(mDevice, pipelineConfig);
}

Shader::Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig, std::vector<Descriptor::DescriptorConfig>& descriptorConfigs) : aDevice{mDevice}
{
    pipeline = new Pipeline(mDevice, pipelineConfig);
    for (auto& descriptorConfig : descriptorConfigs)
        descriptors.push_back(new Descriptor(mDevice, descriptorConfig));
}

Shader::~Shader()
{
    delete pipeline;
    for (auto& descriptor : descriptors)
        delete descriptor;
    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void Shader::createPipelineLayout(std::vector<Descriptor::DescriptorConfig>& descriptorConfigs)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptors.size());
    for (int i = 0; i < descriptors.size(); i++)
    {
        descriptorSetLayouts[i] = descriptors[i]->GetLayout();
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptors.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    VkPushConstantRange range;
    range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    range.offset = 0;
    range.size = sizeof(ObjectPushConstantData);
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;

    if (vkCreatePipelineLayout(aDevice.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}