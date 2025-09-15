#include "Headers/Shader.hpp"
#include "Headers/ResourceManager.hpp"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

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
    pipeline = Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, size_t _descriptorOffset, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);

    pipeline = Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass& renderPass, 
    const std::vector<unsigned char>& fragShaderCode, std::vector<Descriptor>& _descriptors,
    size_t actualDescriptorCount, size_t _descriptorOffset, VkDeviceSize pushConstantSize) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);

    pipeline = Pipeline(mDevice, vertShaderCode, renderPass, pipelineLayout, fragShaderCode);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, 
    const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass,
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    Resource::ResourceManager::GetCurrent()->GetDefaultDescriptorSetConfig(descriptorSetConfigs);

    createPipelineLayout(pushConstantSize);
    pipeline = Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout, vertexTopology);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, size_t _descriptorOffset, 
    const VkPrimitiveTopology vertexTopology, VkDeviceSize pushConstantSize) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);

    pipeline = Pipeline(mDevice, vertShaderCode, fragShaderCode, renderPass, pipelineLayout, vertexTopology);
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, 
    const std::vector<unsigned char>& fragShaderCode, VkRenderPass& renderPass, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, size_t _descriptorOffset, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);

    pipeline = Pipeline(mDevice, taskShaderCode, meshShaderCode, fragShaderCode, renderPass, pipelineLayout);
    hasMeshShader = true;
}

Shader::Shader(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, size_t _descriptorOffset, VkRenderPass& renderPass, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);

    pipeline = Pipeline(mDevice, taskShaderCode, meshShaderCode, pipelineLayout, renderPass);
    hasMeshShader = true;
}

Shader::Shader(Device* mDevice, std::vector<ShaderInfo>& shaderInfos, 
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, 
            size_t _descriptorOffset, VkDeviceSize pushConstantSize) : aDevice{mDevice}, 
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);
    auto swapChain = SwapChain::GetCurrent();
    Pipeline::PipelineConfig pipelineConfig = 
        Pipeline::PipelineConfig::GetForDynamicRendering(mDevice, shaderInfos, pipelineLayout, 
        swapChain->GetFormat(), swapChain->GetDepthFormat(), aDevice->GetMaxUsableSampleCount());
    pipeline = Pipeline(mDevice, pipelineConfig);
    Pipeline::PipelineConfig::CleanupPipelineConfig(mDevice, pipelineConfig);
}

Shader::Shader(Device* mDevice, Pipeline::PipelineConfig pipelineConfig, 
    std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount, size_t _descriptorOffset) : aDevice{mDevice}, descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    pipeline = Pipeline(mDevice, pipelineConfig);
}

Shader::~Shader()
{
    
}

EShLanguage ShaderStageToEshLanguage(VkShaderStageFlagBits stage)
{
    switch(stage)
    {
        case VK_SHADER_STAGE_VERTEX_BIT: return EShLangVertex;
        case VK_SHADER_STAGE_FRAGMENT_BIT: return EShLangFragment;
        case VK_SHADER_STAGE_MESH_BIT_EXT: return EShLangMesh;
        case VK_SHADER_STAGE_TASK_BIT_EXT: return EShLangTask;
        case VK_SHADER_STAGE_COMPUTE_BIT: return EShLangCompute;
        default: return EShLangAnyHit;   
    }
}

bool Shader::Compile(std::vector<ShaderInfo>& shaderInfos)
{
    for (auto& shaderInfo : shaderInfos)
    {
        EShLanguage lang = ShaderStageToEshLanguage(shaderInfo.stage);
        glslang::TShader shader{lang};
        auto code = reinterpret_cast<const char*>(shaderInfo.codes.data());
        shader.setStrings(&code, 1);
        shader.setEntryPoint("main");
        shader.setSourceEntryPoint("main");

        shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, 100);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
        EShMessages messages = EShMsgDefault;
        glslang::TProgram program;
        program.addShader(&shader);
    
        if (!program.link(messages))
        {
            perror(program.getInfoLog());
            return false;
        }
        //glslang::GlslangToSpv(*program.getIntermediate(lang), shaderInfo.spirv);
    }
    return true;
}

const Pipeline& Shader::GetPipeline() const
{
    return pipeline;
}
VkPipelineLayout Shader::GetPipelineLayout() const
{
    return pipeline.GetLayout();
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
            auto& descriptor = descriptors->data()[i + descriptorOffset];
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
            range.stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
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