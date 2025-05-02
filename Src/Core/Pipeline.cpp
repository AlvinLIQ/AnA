#include "Headers/Pipeline.hpp"

using namespace AnA;

Pipeline::Pipeline(Device* mDevice,
const char* vertShaderFileName, const char* fragShaderFileName,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice{mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderFileName, fragShaderFileName, vertexTopology);
}

Pipeline::Pipeline(Device* mDevice,
const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice{mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderCode, fragShaderCode, vertexTopology);
}

Pipeline::Pipeline(Device* mDevice,
const std::vector<unsigned char>& vertShaderCode,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout,
const VkPrimitiveTopology vertexTopology) : aDevice{mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderCode, vertexTopology);
}

Pipeline::Pipeline(Device* mDevice,
const std::vector<unsigned char>& vertShaderCode,
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout, const std::vector<unsigned char>& fragShaderCode, 
const VkPrimitiveTopology vertexTopology) : aDevice{mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline(vertShaderCode, vertexTopology, fragShaderCode);
}

Pipeline::Pipeline(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, 
    const std::vector<unsigned char>& meshShaderCode, const std::vector<unsigned char>& fragShaderCode, 
    VkRenderPass &mRenderPass, 
    VkPipelineLayout &mPipelineLayoutconst) : aDevice{mDevice}, renderPass{mRenderPass}, pipelineLayout{mPipelineLayoutconst}
{
    createMeshShaderPipeline(taskShaderCode, meshShaderCode, fragShaderCode);
}

Pipeline::Pipeline(Device* mDevice, const std::vector<unsigned char>& taskShaderCode, 
    const std::vector<unsigned char>& meshShaderCode, 
    VkPipelineLayout &mPipelineLayoutconst, VkRenderPass &mRenderPass) : aDevice{mDevice}, renderPass{mRenderPass}, pipelineLayout{mPipelineLayoutconst}
{
    createMeshShaderPipeline(taskShaderCode, meshShaderCode);
}

Pipeline::Pipeline(Device* mDevice, const char* computeShaderFile, VkPipelineLayout &mPipelineLayout) : aDevice{mDevice}, pipelineLayout {mPipelineLayout}
{
    createComputePipeline(computeShaderFile);
}

Pipeline::Pipeline(Device* mDevice, const std::vector<unsigned char>& computeShaderCode, VkPipelineLayout &mPipelineLayout) : aDevice{mDevice}, pipelineLayout {mPipelineLayout}
{
    createComputePipeline(computeShaderCode);
}

Pipeline::Pipeline(Device* mDevice, PipelineConfig pipelineConfig) : aDevice{mDevice}, renderPass {pipelineConfig.pipelineInfo.renderPass}, pipelineLayout{pipelineConfig.pipelineInfo.layout}
{
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

    PipelineConfig pipelineConfig = pipelineConfig.GetDefault(vertShaderModule, fragShaderModule, pipelineLayout, renderPass, aDevice->GetMaxUsableSampleCount(), vertexTopology); 

    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const VkPrimitiveTopology vertexTopology)
{
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetForDepthTest(vertShaderModule, VK_NULL_HANDLE, pipelineLayout, renderPass, vertexTopology); 
    
    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const VkPrimitiveTopology vertexTopology, const std::vector<unsigned char>& fragShaderCode)
{
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetForDepthTest(vertShaderModule, fragShaderModule, pipelineLayout, renderPass, vertexTopology); 
    
    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void Pipeline::createGraphicsPipeline(PipelineConfig pipelineConfig)
{
    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");
}

void Pipeline::createMeshShaderPipeline(const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode, const std::vector<unsigned char>& fragShaderCode)
{
    VkShaderModule taskShaderModule = createShaderModule(taskShaderCode);
    VkShaderModule meshShaderModule = createShaderModule(meshShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    auto pipelineConfig = PipelineConfig::GetForMeshShader(taskShaderModule, meshShaderModule, fragShaderModule, pipelineLayout, renderPass, aDevice->GetMaxUsableSampleCount());
    
    if (vkCreateGraphicsPipelines(aDevice->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(aDevice->GetLogicalDevice(), taskShaderModule, nullptr);
    vkDestroyShaderModule(aDevice->GetLogicalDevice(), meshShaderModule, nullptr);
    vkDestroyShaderModule(aDevice->GetLogicalDevice(), fragShaderModule, nullptr);
}

void Pipeline::createMeshShaderPipeline(const std::vector<unsigned char>& taskShaderCode, const std::vector<unsigned char>& meshShaderCode)
{
    VkShaderModule taskShaderModule = createShaderModule(taskShaderCode);
    VkShaderModule meshShaderModule = createShaderModule(meshShaderCode);

    auto pipelineConfig = PipelineConfig::GetForDepthTestMeshShader(taskShaderModule, meshShaderModule, pipelineLayout, renderPass);
    
    if (vkCreateGraphicsPipelines(aDevice->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(aDevice->GetLogicalDevice(), taskShaderModule, nullptr);
    vkDestroyShaderModule(aDevice->GetLogicalDevice(), meshShaderModule, nullptr);
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
    
    if (vkCreateComputePipelines(aDevice->GetLogicalDevice(), NULL, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(aDevice->GetLogicalDevice(), computeShaderModule, nullptr);
}

VkShaderModule Pipeline::createShaderModule(const std::vector<unsigned char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(aDevice->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}
