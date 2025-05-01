#pragma once
#include "../../Headers/Pipeline.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device* mDevice);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, VkDeviceSize pushConstantSize = 0);
        //Depth Test
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, std::vector<Descriptor>& _descriptors,
            size_t actualDescriptorCount, VkDeviceSize pushConstantSize = 0);
        //Depth Test
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, const std::vector<unsigned char>& fragShaderCode, 
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, VkDeviceSize pushConstantSize = 0);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass,
            const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkDeviceSize pushConstantSize = 0);
        Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, 
            const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkDeviceSize pushConstantSize = 0);
        Shader(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, VkDeviceSize pushConstantSize = 0);

        Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig);
        Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig, std::vector<Descriptor>& descriptors, size_t actualDescriptorCount);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& shader) noexcept : aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, descriptorSets{shader.descriptorSets}
        {
            shader.pipeline = nullptr;
            shader.pipelineLayout = VK_NULL_HANDLE;
            shader.descriptorSets.clear();
        }
        Shader& operator=(Shader&& shader) noexcept
        {
            if (&shader != this)
            {
                Shader::~Shader();
                aDevice = shader.aDevice;
                pipeline = shader.pipeline;
                pipelineLayout = shader.pipelineLayout;
                descriptorSets = shader.descriptorSets;

                shader.pipeline = nullptr;
                shader.pipelineLayout = VK_NULL_HANDLE;
                shader.descriptorSets.clear();
            }
            return *this;
        }
        ~Shader();

        Pipeline* GetPipeline() const;
        const VkPipelineLayout& GetPipelineLayout() const;
        const std::vector<Descriptor>& GetDescriptors() const;
        std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets();
    private:
        Device* aDevice{nullptr};
        Pipeline* pipeline{nullptr};
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        void createPipelineLayout(VkDeviceSize pushConstantSize = 0);
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;
        std::vector<Descriptor>* descriptors{nullptr};
        size_t descriptorCount = 0;
    };
}