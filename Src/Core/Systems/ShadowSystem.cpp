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
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    if (swapChain->GetDevice()->MeshShaderSupport())
        stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT;
    auto pipelineLayout = shader.GetPipelineLayout();
    vkCmdPushConstants(commandBuffer, pipelineLayout,
            stageFlags, 0, sizeof(uint32_t), &index);
    renderable.Bind(commandBuffer, shader, swapChain->CurrentFrame);
    renderable.Draw(commandBuffer);
}
