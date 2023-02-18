#include "Headers/AnA_RenderSystem.h"
#include <cstddef>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::RenderSystems;

void CameraBufferObject::CreateBindingDescriptionSet(VkDescriptorSetLayoutBinding* pDescSet)
{

}

VkDescriptorBufferInfo CameraBufferObject::GetBufferInfo(VkBuffer camBuffer)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = camBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(CameraBufferObject);

    return bufferInfo;
}

AnA_RenderSystem::AnA_RenderSystem(AnA_Device *&mDevice, AnA_SwapChain *&mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    createPipelineLayout();
    aPipeline = new AnA_Pipeline(aDevice, aSwapChain->GetRenderPass(), pipelineLayout);
}

AnA_RenderSystem::~AnA_RenderSystem()
{
    for (auto &descriptionSetLayout : descriptionSetLayouts)
        vkDestroyDescriptorSetLayout(aDevice->GetLogicalDevice(), descriptionSetLayout, nullptr);

    delete aPipeline;
    vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);
}

void AnA_RenderSystem::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ObjectPushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(aDevice->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void AnA_RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, std::vector<AnA_Object*> &objects, AnA_Camera &camera)
{
    aPipeline->Bind(commandBuffer);
    for (auto& object : objects)
    {
        object->Model->Bind(commandBuffer);

        ObjectPushConstantData push{};
        auto extent = aSwapChain->GetExtent();
        push.resolution = {extent.width, extent.height};

        auto projectionMatrix = camera.GetProjectionMatrix() * camera.GetView();
        for (auto itemProperties = object->ItemsProperties.begin(); itemProperties != object->ItemsProperties.end(); itemProperties++)
        {
            push.sType = itemProperties->sType;
            //push.projectionMatrix = camera.GetProjectionMatrix();
            if (push.sType == ANA_MODEL)
            {
                itemProperties->transform.rotation.y = glm::mod(itemProperties->transform.rotation.y + 0.01f, glm::two_pi<float>());
                itemProperties->transform.translation.y = glm::sin(itemProperties->transform.rotation.y) * 0.1f;
                push.transformMatrix = projectionMatrix * itemProperties->transform.mat4();
            }
            else // For 2D Objects
                push.transformMatrix = itemProperties->transform.mat4();

            push.color = itemProperties->color.has_value() ? itemProperties->color.value() : object->Color;
            vkCmdPushConstants(commandBuffer, 
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(ObjectPushConstantData),
            &push);
            
            object->Model->Draw(commandBuffer);
        }
    }
}
