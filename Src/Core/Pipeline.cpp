#include "Headers/Pipeline.hpp"

using namespace AnA;

Pipeline::Pipeline(Device& mDevice,
const char* vertShaderFileName, const char* fragShaderFileName,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice {mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderFileName, fragShaderFileName, vertexTopology);
}

Pipeline::Pipeline(Device& mDevice,
const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice {mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderCode, fragShaderCode, vertexTopology);
}

Pipeline::Pipeline(Device& mDevice,
const std::vector<unsigned char>& vertShaderCode,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice {mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderCode, vertexTopology);
}

Pipeline::Pipeline(Device& mDevice, const char* computeShaderFile, VkPipelineLayout &mPipelineLayout) : aDevice {mDevice}, pipelineLayout {mPipelineLayout}
{
    createComputePipeline(computeShaderFile);
}

Pipeline::Pipeline(Device& mDevice, const std::vector<unsigned char>& computeShaderCode, VkPipelineLayout &mPipelineLayout) : aDevice {mDevice}, pipelineLayout {mPipelineLayout}
{
    createComputePipeline(computeShaderCode);
}

Pipeline::Pipeline(Device& mDevice, PipelineConfig pipelineConfig) : aDevice {mDevice}, renderPass {pipelineConfig.pipelineInfo.renderPass}, pipelineLayout{pipelineConfig.pipelineInfo.layout}
{
    createGraphicsPipeline(pipelineConfig);
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

void Pipeline::createGraphicsPipeline(const std::string &vertShaderFileName, const std::string &fragShaderFileName, const VkPrimitiveTopology vertexTopology)
{
    auto vertShaderCode = ReadFile(vertShaderFileName);
    auto fragShaderCode = ReadFile(fragShaderFileName);

    createGraphicsPipeline(vertShaderCode, fragShaderCode, vertexTopology);
}

void Pipeline::createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, const VkPrimitiveTopology vertexTopology)
{
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetDefault(vertShaderModule, fragShaderModule, pipelineLayout, renderPass, vertexTopology); 

    auto logicalDevice = aDevice.GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const VkPrimitiveTopology vertexTopology)
{
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetForDepthTest(vertShaderModule, pipelineLayout, renderPass, vertexTopology); 
    
    auto logicalDevice = aDevice.GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createGraphicsPipeline(PipelineConfig pipelineConfig)
{
    auto logicalDevice = aDevice.GetLogicalDevice();

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
    
    if (vkCreateComputePipelines(aDevice.GetLogicalDevice(), NULL, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(aDevice.GetLogicalDevice(), computeShaderModule, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const std::vector<unsigned char> &code)
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
