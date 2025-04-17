#include "Headers/RenderSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
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

void RenderSystem::RenderShapes(CommandBuffer& commandBuffer, Shapes& shapes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    shapes.PrepareDraw(resourceManager->MainControl);
    shapes.Draw(commandBuffer, shader.GetPipelineLayout());
}

void RenderSystem::RenderShapesIndirect(CommandBuffer& commandBuffer, Shapes& shapes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    shapes.DrawIndirect(commandBuffer, shader.GetPipelineLayout());
}

void RenderSystem::RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, uint32_t bufferIndex)
{
    renderable.Bind(commandBuffer, bufferIndex);
    renderable.DrawIndirect(commandBuffer);
}
