#pragma once
#include "../../Headers/Pipeline.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device& mDevice);
        Shader(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass);
        Shader(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass);
        
        Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig);
        Shader(Device& mDevice, Pipeline::PipelineConfig pipelineConfig, std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);

        ~Shader();

        Pipeline* GetPipeline() const;
        const VkPipelineLayout& GetPipelineLayout() const;
        const std::vector<Descriptor*>& GetDescriptors() const;
        std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets();
    private:
        Device& aDevice;
        Pipeline* pipeline;
        VkPipelineLayout pipelineLayout {VK_NULL_HANDLE};
        void createPipelineLayout(std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);
        void createDescriptors(std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);
        std::vector<Descriptor*> descriptors;
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;
    };
}