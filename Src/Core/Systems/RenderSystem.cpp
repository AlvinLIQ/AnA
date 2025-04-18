#include "Headers/RenderSystem.hpp"
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::Systems;
using namespace AnA::Cameras;

RenderSystem* currentRenderSystem = nullptr;

RenderSystem::RenderSystem(Device* mDevice, SwapChain& mSwapChain) : aDevice{mDevice}, aSwapChain {mSwapChain}
{
    currentRenderSystem = this;
}

RenderSystem::~RenderSystem()
{

}

RenderSystem* RenderSystem::GetCurrent()
{
    return currentRenderSystem;
}

void RenderSystem::RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, uint32_t bufferIndex)
{
    renderable.Bind(commandBuffer, bufferIndex);
    renderable.DrawIndirect(commandBuffer);
}
