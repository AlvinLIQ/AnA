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

void RenderSystem::RenderMeshes(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, uint32_t bufferIndex)
{
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[bufferIndex];

    meshes.DrawMesh(commandBuffer, sets, shader.GetPipelineLayout());
}

void RenderSystem::RenderMeshesIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, uint32_t bufferIndex)
{
    shader.GetPipeline()->Bind(commandBuffer);
    //aSwapChain.SetViewport(commandBuffer, App::GetCurrent()->GetSceneOffset());
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[bufferIndex];
    sets[DEFAULT_VERTEX_LAYOUT] = meshes.GetVertexDescriptorSet();
    sets[DEFAULT_SAMPLER_LAYOUT] = meshes.GetSamplersDescriptorSet();
    sets[DEFAULT_MESHLET_LAYOUT] = meshes.GetMeshDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
    sets.data(), 0, nullptr);

    meshes.DrawMeshIndirect(commandBuffer);
}