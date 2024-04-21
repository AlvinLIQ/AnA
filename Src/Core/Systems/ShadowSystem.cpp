#include "Headers/ShadowSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Systems;

ShadowSystem* _shadowSystem;

ShadowSystem::ShadowSystem(Device& mDevice, SwapChain* pSwapchain) : aDevice(mDevice)
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

VkExtent2D ShadowSystem::GetExtent()
{
    return swapChain->GetExtent();
}

void ShadowSystem::RenderShadows(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[Resource::ResourceManager::GetCurrent()->SecondaryCommandBufferPool.CurrentBufferIndex];
    sets[DEFAULT_SSBO_LAYOUT] = meshes.GetSSBODescriptor()->GetSets()[0];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3,
        sets.data(), 0, nullptr);
    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer);
}

void ShadowSystem::RenderShadowsIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    swapChain->SetViewport(commandBuffer);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[Resource::ResourceManager::GetCurrent()->SecondaryCommandBufferPool.CurrentBufferIndex];
    sets[DEFAULT_SSBO_LAYOUT] = meshes.GetSSBODescriptor()->GetSets()[0];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3,
        sets.data(), 0, nullptr);
    meshes.Bind(commandBuffer);
    meshes.DrawIndirect(commandBuffer);
}