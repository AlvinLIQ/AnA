#include "Headers/AnA_RenderSystem.h"

using namespace AnA::RenderSystems;

struct SimplePushConstantData
{
    glm::mat2 transform {1.f};
    glm::uint32_t sType;
    alignas(8) glm::vec2 offset;
    glm::vec2 resolution;
    alignas(16) glm::vec3 color;
};

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
    pushConstantRange.size = sizeof(SimplePushConstantData);


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

        SimplePushConstantData push{};
        push.sType = 1;
        push.offset = object->Transform2D.translation;
        push.color = object->Color;
        push.transform = object->Transform2D.mat2();

/*        int width, height;
        glfwGetFramebufferSize(aWindow->GetGLFWwindow(), &width, &height);
        push.resolution = {(float)width, (float)height};*/

        auto extent = aSwapChain->GetExtent();
        push.resolution = {extent.width, extent.height};

        vkCmdPushConstants(commandBuffer, 
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
            
        object->Model->Draw(commandBuffer);
    }
}