#pragma once
#include "../../Headers/Pipeline.hpp"
#include "Descriptor.hpp"
#include <glslang/Public/ShaderLang.h>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device* mDevice);
        Shader(Device* mDevice, std::vector<ShaderInfo>& shaderInfos,
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount,
            size_t _descriptorOffset, VkDeviceSize pushConstantSize = 0,
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader& shader) noexcept : Topology{shader.Topology}, aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, descriptorSets{shader.descriptorSets}, descriptors{shader.descriptors}, descriptorCount{shader.descriptorCount}, descriptorOffset{shader.descriptorOffset}
        {
        }
        Shader& operator=(Shader& shader) noexcept
        {
            if (&shader != this)
            {
                Shader::~Shader();
                aDevice = shader.aDevice;
                pipeline = shader.pipeline;
                pipelineLayout = shader.pipelineLayout;
                descriptorSets = shader.descriptorSets;
                descriptors = shader.descriptors;
                descriptorCount = shader.descriptorCount;
                descriptorOffset = shader.descriptorOffset;
                Topology = shader.Topology;
            }
            return *this;
        }
        Shader(Shader&& shader) noexcept : aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, descriptorSets{shader.descriptorSets}, descriptors{shader.descriptors}, descriptorCount{shader.descriptorCount}, descriptorOffset{shader.descriptorOffset}, hasMeshShader{shader.hasMeshShader}
        {
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
                descriptors = shader.descriptors;
                descriptorCount = shader.descriptorCount;
                descriptorOffset = shader.descriptorOffset;
                Topology = shader.Topology;
                hasMeshShader = shader.hasMeshShader;

                shader.pipelineLayout = VK_NULL_HANDLE;
                shader.descriptorSets.clear();
            }
            return *this;
        }
        ~Shader();
        bool Compile(std::vector<ShaderInfo>& shaderInfos);

        const Pipeline& GetPipeline() const;
        VkPipelineLayout GetPipelineLayout() const;
        const std::vector<Descriptor>& GetDescriptors() const;
        std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets();

        bool HasMeshShader() const
        {
            return hasMeshShader;
        }
        VkPrimitiveTopology Topology{};
    private:
        Device* aDevice{nullptr};
        Pipeline pipeline{};
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        void createPipelineLayout(VkDeviceSize pushConstantSize = 0);
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;
        std::vector<Descriptor>* descriptors{nullptr};
        size_t descriptorCount = 0;
        size_t descriptorOffset = 0;
        bool hasMeshShader = false;
    };
}
