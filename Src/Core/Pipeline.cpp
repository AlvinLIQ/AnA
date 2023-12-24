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

void Pipeline::createGraphicsPipeline(const std::string &vertShaderFileName, const std::string &fragShaderFileName, const VkPrimitiveTopology vertexTopology)
{
    auto vertShaderCode = ReadFile(vertShaderFileName);
    auto fragShaderCode = ReadFile(fragShaderFileName);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    PipelineConfig pipelineConfig = pipelineConfig.GetDefault(vertShaderModule, fragShaderModule, pipelineLayout, renderPass, vertexTopology); 

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

Pipelines* _pipelines = nullptr;

Pipelines::Pipelines(Device& mDevice, VkRenderPass renderPass, VkDescriptorSetLayoutBinding uboLayoutBinding, VkPushConstantRange pushConstantRange) : aDevice{mDevice}
{
    _pipelines = this;
    createDescriptorSetLayouts(uboLayoutBinding);
    createPipelineLayouts(pushConstantRange);
    pipelines.resize(PIPELINE_COUNT);
    pipelines[TRIANGLE_LIST_PIPELINE] = new Pipeline(aDevice, "Shaders/vert.spv", "Shaders/frag.spv", renderPass, pipelineLayout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelines[LINE_LIST_PIPELINE] = new Pipeline(aDevice, "Shaders/lineVert.spv", "Shaders/lineFrag.spv", renderPass, pipelineLayout, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    pipelines[POINT_LIST_PIPELINE] = new Pipeline(aDevice, "Shaders/lineVert.spv", "Shaders/lineFrag.spv", renderPass, pipelineLayout, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    pipelines[COMPUTE_PIPELINE] = new Pipeline(aDevice, "Shaders/compute.spv", computePipelineLayout);
}

Pipelines::~Pipelines()
{
    delete pipelines[COMPUTE_PIPELINE];
    vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), computePipelineLayout, nullptr);
    delete pipelines[TRIANGLE_LIST_PIPELINE];
    delete pipelines[LINE_LIST_PIPELINE];
    delete pipelines[POINT_LIST_PIPELINE];
    vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), pipelineLayout, nullptr);

    for (auto descriptorSetLayout : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(aDevice.GetLogicalDevice(), descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(aDevice.GetLogicalDevice(), computeDescriptorSetLayout, nullptr);
}

Pipelines* Pipelines::GetCurrent()
{
    return _pipelines;
}

void Pipelines::createPipelineLayouts(VkPushConstantRange pushConstantRange)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = numsof(descriptorSetLayouts);
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    if (pushConstantRange.size)
    {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }
    if (vkCreatePipelineLayout(aDevice.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    
    VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};
    computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutInfo.setLayoutCount = 1;
    computePipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;
    if (vkCreatePipelineLayout(aDevice.GetLogicalDevice(), &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }
}

void Pipelines::createDescriptorSetLayouts(VkDescriptorSetLayoutBinding uboLayoutBinding)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[UBO_LAYOUT]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 1");
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the ComputeDescriptorSetLayout");
    
    auto ssboLayoutBinding = Model::ModelStorageBufferObject::GetBindingDescriptionSet();
    layoutInfo.pBindings = &ssboLayoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[SSBO_LAYOUT]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 2");

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.binding = 0;
    layoutInfo.pBindings = &samplerLayoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[SAMPLER_LAYOUT]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 3");
}