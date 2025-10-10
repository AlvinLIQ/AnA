#include "Headers/Pipeline.hpp"

using namespace AnA;

Pipeline::Pipeline(Device* mDevice, PipelineConfig& pipelineConfig) : aDevice{mDevice}, renderPass {pipelineConfig.pipelineInfo.renderPass}, pipelineLayout{pipelineConfig.pipelineInfo.layout}
{
    if (pipelineConfig.shaderStages[0].stage == VK_SHADER_STAGE_COMPUTE_BIT)
        createComputePipeline(pipelineConfig);
    else
        createGraphicsPipeline(pipelineConfig);
}

Pipeline::~Pipeline()
{
    Cleanup();
}

void Pipeline::Bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void Pipeline::Cleanup()
{
    if (pipeline != VK_NULL_HANDLE)
    {
        auto logicalDevice = aDevice->GetLogicalDevice();
        vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    }
    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void Pipeline::createGraphicsPipeline(PipelineConfig& pipelineConfig)
{
    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");
}

void Pipeline::createComputePipeline(const std::string& computeShaderFileName)
{
    auto computeShaderCode = ReadFile(computeShaderFileName);
    createComputePipeline(computeShaderCode);
}

void Pipeline::createComputePipeline(const std::vector<unsigned char>& computeShaderCode)
{
    VkShaderModule computeShaderModule = aDevice->CreateShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineInfo{};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.layout = pipelineLayout;
    computePipelineInfo.stage = computeShaderStageInfo;
    
    if (vkCreateComputePipelines(aDevice->GetLogicalDevice(), NULL, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(aDevice->GetLogicalDevice(), computeShaderModule, nullptr);
}

void Pipeline::createComputePipeline(PipelineConfig& pipelineConfig)
{
    VkComputePipelineCreateInfo computePipelineInfo{};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.layout = pipelineLayout;
    computePipelineInfo.stage = pipelineConfig.shaderStages[0];

    if (vkCreateComputePipelines(aDevice->GetLogicalDevice(), NULL, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }
}