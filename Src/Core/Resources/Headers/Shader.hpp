#pragma once
#include "../../Headers/Pipeline.hpp"

namespace AnA
{
    class Shader
    {
    public:
        Shader(Device* mDevice);
        Shader(Device* mDevice, std::vector<ShaderInfo>& shaderInfos,
            VkDescriptorSetLayout _setLayout,
            VkDeviceSize pushConstantSize = 0,
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader& shader) noexcept : Topology{shader.Topology}, aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, setLayout{shader.setLayout}
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
                setLayout = shader.setLayout;
                Topology = shader.Topology;
            }
            return *this;
        }
        Shader(Shader&& shader) noexcept : aDevice{shader.aDevice}, pipeline{shader.pipeline}, pipelineLayout{shader.pipelineLayout}, setLayout{shader.setLayout}
        {
            shader.pipelineLayout = VK_NULL_HANDLE;
        }
        Shader& operator=(Shader&& shader) noexcept
        {
            if (&shader != this)
            {
                Shader::~Shader();
                aDevice = shader.aDevice;
                pipeline = shader.pipeline;
                pipelineLayout = shader.pipelineLayout;
                setLayout = shader.setLayout;
                Topology = shader.Topology;
                hasMeshShader = shader.hasMeshShader;

                shader.pipelineLayout = VK_NULL_HANDLE;
                shader.setLayout = VK_NULL_HANDLE;
            }
            return *this;
        }
        ~Shader();
        bool Compile(std::vector<ShaderInfo>& shaderInfos);

        const Pipeline& GetPipeline() const;
        VkPipelineLayout GetPipelineLayout() const;

        bool HasMeshShader() const
        {
            return hasMeshShader;
        }
        VkPrimitiveTopology Topology{};
        VkShaderStageFlags StageFlags;
    private:
        Device* aDevice{nullptr};
        Pipeline pipeline{};
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        void createPipelineLayout(VkDeviceSize pushConstantSize = 0);
        VkDescriptorSetLayout setLayout{VK_NULL_HANDLE};
        bool hasMeshShader = false;
    };
}
