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

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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

RenderSystem::RenderSystem(Device *&mDevice, SwapChain *&mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    createCameraBuffers();
    createDescriptorPool();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelineLayout();
    aPipeline = new Pipeline(aDevice, "Shaders/vert.spv", "Shaders/frag.spv", aSwapChain->GetRenderPass(), pipelineLayout);
}

RenderSystem::~RenderSystem()
{
    delete aPipeline;
    vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);

    vkDestroyDescriptorPool(aDevice->GetLogicalDevice(), descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(aDevice->GetLogicalDevice(), descriptorSetLayout, nullptr);
    for (auto &cameraBuffer : cameraBuffers)
        delete cameraBuffer;
}

void RenderSystem::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ObjectPushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(aDevice->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
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

void RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, std::vector<Object*> &objects, Camera &camera)
{
    aPipeline->Bind(commandBuffer);

    UpdateCameraBuffer(camera);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1,
        &descriptorSets[aSwapChain->CurrentFrame], 0, nullptr);

    for (auto& object : objects)
    {
        object->Model->Bind(commandBuffer);
        ObjectPushConstantData push{};
        auto extent = aSwapChain->GetExtent();
        push.resolution = {extent.width, extent.height};

        //auto projectionMatrix = camera.GetProjectionMatrix() * camera.GetView();
        object->PrepareDraw();
        auto &itemProperties = object->Properties;
        push.sType = itemProperties.sType;
        //push.projectionMatrix = camera.GetProjectionMatrix();
        if (push.sType == ANA_MODEL)
        {
            itemProperties.transform.rotation.y = glm::mod(itemProperties.transform.rotation.y + 1.f * camera.GetSpeedRatio(), glm::two_pi<float>());
            itemProperties.transform.translation.y = glm::sin(itemProperties.transform.rotation.y) * 0.1f;
            push.transformMatrix = itemProperties.transform.mat4();//projectionMatrix * itemProperties->transform.mat4();
        }
        else // For 2D Objects
            push.transformMatrix = itemProperties.transform.mat4();
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

    memcpy(cameraBuffers[aSwapChain->CurrentFrame]->GetMappedData(), &cbo, sizeof(cbo));
}

void RenderSystem::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = poolSize.descriptorCount;

    if (vkCreateDescriptorPool(aDevice->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
}

void RenderSystem::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(aDevice->GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cameraBuffers[i]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
        
    }
}

void RenderSystem::createDescriptorSetLayout()
{
    auto uboLayoutBinding = CameraBufferObject::GetBindingDescriptionSet();
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
}