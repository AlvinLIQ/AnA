#include "Headers/RenderSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
#include "../Headers/App.hpp"
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

void RenderSystem::RenderShapes(VkCommandBuffer commandBuffer, Shapes& shapes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    shapes.PrepareDraw(resourceManager->MainControl);
    shapes.Draw(commandBuffer, shader.GetPipelineLayout());
}

void RenderSystem::RenderShapesIndirect(VkCommandBuffer commandBuffer, Shapes& shapes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    shapes.DrawIndirect(commandBuffer, shader.GetPipelineLayout());
}

void RenderSystem::RenderMeshes(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];

    meshes.Bind(commandBuffer);
    meshes.DrawMesh(commandBuffer, sets, shader.GetPipelineLayout());
}

void RenderSystem::RenderMeshesIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();

    aSwapChain.SetViewport(commandBuffer, App::GetCurrent()->GetSceneOffset());
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];

    meshes.Bind(commandBuffer);
    meshes.DrawIndirect(commandBuffer, sets, shader.GetPipelineLayout());
}