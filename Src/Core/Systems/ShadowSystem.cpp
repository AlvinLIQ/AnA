#include "Headers/ShadowSystem.hpp"
#include "../Resources/Headers/Shader.hpp"
#include "../Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Systems;

ShadowSystem* _shadowSystem;

ShadowSystem::ShadowSystem(Device* mDevice, SwapChain* pSwapchain) : aDevice{mDevice}
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

void ShadowSystem::RenderShadows(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
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
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
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

void ShadowSystem::RenderCascadedShadowsIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, uint32_t& index)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[Resource::ResourceManager::GetCurrent()->SecondaryCommandBufferPool.CurrentBufferIndex];
    //sets[DEFAULT_SSBO_LAYOUT] = meshes.GetSSBODescriptor()->GetSets()[0];
    auto pipelineLayout = shader.GetPipelineLayout();
    /*
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, sets.size(),
        sets.data(), 0, nullptr);*/
    vkCmdPushConstants(commandBuffer, pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &index);
    meshes.Bind(commandBuffer);
    meshes.DrawIndirect(commandBuffer, sets, pipelineLayout);
}