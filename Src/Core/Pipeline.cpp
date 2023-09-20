#include "Headers/Pipeline.hpp"

using namespace AnA;

Pipeline::Pipeline(Device& mDevice,
const char* vertShaderFileName, const char* fragShaderFileName,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout) : aDevice {mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderFileName, fragShaderFileName);
}
Pipeline::Pipeline(Device& mDevice, const char* computeShaderFile, VkPipelineLayout &mPipelineLayout) : aDevice {mDevice}, pipelineLayout {mPipelineLayout}
{
    createComputePipeline(computeShaderFile);
}

Pipeline::~Pipeline()
{
    auto logicalDevice = aDevice.GetLogicalDevice();
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void Pipeline::createGraphicsPipeline(const std::string &vertShaderFileName, const std::string &fragShaderFileName)
{
    auto vertShaderCode = ReadFile(vertShaderFileName);
    auto fragShaderCode = ReadFile(fragShaderFileName);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetDefault(vertShaderModule, fragShaderModule, pipelineLayout, renderPass); 

    auto logicalDevice = aDevice.GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createComputePipeline(const std::string& computeShaderFileName)
{
    auto computeShaderCode = ReadFile(computeShaderFileName);

    VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineInfo{};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.layout = pipelineLayout;
    computePipelineInfo.stage = computeShaderStageInfo;
    
    if (vkCreateComputePipelines(aDevice.GetLogicalDevice(), nullptr, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(aDevice.GetLogicalDevice(), computeShaderModule, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(aDevice.GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}
