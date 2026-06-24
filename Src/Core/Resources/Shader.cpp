#include "Headers/Shader.hpp"
#include "Headers/SwapChain.hpp"
//#include <glslang/Public/ShaderLang.h>
//#include <glslang/SPIRV/GlslangToSpv.h>

using namespace AnA;

Shader::Shader(Device* mDevice) : aDevice{mDevice}
{

}

Shader::Shader(Device* mDevice, std::vector<ShaderInfo>& shaderInfos,
            std::vector<Descriptor>& _descriptors, size_t actualDescriptorCount,
            size_t _descriptorOffset, VkDeviceSize pushConstantSize, VkPrimitiveTopology topology) :
    Topology{topology}, aDevice{mDevice},
    descriptors{&_descriptors}, descriptorCount{actualDescriptorCount}, descriptorOffset{_descriptorOffset}
{
    createPipelineLayout(pushConstantSize);
    auto swapChain = SwapChain::GetCurrent();
    Pipeline::PipelineConfig pipelineConfig =
        Pipeline::PipelineConfig::GetForDynamicRendering(mDevice, shaderInfos, pipelineLayout,
        swapChain->GetFormat(), swapChain->GetDepthFormat(), aDevice->GetMaxUsableSampleCount(),
        Topology);
    hasMeshShader= pipelineConfig.hasMeshShader;
    pipeline = Pipeline(mDevice, pipelineConfig);
    Pipeline::PipelineConfig::CleanupPipelineConfig(mDevice, pipelineConfig);
}

Shader::~Shader()
{

}
#ifdef HAVE_GLSLANG

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
    glslang::InitializeProcess();
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
    glslang::FinalizeProcess();
    return true;
}
#endif

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
                VK_NULL_HANDLE);
            }
        }
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = uint32_t(descriptorCount);
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    VkPushConstantRange range;
    if (pushConstantSize)
    {
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        if (aDevice->MeshShaderSupport())
            range.stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        range.offset = 0;
        range.size = uint32_t(pushConstantSize);
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
