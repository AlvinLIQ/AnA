#include "Headers/RenderSystem.hpp"
#include "Headers/ShadowSystem.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::Systems;
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
    pipelines = new Pipelines(aDevice, aSwapChain.GetRenderPass(), aSwapChain.GetOffscreenRenderPass(),
        CameraBufferObject::GetBindingDescriptionSet(), 
        {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ObjectPushConstantData)});
    aDevice.CreateDescriptorPool(MAX_FRAMES_IN_FLIGHT, descriptorPool);
    aDevice.CreateDescriptorSets((std::vector<void*>&)cameraBuffers, sizeof(CameraBufferObject), 0, MAX_FRAMES_IN_FLIGHT, descriptorPool, pipelines->GetDescriptorSetLayouts()[UBO_LAYOUT], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSets);
}

RenderSystem::~RenderSystem()
{
    delete pipelines;

    vkDestroyDescriptorPool(aDevice.GetLogicalDevice(), descriptorPool, nullptr);

    for (auto &cameraBuffer : cameraBuffers)
        delete cameraBuffer;
}

RenderSystem* RenderSystem::GetCurrent()
{
    return currentRenderSystem;
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
    pipelines->Get()[pipeLineIndex]->Bind(commandBuffer);

    VkDescriptorSet sets[DESCRIPTOR_SET_LAYOUT_COUNT];
    sets[UBO_LAYOUT] = descriptorSets[aSwapChain.CurrentFrame];
    sets[SSBO_LAYOUT] = objects.GetDescriptorSets()[aSwapChain.CurrentFrame];
    sets[SHADOW_SAMPLER_LAYOUT] = ShadowSystem::GetCurrent()->GetShadowSamplerSet();

    Object* object;
    auto objectArray = objects.Get();
    for (int i = 0; i < objectArray.size(); i++)
    {
        object = objectArray[i];
        if (object->Texture.get() == nullptr)
        {
            uint32_t color = (uint32_t)0xFF000000 ^ ((uint32_t)(object->Color.b * 255.0f) << 16) ^ ((uint32_t)(object->Color.g * 255.0f) << 8) ^ ((uint32_t)(object->Color.r * 255.0f));
            object->Texture = std::make_unique<Texture>(color, aDevice);
        }
        sets[SAMPLER_LAYOUT] = object->Texture->GetDescriptorSet();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelines->Get()[pipeLineIndex]->GetLayout(), 0, DESCRIPTOR_SET_LAYOUT_COUNT,
        sets, 0, nullptr);

        object->Model->Bind(commandBuffer);
        ObjectPushConstantData push{};
        auto &itemProperties = object->Properties;
        push.sType = itemProperties.sType;
        push.color = itemProperties.color.has_value() ? itemProperties.color.value() : object->Color;
        vkCmdPushConstants(commandBuffer, 
        pipelines->Get()[pipeLineIndex]->GetLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(ObjectPushConstantData),
        &push);

        object->Model->Draw(commandBuffer, i);
    }
}

void RenderSystem::UpdateCameraBuffer(Cameras::Camera &camera)
{
    CameraBufferObject cbo;
    cbo.proj = camera.GetProjectionMatrix();
    cbo.view = camera.GetView();
    cbo.invView = camera.GetInverseView();
    glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, -3.0f, 1.0f)); // Example light direction
    glm::vec3 upVector = glm::vec3(0.0f, -1.0f, 0.0f); // Up vector (usually positive y-axis)
    glm::vec3 referencePoint = glm::vec3(0.0f, -1.5f, 5.7f); // Reference point in the scene
    glm::mat4 viewMatrix = glm::lookAt(referencePoint, referencePoint + lightDirection, upVector);

    Camera lightCam;
    lightCam.SetPerspectiveProjection(45.0f, 4.0f / 3.0f, 1.25f, 1.75f);
    cbo.lightView = lightCam.GetProjectionMatrix() * viewMatrix;//glm::perspective(45.0f, 4.0f/3.0f, 1.0f, 96.0f);// * viewMatrix;
    auto extent = aSwapChain.GetExtent();
    cbo.resolution = {(float)extent.width, (float)extent.height};

    memcpy(cameraBuffers[aSwapChain.CurrentFrame]->GetMappedData(), &cbo, sizeof(cbo));
}
