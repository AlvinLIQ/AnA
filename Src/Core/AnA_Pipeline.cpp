#include "Headers/AnA_Pipeline.h"

using namespace AnA;

static std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + filename + "!");
    }
    
    size_t fs = (size_t)file.tellg();
    std::vector<char> buffer(fs);
    file.seekg(0);
    file.read(buffer.data(), fs);
    file.close();
    return buffer;
}

AnA_Pipeline::AnA_Pipeline(AnA_Device *&mDevice, 
VkRenderPass &mRenderPass, 
VkPipelineLayout &mPipelineLayout) : aDevice {mDevice}, renderPass {mRenderPass}, pipelineLayout{mPipelineLayout}
{
    createGraphicsPipeline("Shaders/vert.spv", "Shaders/frag.spv");
}
AnA_Pipeline::~AnA_Pipeline()
{
    auto logicalDevice = aDevice->GetLogicalDevice();
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
}

void AnA_Pipeline::Bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void AnA_Pipeline::createGraphicsPipeline(const std::string &vertShaderFileName, const std::string &fragShaderFileName)
{
    auto vertShaderCode = readFile(vertShaderFileName);
    auto fragShaderCode = readFile(fragShaderFileName);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetDefault(vertShaderModule, fragShaderModule, pipelineLayout, renderPass); 

    auto logicalDevice = aDevice->GetLogicalDevice();

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineConfig.pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline!");

    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

VkShaderModule AnA_Pipeline::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(aDevice->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}
