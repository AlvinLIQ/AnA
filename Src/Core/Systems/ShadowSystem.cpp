#include "Headers/ShadowSystem.hpp"
#include "../Resources/Headers/Shader.hpp"

using namespace AnA;
using namespace Systems;

ShadowSystem* _shadowSystem;

ShadowSystem::ShadowSystem(SwapChain* pSwapchain)// : aDevice{mDevice}
{
    swapChain = pSwapchain;
    _shadowSystem = this;
}

ShadowSystem::~ShadowSystem()
{

}

ShadowSystem* ShadowSystem::GetCurrent()
{
    return _shadowSystem;
}

void ShadowSystem::RenderCascadedShadowsIndirect(CommandBuffer& commandBuffer, Renderable &renderable, Shader& shader, uint32_t& index)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
    vkCmdSetDepthBias(commandBuffer, 2.0f, 0.0f, 4.0f);
    auto pipelineLayout = shader.GetPipelineLayout();
    /*
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, sets.size(),
        sets.data(), 0, nullptr);*/
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    if (swapChain->GetDevice()->MeshShaderSupport())
        stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT;
    vkCmdPushConstants(commandBuffer, pipelineLayout,
            stageFlags, 0, sizeof(uint32_t), &index);
    renderable.Bind(commandBuffer, shader, swapChain->CurrentFrame);
    renderable.DrawIndirect(commandBuffer);
}
