#pragma once
#include "../../Headers/Pipeline.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device& mDevice);
        Shader(Device& mDevice, std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass);
        Shader(Device& mDevice, std::vector<unsigned char>& vertShaderCode, std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass);
        
        Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig);
        Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig, std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);

        ~Shader();

    private:
        Device& aDevice;
        Pipeline* pipeline;
        VkPipelineLayout pipelineLayout {VK_NULL_HANDLE};
        std::vector<Descriptor*> descriptors;
    };
}