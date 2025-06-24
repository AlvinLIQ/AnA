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

void ShadowSystem::RenderCascadedShadowsIndirect(CommandBuffer& commandBuffer, Renderable &renderable, Shader& shader)
{
    VkExtent2D extent = {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT};
    swapChain->SetViewport(commandBuffer, extent);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 4.0f);

    renderable.Bind(commandBuffer, shader, swapChain->CurrentFrame);
    renderable.DrawIndirect(commandBuffer);
    //renderable.DrawIndirect(commandBuffer);
}
