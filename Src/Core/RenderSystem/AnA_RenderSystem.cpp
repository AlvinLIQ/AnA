#include "Headers/AnA_RenderSystem.h"

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
    pushConstantRange.size = sizeof(ShapePushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(aDevice->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void AnA_RenderSystem::RenderObjects(VkCommandBuffer commandBuffer, std::vector<AnA_Object*> &objects)
{
    aPipeline->Bind(commandBuffer);
    for (auto& object : objects)
    {
        object->Model->Bind(commandBuffer);

        ShapePushConstantData push{};

        auto extent = aSwapChain->GetExtent();
        push.resolution = {extent.width, extent.height};

        for (auto itemProperties : object->ItemsProperties)
        {
            push.sType = itemProperties.transform.sType;
            push.offset = itemProperties.transform.translation;
            push.transform = itemProperties.transform.mat2();
            push.color = itemProperties.color.has_value() ? itemProperties.color.value() : object->Color;

            vkCmdPushConstants(commandBuffer, 
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(ShapePushConstantData),
            &push);
            
            object->Model->Draw(commandBuffer);
        }
    }
}