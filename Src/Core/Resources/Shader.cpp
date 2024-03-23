#include "Headers/Shader.hpp"

using namespace AnA;

Shader::Shader(Device& mDevice) : aDevice{mDevice}
{
    
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
        vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), pipelineLayout, nullptr);
}