#include "Headers/AnA_RenderSystem.h"
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::RenderSystems;

AnA_RenderSystem::AnA_RenderSystem(AnA_Device *&mDevice, AnA_SwapChain *&mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
{
    createPipelineLayout();
    aPipeline = new AnA_Pipeline(aDevice, aSwapChain->GetRenderPass(), pipelineLayout);
}

AnA_RenderSystem::~AnA_RenderSystem()
{
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
        RenderObject(commandBuffer, object, camera);
    }
}

void AnA_RenderSystem::RenderObject(VkCommandBuffer commandBuffer, AnA_Object* &object, AnA_Camera &camera)
{
    object->Model->Bind(commandBuffer);

    ObjectPushConstantData push{};
    auto extent = aSwapChain->GetExtent();
    push.resolution = {extent.width, extent.height};

    //auto projectionMatrix = camera.GetProjectionMatrix() * camera.GetView();
    for (auto itemProperties = object->ItemsProperties.begin(); itemProperties != object->ItemsProperties.end(); itemProperties++)
    {
        push.sType = itemProperties->sType;
        //push.projectionMatrix = camera.GetProjectionMatrix();
        itemProperties->transform.rotation.y = glm::mod(itemProperties->transform.rotation.y + 0.01f, glm::two_pi<float>());
        itemProperties->transform.rotation.x = glm::mod(itemProperties->transform.rotation.x + 0.01f, glm::two_pi<float>());
        push.transformMatrix = itemProperties->transform.mat4();
        //if (push.sType == ANA_MODEL)
        //   push.transformMatrix = projectionMatrix * push.transformMatrix;

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