#pragma once
#include "../../Headers/Pipeline.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device* mDevice);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
            std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);

        Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig);
        Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig, std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& shader) noexcept : aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, descriptors{shader.descriptors}, descriptorSets{shader.descriptorSets}
        {
            shader.pipeline = nullptr;
            shader.pipelineLayout = VK_NULL_HANDLE;
            shader.descriptors.clear();
            shader.descriptorSets.clear();
        }
        Shader& operator=(Shader&& shader) noexcept
        {
            if (&shader != this)
            {
                aDevice = shader.aDevice;
                pipeline = shader.pipeline;
                pipelineLayout = shader.pipelineLayout;
                descriptors = shader.descriptors;
                descriptorSets = shader.descriptorSets;

                shader.pipeline = nullptr;
                shader.pipelineLayout = VK_NULL_HANDLE;
                shader.descriptors.clear();
                shader.descriptorSets.clear();
            }
            return *this;
        }
        ~Shader();

        Pipeline* GetPipeline() const;
        const VkPipelineLayout& GetPipelineLayout() const;
        const std::vector<Descriptor*>& GetDescriptors() const;
        std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets();
    private:
        Device* aDevice;
        Pipeline* pipeline;
        VkPipelineLayout pipelineLayout {VK_NULL_HANDLE};
        void createPipelineLayout(std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);
        void createDescriptors(std::vector<Descriptor::DescriptorConfig>& descriptorConfigs);
        std::vector<Descriptor*> descriptors;
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;
    };
}