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

RenderSystem::RenderSystem(Device& mDevice, SwapChain& mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    createCameraBuffers();
    createSamplerBuffers();
    createDescriptorPool();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelineLayout();
    aPipeline = new Pipeline(aDevice, "Shaders/vert.spv", "Shaders/frag.spv", aSwapChain.GetRenderPass(), pipelineLayout);
}

RenderSystem::~RenderSystem()
{
    delete aPipeline;
    vkDestroyPipelineLayout(aDevice.GetLogicalDevice(), pipelineLayout, nullptr);

    vkDestroyDescriptorPool(aDevice.GetLogicalDevice(), descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(aDevice.GetLogicalDevice(), descriptorSetLayout, nullptr);
    for (auto &cameraBuffer : cameraBuffers)
        delete cameraBuffer;
    for (auto &samplerBuffer : samplerBuffers)
        delete samplerBuffer;
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
    if (vkCreatePipelineLayout(aDevice.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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

void RenderSystem::createSamplerBuffers()
{
    VkDeviceSize bufferSize = sizeof(VkSampler);
    samplerBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &samplerBuffer : samplerBuffers)
    {
        samplerBuffer = new Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        samplerBuffer->Map(0, bufferSize);
    }
}

void RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, std::vector<Object*> &objects, Camera &camera)
{
    aPipeline->Bind(commandBuffer);

    UpdateCameraBuffer(camera);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1,
        &descriptorSets[aSwapChain.CurrentFrame], 0, nullptr);

    for (auto& object : objects)
    {
        object->Model->Bind(commandBuffer);
        ObjectPushConstantData push{};
        auto extent = aSwapChain.GetExtent();
        push.resolution = {extent.width, extent.height};

        //auto projectionMatrix = camera.GetProjectionMatrix()*  camera.GetView();
        object->PrepareDraw();
        auto &itemProperties = object->Properties;
        push.sType = itemProperties.sType;
        //push.projectionMatrix = camera.GetProjectionMatrix();
        if (push.sType == ANA_MODEL)
        {
            //itemProperties.transform.rotation.y = glm::mod(itemProperties.transform.rotation.y + 1.f*  camera.GetSpeedRatio(), glm::two_pi<float>());
            //itemProperties.transform.translation.y = glm::sin(itemProperties.transform.rotation.y)*  0.1f;
            push.transformMatrix = itemProperties.transform.mat4();//projectionMatrix*  itemProperties->transform.mat4();
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

    memcpy(cameraBuffers[aSwapChain.CurrentFrame]->GetMappedData(), &cbo, sizeof(cbo));
}

void RenderSystem::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(aDevice.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
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
    if (vkAllocateDescriptorSets(aDevice.GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cameraBuffers[i]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = aSwapChain.GetTextureImageView();
        imageInfo.sampler = aSwapChain.GetTextureSampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;

        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;

        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(aDevice.GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
        
    }
}

void RenderSystem::createDescriptorSetLayout()
{
    auto uboLayoutBinding = CameraBufferObject::GetBindingDescriptionSet();
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
}