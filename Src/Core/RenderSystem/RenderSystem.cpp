#include "Headers/RenderSystem.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::RenderSystems;
using namespace AnA::Cameras;

VkDescriptorSetLayoutBinding CameraBufferObject::GetBindingDescriptionSet()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    return uboLayoutBinding;
}

VkDescriptorBufferInfo CameraBufferObject::GetBufferInfo(VkBuffer camBuffer)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = camBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(CameraBufferObject);

    return bufferInfo;
}

RenderSystem* currentRenderSystem = nullptr;

RenderSystem::RenderSystem(Device& mDevice, SwapChain& mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    currentRenderSystem = this;
    createCameraBuffers();
    aDevice.CreateDescriptorPool(MAX_FRAMES_IN_FLIGHT, descriptorPool);
    createDescriptorSetLayout();
    aDevice.CreateDescriptorSets((std::vector<void*>&)cameraBuffers, sizeof(CameraBufferObject), 0, MAX_FRAMES_IN_FLIGHT, descriptorPool, descriptorSetLayouts[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSets);
    createPipelineLayouts();
    pipelines[TRIANGLE_LIST_PIPELINE] = new Pipeline(aDevice, "Shaders/vert.spv", "Shaders/frag.spv", aSwapChain.GetRenderPass(), pipelineLayout);
    pipelines[LINE_LIST_PIPELINE] = new Pipeline(aDevice, "Shaders/lineVert.spv", "Shaders/lineFrag.spv", aSwapChain.GetRenderPass(), pipelineLayout, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    pipelines[COMPUTE_PIPELINE] = new Pipeline(aDevice, "Shaders/compute.spv", computePipelineLayout);
}

RenderSystem::~RenderSystem()
{
    delete pipelines[COMPUTE_PIPELINE];
    vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), computePipelineLayout, nullptr);
    delete pipelines[TRIANGLE_LIST_PIPELINE];
    delete pipelines[LINE_LIST_PIPELINE];
    vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), pipelineLayout, nullptr);

    vkDestroyDescriptorPool(aDevice.GetLogicalDevice(), descriptorPool, nullptr);
    for (auto descriptorSetLayout : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(aDevice.GetLogicalDevice(), descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(aDevice.GetLogicalDevice(), computeDescriptorSetLayout, nullptr);
    for (auto &cameraBuffer : cameraBuffers)
        delete cameraBuffer;
}

RenderSystem* RenderSystem::GetCurrent()
{
    return currentRenderSystem;
}

std::array<VkDescriptorSetLayout, 3>& RenderSystem::GetDescriptorSetLayouts()
{
    return descriptorSetLayouts;
}

void RenderSystem::createPipelineLayouts()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ObjectPushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
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

void RenderSystem::createCameraBuffers()
{
    VkDeviceSize bufferSize = sizeof(CameraBufferObject);
    cameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &cameraBuffer : cameraBuffers)
    {
        cameraBuffer = new Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        cameraBuffer->Map(0, bufferSize);
    }
}

void RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, Objects &objects, int pipeLineIndex)
{
    pipelines[pipeLineIndex]->Bind(commandBuffer);

    std::array<VkDescriptorSet, 3> sets;
    sets[0] = descriptorSets[aSwapChain.CurrentFrame];
    sets[1] = objects.GetDescriptorSets()[aSwapChain.CurrentFrame];

    Object* object;
    auto objectArray = objects.Get();
    for (int i = 0; i < objectArray.size(); i++)
    {
        object = objectArray[i];
        if (object->Texture.get() == nullptr)
        {
            printf("%d\n", i);
            uint32_t color = (uint32_t)0xFF000000 ^ ((uint32_t)(object->Color.b * 255.0f) << 16) ^ ((uint32_t)(object->Color.g * 255.0f) << 8) ^ ((uint32_t)(object->Color.r * 255.0f));
            object->Texture = std::make_unique<Texture>(color, aDevice);
        }
        sets[2] = object->Texture->GetDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

        object->Model->Bind(commandBuffer);
        ObjectPushConstantData push{};
        push.index = i;
        auto &itemProperties = object->Properties;
        push.sType = itemProperties.sType;
        push.color = itemProperties.color.has_value() ? itemProperties.color.value() : object->Color;
        vkCmdPushConstants(commandBuffer, 
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(ObjectPushConstantData),
        &push);

        object->Model->Draw(commandBuffer);
    }
}

void RenderSystem::UpdateCameraBuffer(Cameras::Camera &camera)
{
    CameraBufferObject cbo;
    cbo.proj = camera.GetProjectionMatrix();
    cbo.view = camera.GetView();
    auto extent = aSwapChain.GetExtent();
    cbo.resolution = {(float)extent.width, (float)extent.height};

    memcpy(cameraBuffers[aSwapChain.CurrentFrame]->GetMappedData(), &cbo, sizeof(cbo));
}

void RenderSystem::createDescriptorSetLayout()
{
    auto uboLayoutBinding = CameraBufferObject::GetBindingDescriptionSet();
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[0]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 1");

    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the ComputeDescriptorSetLayout");

    auto ssboLayoutBinding = Model::ModelStorageBufferObject::GetBindingDescriptionSet();
    layoutInfo.pBindings = &ssboLayoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[1]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 2");

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.binding = 0;

    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[2]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout 3");
}