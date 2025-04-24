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

void ShadowSystem::RenderShadows(CommandBuffer& commandBuffer, Scene &meshes, Shader& shader)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[0];
    sets[DEFAULT_VERTEX_LAYOUT] = meshes.GetVertexDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3,
        sets.data(), 0, nullptr);
    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer);
}

void ShadowSystem::RenderShadowsIndirect(CommandBuffer& commandBuffer, Scene &meshes, Shader& shader)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[0];
    sets[DEFAULT_VERTEX_LAYOUT] = meshes.GetMeshDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 3,
        sets.data(), 0, nullptr);
    meshes.Bind(commandBuffer);
    meshes.DrawIndirect(commandBuffer);
}

void ShadowSystem::RenderCascadedShadowsIndirect(CommandBuffer& commandBuffer, Scene &meshes, Shader& shader, uint32_t& index)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};
    swapChain->SetViewport(commandBuffer, {}, extent);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[0];
    //sets[DEFAULT_VERTEX_LAYOUT] = meshes.GetSSBODescriptor()->GetSets()[0];
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