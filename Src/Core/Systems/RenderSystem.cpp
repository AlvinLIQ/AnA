#include "Headers/RenderSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
#include <vulkan/vulkan_core.h>
#include <glm/gtc/constants.hpp>

using namespace AnA::Systems;
using namespace AnA::Cameras;

RenderSystem* currentRenderSystem = nullptr;

RenderSystem::RenderSystem(Device& mDevice, SwapChain& mSwapChain) : aDevice {mDevice}, aSwapChain {mSwapChain}
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

void RenderSystem::RenderObject(VkCommandBuffer commandBuffer, Object& object, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            shader.GetPipelineLayout(), 0, 1,
            sets.data(), 0, nullptr);
    object.Draw(commandBuffer);
}

void RenderSystem::RenderMeshes(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];
    auto& textureMap = resourceManager->TextureMap;
    auto& texture = textureMap.at(DEFAULT_TEXTURE_ID);
    //sets[DEFAULT_SAMPLER_LAYOUT] = texture.GetDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer);
}

void RenderSystem::RenderBatch(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, size_t index)
{
    shader.GetPipeline()->Bind(commandBuffer);
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    std::vector<VkDescriptorSet>& sets = shader.GetDescriptorSets()[resourceManager->SecondaryCommandBufferPool.CurrentBufferIndex];
    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer, sets, shader.GetPipelineLayout(), 0, meshes.GetMeshCount());
}
